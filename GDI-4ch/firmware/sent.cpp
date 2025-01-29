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
#include "chprintf.h"

#include "sent.h"

#ifndef SENT_INPUT_COUNT
#define SENT_INPUT_COUNT		4 // Number of sent channels
#endif

static sent_channel channels[SENT_INPUT_COUNT];

/*==========================================================================*/
/* Misc helpers.															*/
/*==========================================================================*/
#define BIT(n) (UINT32_C(1) << (n))

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
/* Forward declarations.													*/
/*==========================================================================*/

static int GmPressureDecode(size_t index);
static void GmPressureDebug(void);

/*==========================================================================*/
/* Debug.																	*/
/*==========================================================================*/
extern BaseSequentialStream *chp;

void sent_channel::Info() {
	uint8_t stat;
	uint16_t sig0, sig1;

	chprintf(chp, "Unit time %lu timer ticks\r\n", tickPerUnit);
	chprintf(chp, "Pause pulse detected %s\r\n", pausePulseReceived ? "Yes" : "No");
	chprintf(chp, "Total pulses %lu\r\n", pulseCounter);

	if (GetSignals(&stat, &sig0, &sig1) == 0) {
		chprintf(chp, "Last valid fast msg Status 0x%01x Sig0 0x%03x Sig1 0x%03x\r\n", stat, sig0, sig1);
	}

	chprintf(chp, "Slow channels:\r\n");
	for (int i = 0; i < SENT_SLOW_CHANNELS_MAX; i++) {
		if (scMsg[i].valid) {
			chprintf(chp, " %d: ID %d: %d (0x%x)\r\n", i, scMsg[i].id, scMsg[i].data, scMsg[i].data);
		}
	}

	#if SENT_STATISTIC_COUNTERS
	    static int aliveCounter = 0;
		chprintf(chp, "HW overflows %lu\r\n", statistic.hwOverflowCnt);

		chprintf(chp, "Pause pulses %lu\r\n", statistic.PauseCnt);
		chprintf(chp, "Restarts %lu\r\n", statistic.RestartCnt);
		chprintf(chp, "Interval errors %lu short, %lu long\r\n", statistic.ShortIntervalErr, statistic.LongIntervalErr);
		chprintf(chp, "Total frames %lu with CRC error %lu (%d%%)\r\n", statistic.FrameCnt, statistic.CrcErrCnt, 100 * statistic.CrcErrCnt / statistic.FrameCnt);
		chprintf(chp, "Total slow channel messages %lu + %lu with crc6 errors %lu (%d%%)\r\n", statistic.sc12, statistic.sc16, statistic.scCrcErr, 100 * statistic.scCrcErr / (statistic.sc12 + statistic.sc16));
		chprintf(chp, "Sync errors %lu alive=%d\r\n", statistic.SyncErr, aliveCounter++);
	#endif
	GmPressureDebug();
}

void sentDebug(void)
{
	for (int i = 0; i < SENT_INPUT_COUNT; i++) {
		if (icudrivers[i] == nullptr)
			continue;

		sent_channel &channel = channels[i];

		chprintf(chp, "---- SENT input %d ----\r\n", i);
		channel.Info();
		chprintf(chp, "--------------------\r\n");
	}
}

/*==========================================================================*/
/* Decoder feed handler and mailbox											*/
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

/*==========================================================================*/
/* Decoder thread.															*/
/*==========================================================================*/
static void SentDecoderThread(void*) {
	chRegSetThreadName("SEND RX");

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
					GmPressureDecode(n);
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

int getSentSlowChannelValue(size_t index, size_t id)
{
	if (index < SENT_INPUT_COUNT) {
		sent_channel &channel = channels[index];

		return channel.GetSlowChannelValue(id);
	}

	/* invalid channel */
	return -1;
}

/*==========================================================================*/
/* Sensor handler(s)														*/
/*==========================================================================*/

static bool gm_valid = false;
static float gm_pres = 0.0;
static float gm_temp = -273.0;

static int GmPressureDecode(size_t index)
{
	int t1, t2;

	/* Slow channels 16 and 22 transmit temperature */
	t1 = getSentSlowChannelValue(index, 16);
	t2 = getSentSlowChannelValue(index, 22);

	if ((t1 < 0) || (t2 < 0)) {
		/* this is not GM pressure sensor or slow channels is not received yet */
		return 0;
	}

	/* TODO: this is just a guess */
	/* average, 8 per degree C, 65 offset */
	gm_temp = (((float)t1 + (float)t2) / 2.0 - 512.0) / 8.0;

	/*
     * Sig0 shows about 197..198 at 1 Atm (open air) and 282 at 1000 KPa (9.86 Atm)
     * Sig1 shows about 202..203 at 1 Atm (open air) and 283 at 1000 KPa (9.86 Atm)
     * So for 8.86 Atm delta there are:
     * 84..85 units for sig0
     * 80..81 units for sig1
     * Measurements are not ideal, so lets ASSUME 10 units per 1 Atm
     * Offset is 187..188 for Sig0 and 192..193 for Sig1.
     * Average offset is 190 for 0 Atm.
     */
	uint16_t sig0, sig1;
	getSentValues(index, &sig0, &sig1);

	gm_pres = (((float)sig0 + (float)sig1) / 2 - 190) / 10.0;

	/* TODO: timestamp value */

	gm_valid = true;

	return 1;
}

static void GmPressureDebug(void)
{
	if (!gm_valid)
		return;

	/* We can not print float */
	int pres = gm_pres * 1000;
	int temp = gm_temp * 1000;

	chprintf(chp, "GM: press %d.%03d, temp %d.%03d\r\n", pres / 1000, pres % 1000, temp / 1000, temp % 1000);
}

float GmPressureGetPressure(void)
{
	return gm_pres;
}

float GmPressureGetTemperature(void)
{
	return gm_temp;
}
