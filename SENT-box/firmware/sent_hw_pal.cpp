/*
 * sent.cpp
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#include "ch.h"
#include "hal.h"

#include "sent.h"
#include "sent_hw_pal.h"

#if SENT_MODE_PAL

uint32_t cyccnt_ch1;
uint32_t cyccnt_ch1_prev = 0;
uint16_t cyccnt_ch1_period;

uint32_t cyccnt_ch2;
uint32_t cyccnt_ch2_prev = 0;
uint16_t cyccnt_ch2_period;

uint32_t dwtTestValMax = 0;
uint32_t dwtTestValMin = 40000;

extern uint32_t sentMinIntervalErr;
extern uint32_t sentMaxIntervalErr;

extern uint8_t sentCh1DataReady;
extern uint8_t sentCh2DataReady;

uint8_t SENT_GetTickValue(uint16_t dwt_val);

#pragma GCC push_options
#pragma GCC optimize ("O2")
static void palperiodcb_in1(void *arg)
{

  (void)arg;

  cyccnt_ch1 = DWT->CYCCNT;

  if(cyccnt_ch1 > cyccnt_ch1_prev)
  {
      cyccnt_ch1_period = (uint16_t)((cyccnt_ch1 - cyccnt_ch1_prev));
  }
  else
  {
      cyccnt_ch1_period = (uint16_t)((0xFFFFFFFF - cyccnt_ch1_prev + cyccnt_ch1));
  }

  cyccnt_ch1_prev = cyccnt_ch1;

#if SENT_DEV == SENT_GM_ETB
  SENT_ResetRawDataProp();
#endif

  SENT_ISR_Handler(SENT_CH1, SENT_GetTickValue(cyccnt_ch1_period));
}

static void palperiodcb_in2(void *arg)
{

  (void)arg;

  cyccnt_ch2 = DWT->CYCCNT;

  if(cyccnt_ch2 > cyccnt_ch2_prev)
  {
    cyccnt_ch2_period = (uint16_t)((cyccnt_ch2 - cyccnt_ch2_prev));
  }
  else
  {
    cyccnt_ch2_period = (uint16_t)((0xFFFFFFFF - cyccnt_ch2_prev + cyccnt_ch2));
  }

  cyccnt_ch2_prev = cyccnt_ch2;

#if SENT_DEV == SENT_GM_ETB
  SENT_SetRawDataProp();
#endif

  SENT_ISR_Handler(SENT_CH2, SENT_GetTickValue(cyccnt_ch2_period));
}
#pragma GCC pop_options

void InitSent()
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;

  palSetLineMode(HAL_SENT_CH1_LINE, PAL_MODE_INPUT_PULLUP);
  palEnableLineEvent(HAL_SENT_CH1_LINE, PAL_EVENT_MODE_FALLING_EDGE);
  palSetLineCallback(HAL_SENT_CH1_LINE, (palcallback_t)palperiodcb_in1, NULL);

  palSetLineMode(HAL_SENT_CH2_LINE, PAL_MODE_INPUT_PULLUP);
  palEnableLineEvent(HAL_SENT_CH2_LINE, PAL_EVENT_MODE_FALLING_EDGE);
  palSetLineCallback(HAL_SENT_CH2_LINE, (palcallback_t)palperiodcb_in2, NULL);
}

#pragma GCC push_options
#pragma GCC optimize ("O2")
uint8_t SENT_GetTickValue(uint16_t dwt_val)
{
#if SENT_DEV == SENT_GM_ETB
      return (dwt_val/CLOCK_TICK);
#else
        if((dwt_val > SENT_56_TICKS_MIN) && (dwt_val < SENT_56_TICKS_MAX))
        {
            return SENT_SYNC_VAL_TICKS;
        }
        else
        {
            if((dwt_val > SENT_12_TICKS_MIN) && (dwt_val < SENT_12_TICKS_MAX))
            {
                return SENT_0_VAL_TICKS;
            }
            else
            {
                if((dwt_val > SENT_13_TICKS_MIN) && (dwt_val < SENT_13_TICKS_MAX))
                {
                    return SENT_1_VAL_TICKS;
                }
                else
                {
                    if((dwt_val > SENT_14_TICKS_MIN) && (dwt_val < SENT_14_TICKS_MAX))
                    {
                        return SENT_2_VAL_TICKS;
                    }
                    else
                    {
                        if((dwt_val > SENT_15_TICKS_MIN) && (dwt_val < SENT_15_TICKS_MAX))
                        {
                            return SENT_3_VAL_TICKS;
                        }
                        else
                        {
                            if((dwt_val > SENT_16_TICKS_MIN) && (dwt_val < SENT_16_TICKS_MAX))
                            {
                                return SENT_4_VAL_TICKS;
                            }
                            else
                            {
                                if((dwt_val > SENT_17_TICKS_MIN) && (dwt_val < SENT_17_TICKS_MAX))
                                {
                                    return SENT_5_VAL_TICKS;
                                }
                                else
                                {
                                    if((dwt_val > SENT_18_TICKS_MIN) && (dwt_val < SENT_18_TICKS_MAX))
                                    {
                                        return SENT_6_VAL_TICKS;
                                    }
                                    else
                                    {
                                        if((dwt_val > SENT_19_TICKS_MIN) && (dwt_val < SENT_19_TICKS_MAX))
                                        {
                                            return SENT_7_VAL_TICKS;
                                        }
                                        else
                                        {
                                            if((dwt_val > SENT_20_TICKS_MIN) && (dwt_val < SENT_20_TICKS_MAX))
                                            {
                                                return SENT_8_VAL_TICKS;
                                            }
                                            else
                                            {
                                                if((dwt_val > SENT_21_TICKS_MIN) && (dwt_val < SENT_21_TICKS_MAX))
                                                {
                                                    return SENT_9_VAL_TICKS;
                                                }
                                                else
                                                {
                                                    if((dwt_val > SENT_22_TICKS_MIN) && (dwt_val < SENT_22_TICKS_MAX))
                                                    {
                                                        return SENT_10_VAL_TICKS;
                                                    }
                                                    else
                                                    {
                                                        if((dwt_val > SENT_23_TICKS_MIN) && (dwt_val < SENT_23_TICKS_MAX))
                                                        {
                                                            return SENT_11_VAL_TICKS;
                                                        }
                                                        else
                                                        {
                                                            if((dwt_val > SENT_24_TICKS_MIN) && (dwt_val < SENT_24_TICKS_MAX))
                                                            {
                                                                return SENT_12_VAL_TICKS;
                                                            }
                                                            else
                                                            {
                                                                if((dwt_val > SENT_25_TICKS_MIN) && (dwt_val < SENT_25_TICKS_MAX))
                                                                {
                                                                    return SENT_13_VAL_TICKS;
                                                                }
                                                                else
                                                                {
                                                                    if((dwt_val > SENT_26_TICKS_MIN) && (dwt_val < SENT_26_TICKS_MAX))
                                                                    {
                                                                        return SENT_14_VAL_TICKS;
                                                                    }
                                                                    else
                                                                    {
                                                                        if((dwt_val > SENT_27_TICKS_MIN) && (dwt_val < SENT_27_TICKS_MAX))
                                                                        {
                                                                           return SENT_15_VAL_TICKS;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return 0xFF;
#endif
}
#pragma GCC pop_options

#endif // SENT_MODE_PAL