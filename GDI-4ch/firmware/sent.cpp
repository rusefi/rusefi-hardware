/*
 * sent.cpp
 *
 * SENT protocol decoder
 *
 * @date Dec 13, 2024
 * @author Andrey Gusakov <dron0gus@gmail.com>, (c) 2024
 */

#include "ch.h"
#include "hal.h"

#include "sent.h"

#ifndef SENT_INPUT_COUNT
#define SENT_INPUT_COUNT		4 // Number of sent channels
#endif

static sent_channel channels[SENT_INPUT_COUNT];

#if 0
void sent_channel::Info() {
	uint8_t stat;
	uint16_t sig0, sig1;

	efiPrintf("Unit time %lu timer ticks", tickPerUnit);
	efiPrintf("Pause pulse detected %s", pausePulseReceived ? "Yes" : "No");
	efiPrintf("Total pulses %lu", pulseCounter);

	if (GetSignals(&stat, &sig0, &sig1) == 0) {
		efiPrintf("Last valid fast msg Status 0x%01x Sig0 0x%03x Sig1 0x%03x", stat, sig0, sig1);
	}

	if (scMsgFlags) {
		efiPrintf("Slow channels:");
		for (int i = 0; i < SENT_SLOW_CHANNELS_MAX; i++) {
			if (scMsgFlags & BIT(i)) {
				efiPrintf(" ID %d: %d", scMsg[i].id, scMsg[i].data);
			}
		}
	}

	#if SENT_STATISTIC_COUNTERS
		efiPrintf("HW overflows %lu\n", statistic.hwOverflowCnt);

		efiPrintf("Pause pulses %lu\n", statistic.PauseCnt);
		efiPrintf("Restarts %lu", statistic.RestartCnt);
		efiPrintf("Interval errors %lu short, %lu long", statistic.ShortIntervalErr, statistic.LongIntervalErr);
		efiPrintf("Total frames %lu with CRC error %lu (%f%%)", statistic.FrameCnt, statistic.CrcErrCnt, statistic.CrcErrCnt * 100.0 / statistic.FrameCnt);
		efiPrintf("Total slow channel messages %lu with crc6 errors %lu (%f%%)", statistic.sc, statistic.scCrcErr, statistic.scCrcErr * 100.0 / statistic.sc);
		efiPrintf("Sync errors %lu", statistic.SyncErr);
	#endif
}
#endif

/*==========================================================================*/
/* ICU driver.																*/
/*==========================================================================*/
/* This SENT HW driver is based on ChibiOS ICU driver */
#if (HAL_USE_ICU == TRUE)

/* TODO: do we care about scaling abstract timer ticks to some time base? */
/* TODO: get at runtime */
/* Max timer clock for most timers on STM32 is CPU clock / 2 */
#define SENT_TIMER_CLOCK_DIV	2
/* TODO: not all timers are clocked from STM32_TIMCLK1 */
#define SENT_ICU_FREQ			(STM32_TIMCLK1 / SENT_TIMER_CLOCK_DIV)

static uint16_t lastPulse[SENT_INPUT_COUNT];
static bool overcapture[SENT_INPUT_COUNT];

static void icuperiodcb(ICUDriver *icup, size_t index)
{
	uint16_t clocks;
	uint8_t flags = 0;
	const ICUConfig *icucfg = icup->config;

	if ((icucfg->channel == ICU_CHANNEL_1) || (icucfg->channel == ICU_CHANNEL_2)) {
		/* channel 1 and channel 2 supports period measurements */
		clocks = icuGetPeriodX(icup);
	} else {
		/* this is freerunnig timer and we need to calculate period using just captured timer value and previous one */
		/* TODO: support 32 bit timers too? */
		uint16_t val = icuGetWidthX(icup);

		/* can overflow */
		clocks = val - lastPulse[index];

		lastPulse[index] = val;
	}

	if (overcapture[index]) {
		flags |= SENT_FLAG_HW_OVERFLOW;
		overcapture[index] = false;
	}

	SENT_ISR_Handler(index, clocks, flags);
}

//static void icuovercapture(ICUDriver *icup, size_t index)
//{
//	overcapture[index] = true;
//}

/* ICU callbacks */
static void icuperiodcb_in1(ICUDriver *icup)
{
	icuperiodcb(icup, 0);
}

//static void icuovercapture_in1(ICUDriver *icup)
//{
//	icuovercapture(icup, 0);
//}

static void icuperiodcb_in2(ICUDriver *icup)
{
	icuperiodcb(icup, 1);
}

