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

#if SENT_MODE_ICU

static void icuperiodcb_in1(ICUDriver *icup);
static void icuperiodcb_in2(ICUDriver *icup);
#if SENT_DEV == SENT_SILABS_SENS
static void icuperiodcb_in3(ICUDriver *icup);
static void icuperiodcb_in4(ICUDriver *icup);
#endif

static ICUConfig icucfg_in1 =
{
  .mode = ICU_INPUT_ACTIVE_LOW,
  .frequency = SENT_ICU_FREQ,
  .width_cb = NULL,
  .period_cb = icuperiodcb_in1,
  .overflow_cb = NULL,
  .channel = SENT_ICUD_CH1_CH,
  .dier = 0U,
  .arr = 0xFFFFFFFFU,
};

static ICUConfig icucfg_in2 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in2,
  NULL,
  SENT_ICUD_CH2_CH,
  0U,
  0xFFFFFFFFU
};

#if SENT_DEV == SENT_SILABS_SENS
static ICUConfig icucfg_in3 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in3,
  NULL,
  SENT_ICUD_CH3_CH,
  0U,
  0xFFFFFFFFU
};

static ICUConfig icucfg_in4 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in4,
  NULL,
  SENT_ICUD_CH4_CH,
  0U,
  0xFFFFFFFFU
};
#endif // SENT_DEV

static void icuperiodcb_in1(ICUDriver *icup)
{
  SENT_ISR_Handler(SENT_CH1, icuGetPeriodX(icup));
}

static void icuperiodcb_in2(ICUDriver *icup)
{
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

void InitSent()
{

    //palSetLineMode(HAL_SENT_CH1_LINE, PAL_MODE_INPUT_PULLUP);
    //palSetLineMode(HAL_SENT_CH2_LINE, PAL_MODE_INPUT_PULLUP);

    icuStart(&SENT_ICUD_CH1_D, &icucfg_in1);
    icuStartCapture(&SENT_ICUD_CH1_D);
    icuEnableNotifications(&SENT_ICUD_CH1_D);

#if 0
    icuStart(&SENT_ICUD_CH2_D, &icucfg_in2);
    icuStartCapture(&SENT_ICUD_CH2_D);
    icuEnableNotifications(&SENT_ICUD_CH2_D);
#endif

#if SENT_DEV == SENT_SILABS_SENS
    icuStart(&SENT_ICUD_CH3_D, &icucfg_in3);
    icuStartCapture(&SENT_ICUD_CH3_D);
    icuEnableNotifications(&SENT_ICUD_CH3_D);

    icuStart(&SENT_ICUD_CH4_D, &icucfg_in4);
    icuStartCapture(&SENT_ICUD_CH4_D);
    icuEnableNotifications(&SENT_ICUD_CH4_D);
#endif

  /* Start decoder thread */
  SentDecoder_Init();
}

#endif // SENT_MODE_ICU