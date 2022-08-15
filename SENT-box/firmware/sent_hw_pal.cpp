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

uint32_t cyccnt_ch_prev[SENT_CH_MAX] = {0};

#pragma GCC push_options
#pragma GCC optimize ("O2")

static void palperiodcb_in(void *arg)
{
  uint32_t cyccnt_ch;
  uint16_t cyccnt_ch_period;
  int ch = (int)arg;

  cyccnt_ch = port_rt_get_counter_value();

  if(cyccnt_ch > cyccnt_ch_prev[ch])
  {
      cyccnt_ch_period = (uint16_t)((cyccnt_ch - cyccnt_ch_prev[ch]));
  }
  else
  {
      cyccnt_ch_period = (uint16_t)((0xFFFFFFFF - cyccnt_ch_prev[ch] + cyccnt_ch));
  }

  cyccnt_ch_prev[ch] = cyccnt_ch;

  SENT_ISR_Handler(ch, cyccnt_ch_period);
}
#pragma GCC pop_options

void InitSent()
{
  palSetLineMode(HAL_SENT_CH1_LINE, PAL_MODE_INPUT_PULLUP);
  palEnableLineEvent(HAL_SENT_CH1_LINE, PAL_EVENT_MODE_FALLING_EDGE);
  palSetLineCallback(HAL_SENT_CH1_LINE, (palcallback_t)palperiodcb_in, (void *)SENT_CH1);

  palSetLineMode(HAL_SENT_CH2_LINE, PAL_MODE_INPUT_PULLUP);
  palEnableLineEvent(HAL_SENT_CH2_LINE, PAL_EVENT_MODE_FALLING_EDGE);
  palSetLineCallback(HAL_SENT_CH2_LINE, (palcallback_t)palperiodcb_in, (void *)SENT_CH1);

  /* Start decoder thread */
  SentDecoder_Init();
}

#endif // SENT_MODE_PAL