//static void icuovercapture_in2(ICUDriver *icup)
//{
//	icuovercapture(icup, 1);
//}

/* ICU configs */
static /* const */ ICUConfig icucfg[SENT_INPUT_COUNT] =
{
#if (STM32_ICU_USE_TIM4 == TRUE)
	/* IN_DIN4 -> DIN4 -> PB6 -> TIM4_CH1 */
	{
		.mode = ICU_INPUT_ACTIVE_LOW,
		.frequency = SENT_ICU_FREQ,
		.width_cb = NULL,
		.period_cb = icuperiodcb_in1,
		.overflow_cb = NULL,
		.channel = ICU_CHANNEL_1,
		.dier = 0U,
		.arr = 0xFFFFFFFFU,
		//.overcapture_cb = icuovercapture_in1,
	},
#endif
#if (STM32_ICU_USE_TIM2 == TRUE)
	/* IN_A0 -> A0 -> PA1 -> TIM5_CH2/TIM2_CH2 */
	{
		.mode = ICU_INPUT_ACTIVE_LOW,
		.frequency = SENT_ICU_FREQ,
		.width_cb = NULL,
		.period_cb = icuperiodcb_in2,
		.overflow_cb = NULL,
		.channel = ICU_CHANNEL_2,
		.dier = 0U,
		.arr = 0xFFFFFFFFU,
		//.overcapture_cb = icuovercapture_in2,
	}
#endif
};

static ICUDriver *icudrivers[SENT_INPUT_COUNT] =
{
#if (STM32_ICU_USE_TIM4 == TRUE)
	&ICUD4,
#else
	nullptr,
#endif

#if (STM32_ICU_USE_TIM2 == TRUE)
	&ICUD2,
#else
	nullptr,
#endif
};

void startSent() {
#if (STM32_ICU_USE_TIM4 == TRUE)
	palSetPadMode(GPIOB, 6, PAL_MODE_INPUT);
#endif
#if (STM32_ICU_USE_TIM2 == TRUE)
	palSetPadMode(GPIOA, 1, PAL_MODE_INPUT);
#endif

	for (int i = 0; i < SENT_INPUT_COUNT; i++) {
		ICUConfig *cfg = &icucfg[i];
		ICUDriver *icu = icudrivers[i];

		if (icu == nullptr) {
			continue;
		}

		icuStart(icu, cfg);
		icuStartCapture(icu);
		icuEnableNotifications(icu);
	}
}
#endif //HAL_USE_ICU

/*==========================================================================*/
/* Decoder thread settings.													*/
/*==========================================================================*/

/* 4 per channel should be enough */
#define SENT_MB_SIZE		(4 * SENT_INPUT_COUNT)

static msg_t sent_mb_buffer[SENT_MB_SIZE];
static MAILBOX_DECL(sent_mb, sent_mb_buffer, SENT_MB_SIZE);

static THD_WORKING_AREA(waSentDecoderThread, 256);

void SENT_ISR_Handler(uint8_t channel, uint16_t clocks, uint8_t flags) {
	/* encode to fit msg_t */
	msg_t msg = (flags << 24) | (channel << 16) | clocks;

	/* called from ISR */
	chSysLockFromISR();
	chMBPostI(&sent_mb, msg);
	chSysUnlockFromISR();
}

static void SentDecoderThread(void*) {
	while (true) {
		msg_t ret;
		msg_t msg;

		ret = chMBFetchTimeout(&sent_mb, &msg, TIME_INFINITE);

		if (ret == MSG_OK) {
			uint16_t tick = msg & 0xffff;
			uint8_t n = (msg >> 16) & 0xff;
			uint8_t flags = (msg >> 24) & 0xff;

			if (n < SENT_INPUT_COUNT) {
				sent_channel &channel = channels[n];

				if (channel.Decoder(tick, flags) > 0) {
					/* Call high level decoder from here */
					/* TODO: implemnet subscribers, like it is done for ADC */
				}
			}
		}
	}
}

/* Should be called once */
void initSent(void) {
	/* init interval mailbox */
	chMBObjectInit(&sent_mb, sent_mb_buffer, SENT_MB_SIZE);

	chThdCreateStatic(waSentDecoderThread, sizeof(waSentDecoderThread), NORMALPRIO, SentDecoderThread, nullptr);

	/* Start HW layer */
	startSent();
}

int getSentValues(size_t index, uint16_t *sig0, uint16_t *sig1) {
	if (index < SENT_INPUT_COUNT) {
		sent_channel &channel = channels[index];

		return channel.GetSignals(NULL, sig0, sig1);
	}

	/* invalid channel */
    return -1;
}
