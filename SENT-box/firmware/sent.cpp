/*
 * sent.cpp
 *
 *  Created on: 16 мая 2022 г.
 *      Author: alexv
 */

#include "ch.h"
#include "hal.h"

#include "sent.h"

// Массив состояний СМ приема посылки
SM_SENT_enum sentSMstate[4] = {SM_SENT_INIT_STATE};

// Массив значений датчиков
uint16_t sentValArr[4] = {0};

// Массив для накопления значений датчиков
uint16_t sentTempValArr[4] = {0};

// Массив принятых полубайтов (для вычисления crc)
uint8_t sentTempNibblArr[4][6] = {0};

// Массив битов состояния датчиков
uint8_t sentStat[4] = {0};

// Массив номеров посылок
uint8_t sentRollCnt[4] = {0};
uint8_t sentRollCntPrev[4] = {0};

// Массив значений таймера предыдущего фронта (для вычисления длительности импульса)
uint16_t val_prev[4];

// Счетчик принятых посылок
uint32_t val_res_cnt = 0;

// Счетчик ошибок
uint32_t val_res_err_cnt = 0;
uint32_t val_res_err_cnt1 = 0;

uint32_t sentStatusErr = 0;
uint32_t sentRollErrCnt = 0;
uint32_t sentCrcErrCnt = 0;

float err_per = 0;

uint8_t val_min_cnt = 0;
uint8_t val_max_cnt = 0;

uint16_t sentMaxSyncVal = 0;
uint16_t sentMinSyncVal = 50000;

// Флаг готовности данных дл япередачи
uint8_t senDataReady = 0;

static void icuperiodcb(ICUDriver *icup);

static void SENT_ISR_Handler(uint8_t ch, uint16_t val);
uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata);

static ICUConfig icucfg =
{
  ICU_INPUT_ACTIVE_HIGH,
  400000,                                    /* 400kHz ICU clock frequency - 2.5 us.   */
  NULL,
  icuperiodcb,
  NULL,
  ICU_CHANNEL_2,
  0U,
  0xFFFFFFFFU
};

icucnt_t last_period;

static void icuperiodcb(ICUDriver *icup)
{

  last_period = icuGetPeriodX(icup);

  SENT_ISR_Handler(0, last_period);
}

void InitSent()
{
    icuStart(&ICUD4, &icucfg);
    icuStartCapture(&ICUD4);
    icuEnableNotifications(&ICUD4);
}

uint16_t SentGetPeriodValue(void)
{
    return (last_period >> 1);
}

static void SENT_ISR_Handler(uint8_t ch, uint16_t val)
{
        uint16_t val_res;
        uint8_t sentCrc;

#if 0
        if(val > val_prev[ch])
        {
                val_res = val - val_prev[ch];
        }
        else
        {
                val_res = SENT_PERIOD - val_prev[ch] + val;
        }

        val_prev[ch] = val;


        val_res = val_res >> 1;
#else
        val_res = val >> 1;
#endif

        if(val_res < 12)
        {
                val_res_err_cnt1++;

                val_res = 0;
        }
        else
        {
                val_res = val_res - 12;
        }

#if 0
        sentValArr[ch] = val_res;

        if(val_res > 50)
        {
                if(val_res > sentMaxSyncVal)
                {
                        if(val_max_cnt < 5)
                        {
                                val_max_cnt++;
                        }
                        else
                        {
                                sentMaxSyncVal = val_res;
                        }
                }

                if(val_res < sentMinSyncVal)
                {
                        if(val_min_cnt < 5)
                        {
                                val_min_cnt++;
                        }
                        else
                        {
                            sentMinSyncVal = val_res;
                        }
                }

                val_res_cnt++;

                if(val_res != 56) // 56
                {
                        val_res_err_cnt++;

                        err_per = ((float)val_res_err_cnt/(float)val_res_cnt)*100;
                }
        }
#endif
        switch(sentSMstate[ch])
        {
                case SM_SENT_INIT_STATE:
                    val_res_cnt++;

                    if(val_res == 44)
                    {// Пришел первый sync интервал - 56 тиков
                            sentSMstate[ch] = SM_SENT_STATUS_STATE;
                    }
                    break;

                case SM_SENT_SYNC_STATE:
                    val_res_cnt++;

                    if(val_res == 44)
                    {// Пришел sync интервал - 56 тиков
                            sentTempValArr[ch] = 0;

                            sentSMstate[ch] = SM_SENT_STATUS_STATE;
                    }
                    else
                    {
                            // Увеличиваем счетчик ошибок приема sync интервала
                            val_res_err_cnt++;

                            // Вычисляем процент ошибок
                            err_per = ((float)val_res_err_cnt/(float)val_res_cnt)*100;
                    }
                    break;

                case SM_SENT_STATUS_STATE:
                    // Пришел status интервал
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
                    // Проверяем номер пакета
                    if(sentRollCnt[ch] == (uint8_t)(sentRollCntPrev[ch] + 1))
                    {

                    }
                    else
                    {
                        sentRollErrCnt++;
                    }

                    sentRollCntPrev[ch] = sentRollCnt[ch];

                    // Проверяем crc пакета
                    sentCrc = ((uint8_t)(val_res));

                    if(sentCrc == sent_crc4(sentTempNibblArr[ch], 6))
                    {
                        sentValArr[ch] = sentTempValArr[ch];
                    }
                    else
                    {
                        sentCrcErrCnt++;
                    }

                    sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    break;
        }
}

#define CRCSeed 0x05

static const uint8_t SENT_CRC4_tbl[16] =
{
        0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5
};

uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata)
{
        uint8_t crc;
        uint16_t i;
        crc = 5; // Seed.
        for(i=0; i < ndata; i++)
        {
                crc = *pdata++ ^ SENT_CRC4_tbl[crc]; // Data[#1 to #6]
        }

        //crc = 0 ^ crc4sent_tbl[crc]; // Post-process.
        //return crc; // Return the CRC result.
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
