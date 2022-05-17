/*
 * sent.cpp
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#include "ch.h"
#include "hal.h"

#include "sent.h"

// Sent SM status arr
SM_SENT_enum sentSMstate[SENT_CHANNELS_NUM] = {SM_SENT_INIT_STATE};

// Value sensor arr
uint16_t sentValArr[SENT_CHANNELS_NUM] = {0};

//
uint16_t sentTempValArr[SENT_CHANNELS_NUM] = {0};

// Received nibbles arr
uint8_t sentTempNibblArr[SENT_CHANNELS_NUM][SENT_MSG_PAYLOAD_SIZE] = {0};

// Sensor status arr
uint8_t sentStat[SENT_CHANNELS_NUM] = {0};

// Roll counter arr
uint8_t sentRollCnt[SENT_CHANNELS_NUM] = {0};
uint8_t sentRollCntPrev[SENT_CHANNELS_NUM] = {0};

// Error counters
uint32_t sentIntervalErr = 0;
uint32_t sentSyncErr = 0;
uint32_t sentStatusErr = 0;
uint32_t sentRollErrCnt = 0;
uint32_t sentCrcErrCnt = 0;

static const uint8_t SENT_CRC4_tbl[16] =
{
        0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5
};

#if SENT_ERR_PERCENT
// Received msg counter
uint32_t val_res_cnt = 0;

float err_per = 0;
#endif

static void icuperiodcb_in1(ICUDriver *icup);
static void icuperiodcb_in2(ICUDriver *icup);
static void icuperiodcb_in3(ICUDriver *icup);

static void SENT_ISR_Handler(uint8_t ch, uint16_t val_res);
uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata);

// Sent input1 - TIM4 CH1 - PB6
static ICUConfig icucfg_in1 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,                                    /* 400kHz ICU clock frequency - 2.5 us.   */
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
  SENT_ICU_FREQ,                                    /* 400kHz ICU clock frequency - 2.5 us.   */
  NULL,
  icuperiodcb_in2,
  NULL,
  ICU_CHANNEL_1,
  0U,
  0xFFFFFFFFU
};

// Sent input3 - TIM1 CH1 - PA8
static ICUConfig icucfg_in3 =
{
  ICU_INPUT_ACTIVE_HIGH,
  SENT_ICU_FREQ,                                    /* 400kHz ICU clock frequency - 2.5 us.   */
  NULL,
  icuperiodcb_in3,
  NULL,
  ICU_CHANNEL_1,
  0U,
  0xFFFFFFFFU
};

static void icuperiodcb_in1(ICUDriver *icup)
{
  SENT_ISR_Handler(SENT_CH1, icuGetPeriodX(icup));
}

static void icuperiodcb_in2(ICUDriver *icup)
{
  SENT_ISR_Handler(SENT_CH2, icuGetPeriodX(icup));
}

static void icuperiodcb_in3(ICUDriver *icup)
{
  SENT_ISR_Handler(SENT_CH3, icuGetPeriodX(icup));
}

void InitSent()
{

    icuStart(&SENT_ICUD_CH1, &icucfg_in1);
    icuStartCapture(&SENT_ICUD_CH1);
    icuEnableNotifications(&SENT_ICUD_CH1);

    icuStart(&SENT_ICUD_CH2, &icucfg_in2);
    icuStartCapture(&SENT_ICUD_CH2);
    icuEnableNotifications(&SENT_ICUD_CH2);

    icuStart(&SENT_ICUD_CH3, &icucfg_in3);
    icuStartCapture(&SENT_ICUD_CH3);
    icuEnableNotifications(&SENT_ICUD_CH3);

}

