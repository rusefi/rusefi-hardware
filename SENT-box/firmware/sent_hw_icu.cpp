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
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,
  NULL,
  icuperiodcb_in1,
  NULL,
  ICU_CHANNEL_1,
  0U,
  0xFFFFFFFFU
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

void InitSent()
{

    icuStart(&SENT_ICUD_CH1_D, &icucfg_in1);
    icuStartCapture(&SENT_ICUD_CH1_D);
    icuEnableNotifications(&SENT_ICUD_CH1_D);

    icuStart(&SENT_ICUD_CH2_D, &icucfg_in2);
    icuStartCapture(&SENT_ICUD_CH2_D);
    icuEnableNotifications(&SENT_ICUD_CH2_D);

#if SENT_DEV == SENT_SILABS_SENS
    icuStart(&SENT_ICUD_CH3_D, &icucfg_in3);
    icuStartCapture(&SENT_ICUD_CH3_D);
    icuEnableNotifications(&SENT_ICUD_CH3_D);

    icuStart(&SENT_ICUD_CH4_D, &icucfg_in4);
    icuStartCapture(&SENT_ICUD_CH4_D);
    icuEnableNotifications(&SENT_ICUD_CH4_D);
#endif
}

#endif // SENT_MODE_ICU