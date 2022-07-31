/*
 * sent.cpp
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#include "ch.h"
#include "hal.h"

#include "sent.h"
#include "sent_hw_icu.h"

static void icuperiodcb_in1(ICUDriver *icup);
static void icuperiodcb_in2(ICUDriver *icup);
#if SENT_DEV == SENT_SILABS_SENS
static void icuperiodcb_in3(ICUDriver *icup);
static void icuperiodcb_in4(ICUDriver *icup);
#endif

// Sent input1 - TIM4 CH1 - PB6
static ICUConfig icucfg_in1 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in1,
  NULL,
  ICU_CHANNEL_1,
  0U,
  0xFFFFFFFFU
};

// Sent input2 - TIM3 CH1 - PA6
static ICUConfig icucfg_in2 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in2,
  NULL,
  ICU_CHANNEL_1,
  0U,
  0xFFFFFFFFU
};

#if SENT_DEV == SENT_SILABS_SENS
// Sent input3 - TIM1 CH1 - PA8
static ICUConfig icucfg_in3 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in3,
  NULL,
  ICU_CHANNEL_1,
  0U,
  0xFFFFFFFFU
};

// Sent input4 - TIM2 CH2 - PA1
static ICUConfig icucfg_in4 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in4,
  NULL,
  ICU_CHANNEL_2,
  0U,
  0xFFFFFFFFU
};
#endif

static void icuperiodcb_in1(ICUDriver *icup)
{
#if SENT_DEV == SENT_GM_ETB
  SENT_ResetRawDataProp();
#endif
  SENT_ISR_Handler(SENT_CH1, icuGetPeriodX(icup));
}

static void icuperiodcb_in2(ICUDriver *icup)
{
#if SENT_DEV == SENT_GM_ETB
  SENT_SetRawDataProp();
#endif

  SENT_ISR_Handler(SENT_CH2, icuGetPeriodX(icup));
}

#if SENT_DEV == SENT_SILABS_SENS
static void icuperiodcb_in3(ICUDriver *icup)
{
  SENT_ISR_Handler(SENT_CH3, icuGetPeriodX(icup) >> 1);
}

static void icuperiodcb_in4(ICUDriver *icup)
{
  SENT_ISR_Handler(SENT_CH4, icuGetPeriodX(icup) >> 1);
}
#endif

void InitSentHwIcu()
{

    icuStart(&SENT_ICUD_CH1, &icucfg_in1);
    icuStartCapture(&SENT_ICUD_CH1);
    icuEnableNotifications(&SENT_ICUD_CH1);

    icuStart(&SENT_ICUD_CH2, &icucfg_in2);
    icuStartCapture(&SENT_ICUD_CH2);
    icuEnableNotifications(&SENT_ICUD_CH2);

#if SENT_DEV == SENT_SILABS_SENS
    icuStart(&SENT_ICUD_CH3, &icucfg_in3);
    icuStartCapture(&SENT_ICUD_CH3);
    icuEnableNotifications(&SENT_ICUD_CH3);

    icuStart(&SENT_ICUD_CH4, &icucfg_in4);
    icuStartCapture(&SENT_ICUD_CH4);
    icuEnableNotifications(&SENT_ICUD_CH4);
#endif
}