static void SENT_ISR_Handler(uint8_t ch, uint16_t val_res)
{
        val_res >>= 1;

        if(val_res < SENT_MIN_INTERVAL)
        {// sent interval less than min interval
            sentIntervalErr++;
        }
        else
        {
            val_res -= SENT_OFFSET_INTERVAL;

            if((val_res != SENT_SYNC_INTERVAL) && (val_res > SENT_MAX_INTERVAL))
            {// sent interval more than max interval
                sentIntervalErr++;
            }
            else
            {
                switch(sentSMstate[ch])
                {
                    case SM_SENT_INIT_STATE:
    #if SENT_ERR_PERCENT
                        val_res_cnt++;
    #endif

                        if(val_res == SENT_SYNC_INTERVAL)
                        {// sync interval - 56 ticks
                                sentSMstate[ch] = SM_SENT_STATUS_STATE;
                        }
                        break;

                    case SM_SENT_SYNC_STATE:
    #if SENT_ERR_PERCENT
                        val_res_cnt++;
    #endif

                        if(val_res == SENT_SYNC_INTERVAL)
                        {// sync interval - 56 ticks
                            sentTempValArr[ch] = 0;

                            sentSMstate[ch] = SM_SENT_STATUS_STATE;
                        }
                        else
                        {
                            //  Increment sync interval err count
                            sentSyncErr++;

    #if SENT_ERR_PERCENT
                            // Calc err percentage
                            err_per = ((float)val_res_err_cnt/(float)val_res_cnt)*100;
    #endif
                        }
                        break;

                    case SM_SENT_STATUS_STATE:
                        // status interval
                        sentStat[ch] = val_res;

                        if(sentStat[ch])
                        {
                                sentStatusErr++;
                        }

                        sentSMstate[ch] = SM_SENT_SIG1_DATA1_STATE;
                        break;

                    case SM_SENT_SIG1_DATA1_STATE:
                        sentTempNibblArr[ch][0] = val_res;

                        sentTempValArr[ch] = ((uint16_t)(val_res) << 8);

                        sentSMstate[ch] = SM_SENT_SIG1_DATA2_STATE;
                        break;

                    case SM_SENT_SIG1_DATA2_STATE:
                        sentTempNibblArr[ch][1] = val_res;

                        sentTempValArr[ch] |= ((uint16_t)(val_res) << 4);

                        sentSMstate[ch] = SM_SENT_SIG1_DATA3_STATE;
                        break;

                    case SM_SENT_SIG1_DATA3_STATE:
                        sentTempNibblArr[ch][2] = val_res;

                        sentTempValArr[ch] |= (val_res);

                        sentSMstate[ch] = SM_SENT_SIG2_DATA1_STATE;
                        break;

                    case SM_SENT_SIG2_DATA1_STATE:
                        sentTempNibblArr[ch][3] = val_res;

                        sentRollCnt[ch] = ((uint8_t)(val_res) << 4);

                        sentSMstate[ch] = SM_SENT_SIG2_DATA2_STATE;
                        break;

                    case SM_SENT_SIG2_DATA2_STATE:
                        sentTempNibblArr[ch][4] = val_res;

                        sentRollCnt[ch] |= ((uint8_t)(val_res));

                        sentSMstate[ch] = SM_SENT_SIG2_DATA3_STATE;
                        break;

                    case SM_SENT_SIG2_DATA3_STATE:
                        sentTempNibblArr[ch][5] = val_res;

                        sentSMstate[ch] = SM_SENT_CRC_STATE;
                        break;

                    case SM_SENT_CRC_STATE:
                        // Check msg number
                        if(sentRollCnt[ch] != (uint8_t)(sentRollCntPrev[ch] + 1))
                        {// Msg lost
                            //  Increment msg number err count
                            sentRollErrCnt++;
                        }

                        sentRollCntPrev[ch] = sentRollCnt[ch];

                        // Check crc
                        if((uint8_t)(val_res) == sent_crc4(sentTempNibblArr[ch], SENT_MSG_PAYLOAD_SIZE))
                        {
                            sentValArr[ch] = sentTempValArr[ch];
                        }
                        else
                        {
                          //  Increment crc err count
                            sentCrcErrCnt++;
                        }

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                        break;
                  }
            }
        }
}

uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata)
{
        uint8_t crc;
        uint16_t i;
        crc = SENT_CRC_SEED; // Seed.

        for(i=0; i < ndata; i++)
        {
                crc = *pdata++ ^ SENT_CRC4_tbl[crc]; // Data[#1 to #6]
        }

        return SENT_CRC4_tbl[crc];
}

uint16_t SENT_GetData(uint8_t ch)
{
        return sentValArr[ch];
}

uint16_t SENT_GetRollErrCnt(void)
{
        return sentRollErrCnt;
}

uint16_t SENT_GetCrcErrCnt(void)
{
        return sentCrcErrCnt;
}

uint16_t SENT_GetIntervalErrCnt(void)
{
        return sentIntervalErr;
}

uint16_t SENT_GetSyncErrCnt(void)
{
        return sentSyncErr;
}
