/*
 * sent.cpp
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#include "ch.h"
#include "hal.h"

#include "sent.h"

#if SENT_MODE == SENT_MODE_ICU
#include "sent_hw_icu.h"
#endif

#if SENT_MODE == SENT_MODE_PAL
#include "sent_hw_pal.h"
#endif

// Sent SM status arr
SM_SENT_enum sentSMstate[SENT_CHANNELS_NUM] = {SM_SENT_INIT_STATE};

// Value sensor arr
uint16_t sentValArr[SENT_CHANNELS_NUM] = {0};

//
uint16_t sentTempValArr[SENT_CHANNELS_NUM] = {0};

// Sensor status arr
uint8_t sentStat[SENT_CHANNELS_NUM] = {0};

// Roll counter arr
uint8_t sentRollCnt[SENT_CHANNELS_NUM] = {0};
uint8_t sentRollCntPrev[SENT_CHANNELS_NUM] = {0};

uint8_t sentCrc[SENT_CHANNELS_NUM] = {0};

// Error counters
uint32_t sentMinIntervalErr = 0;
uint32_t sentMaxIntervalErr = 0;
uint32_t sentSyncErr = 0;
uint32_t sentStatusErr = 0;
uint32_t sentRollErrCnt = 0;
uint32_t sentCrcErrCnt = 0;

uint8_t sentCh1DataReady = 0;
uint8_t sentCh2DataReady = 0;

#if SENT_DEV == SENT_GM_ETB
// Received nibbles arr
uint8_t sentTempNibblArr[SENT_CHANNELS_NUM][SENT_MSG_PAYLOAD_SIZE] = {0};

uint16_t sentOpenThrottleVal = 0;
uint16_t sentClosedThrottleVal = 0;

uint16_t sentOpenTempVal = 0;
uint16_t sentClosedTempVal = 0;

uint8_t sentRawData = 0;

uint32_t sentIntervalErr = 0;

const uint8_t sentLookupTable[] =
{
        0xFF,   // 0
        0xFF,   // 1
        0xFF,   // 2
        0xFF,   // 3
        0xFF,   // 4
        0xFF,   // 5
        0xFF,   // 6
        0xFF,   // 7
        0xFF,   // 8
        0xFF,   // 9
        0xFF,   // 10
        0xFF,   // 11
        0xFF,   // 12
        0xFF,   // 13
        0xFF,   // 14
        0xFF,   // 15
        0xFF,   // 16
        0xFF,   // 17
        0xFF,   // 18
        0xFF,   // 19
        0xFF,   // 20
        0xFF,   // 21
        0xFF,   // 22
        0xFF,   // 23
        0xFF,   // 24
        0xFF,   // 25
        0xFF,   // 26
        0xFF,   // 27
        0xFF,   // 28
        0xFF,   // 29
        0xFF,   // 30
        0xFF,   // 31
        0xFF,   // 32
        0xFF,   // 33
        0xFF,   // 34
        0xFF,   // 35
        0xFF,   // 36
        0xFF,   // 37
        0,  // 38
        0,  // 39
        0,  // 40
        1,  // 41
        1,  // 42
        1,  // 43
        2,  // 44
        2,  // 45
        2,  // 46
        3,  // 47
        3,  // 48
        3,  // 49
        4,  // 50
        4,  // 51
        4,  // 52
        0xFF,   // 53
        5,  // 54
        5,  // 55
        5,  // 56
        6,  // 57
        6,  // 58
        6,  // 59
        7,  // 60
        7,  // 61
        7,  // 62
        8,  // 63
        8,  // 64
        8,  // 65
        0xFF,   // 66
        9,  // 67
        9,  // 68
        9,  // 69
        10,     // 70
        10,     // 71
        10,     // 72
        11,     // 73
        11,     // 74
        11,     // 75
        12,     // 76
        12,     // 77
        12,     // 78
        0xFF,   // 79
        13,     // 80
        13,     // 81
        13,     // 82
        14,     // 83
        14,     // 84
        14,     // 85
        15,     // 86
        15,     // 87
        15,     // 88
        0xFF,   // 89
        0xFF,   // 90
        0xFF,   // 91
        0xFF,   // 92
        0xFF,   // 93
        0xFF,   // 94
        0xFF,   // 95
        0xFF,   // 96
        0xFF,   // 97
        0xFF,   // 98
        0xFF,   // 99
        0xFF,   // 100
        0xFF,   // 101
        0xFF,   // 102
        0xFF,   // 103
        0xFF,   // 104
        0xFF,   // 105
        0xFF,   // 106
        0xFF,   // 107
        0xFF,   // 108
        0xFF,   // 109
        0xFF,   // 110
        0xFF,   // 111
        0xFF,   // 112
        0xFF,   // 113
        0xFF,   // 114
        0xFF,   // 115
        0xFF,   // 116
        0xFF,   // 117
        0xFF,   // 118
        0xFF,   // 119
        0xFF,   // 120
        0xFF,   // 121
        0xFF,   // 122
        0xFF,   // 123
        0xFF,   // 124
        0xFF,   // 125
        0xFF,   // 126
        0xFF,   // 127
        0xFF,   // 128
        0xFF,   // 129
        0xFF,   // 130
        0xFF,   // 131
        0xFF,   // 132
        0xFF,   // 133
        0xFF,   // 134
        0xFF,   // 135
        0xFF,   // 136
        0xFF,   // 137
        0xFF,   // 138
        0xFF,   // 139
        0xFF,   // 140
        0xFF,   // 141
        0xFF,   // 142
        0xFF,   // 143
        0xFF,   // 144
        0xFF,   // 145
        0xFF,   // 146
        0xFF,   // 147
        0xFF,   // 148
        0xFF,   // 149
        0xFF,   // 150
        0xFF,   // 151
        0xFF,   // 152
        0xFF,   // 153
        0xFF,   // 154
        0xFF,   // 155
        0xFF,   // 156
        0xFF,   // 157
        0xFF,   // 158
        0xFF,   // 159
        0xFF,   // 160
        0xFF,   // 161
        0xFF,   // 162
        0xFF,   // 163
        0xFF,   // 164
        0xFF,   // 165
        0xFF,   // 166
        0xFF,   // 167
        0xFF,   // 168
        0xFF,   // 169
        0xFF,   // 170
        0xFF,   // 171
        0xFF,   // 172
        0xFF,   // 173
        0xFF,   // 174
        0xFF,   // 175
        0xFF,   // 176
        0xFF,   // 177
        0xFF,   // 178
        0xFF,   // 179
        44, // 180
        44, // 181
        44, // 182
};

uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata);
#endif

extern uint16_t cyccnt_ch1_period;
extern uint16_t cyccnt_ch2_period;

static const uint8_t SENT_CRC4_tbl[16] =
{
        0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5
};

#if SENT_ERR_PERCENT
// Received msg counter
uint32_t sentPulseCnt = 0;

float err_per = 0;
#endif

void InitSent()
{
#if SENT_MODE == SENT_MODE_ICU
    InitSentHwIcu();
#endif

#if SENT_MODE == SENT_MODE_PAL
    InitSentHwPal();
#endif
}

#pragma GCC push_options
#pragma GCC optimize ("O2")
#if SENT_DEV == SENT_GM_ETB
void SENT_ISR_Handler(uint8_t ch, uint16_t val_res)
{
        val_res = sentLookupTable[val_res];

        if(val_res == 0xFF)
        {
            sentIntervalErr++;
        }
        else
        {
            switch(sentSMstate[ch])
            {
                case SM_SENT_INIT_STATE:
#if SENT_ERR_PERCENT
                    sentPulseCnt++;
#endif

                    if(val_res == SENT_SYNC_INTERVAL)
                    {// sync interval - 56 ticks
                        sentSMstate[ch] = SM_SENT_STATUS_STATE;
                    }
                    break;

                case SM_SENT_SYNC_STATE:
#if SENT_ERR_PERCENT
                    sentPulseCnt++;
#endif

                    if(val_res == SENT_SYNC_INTERVAL)
                    {// sync interval - 56 ticks
                        sentSMstate[ch] = SM_SENT_STATUS_STATE;
                    }
                    else
                    {
                        //  Increment sync interval err count
                        sentSyncErr++;

#if SENT_ERR_PERCENT
                        // Calc err percentage
                        err_per = ((float)sentSyncErr/(float)sentPulseCnt)*100;
#endif
                    }
                    break;

                case SM_SENT_STATUS_STATE:
                    // status interval
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    }
                    else
                    {
                        sentStat[ch] = val_res;

                        if(sentStat[ch])
                        {
                            sentStatusErr++;
                        }

                        sentTempNibblArr[ch][0] = val_res;

                        sentSMstate[ch] = SM_SENT_SIG1_DATA1_STATE;
                    }
                    break;

                case SM_SENT_SIG1_DATA1_STATE:
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    }
                    else
                    {
                        sentTempNibblArr[ch][1] = val_res;

                        sentOpenTempVal = ((uint16_t)(val_res) << 8);

                        sentSMstate[ch] = SM_SENT_SIG1_DATA2_STATE;
                    }
                    break;

                case SM_SENT_SIG1_DATA2_STATE:
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    }
                    else
                    {
                        sentTempNibblArr[ch][2] = val_res;

                        sentOpenTempVal |= ((uint16_t)(val_res) << 4);

                        sentSMstate[ch] = SM_SENT_SIG1_DATA3_STATE;
                    }
                    break;

                case SM_SENT_SIG1_DATA3_STATE:
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    }
                    else
                    {
                        sentTempNibblArr[ch][3] = val_res;

                        sentOpenTempVal |= (val_res);

                        sentSMstate[ch] = SM_SENT_SIG2_DATA1_STATE;
                    }
                    break;

                case SM_SENT_SIG2_DATA1_STATE:
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    }
                    else
                    {
                        sentTempNibblArr[ch][4] = val_res;

                        sentClosedTempVal = (val_res);

                        sentSMstate[ch] = SM_SENT_SIG2_DATA2_STATE;
                    }
                    break;

                case SM_SENT_SIG2_DATA2_STATE:
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    }
                    else
                    {
                        sentTempNibblArr[ch][5] = val_res;

                        sentClosedTempVal |= ((uint16_t)(val_res) << 4);

                        sentSMstate[ch] = SM_SENT_SIG2_DATA3_STATE;
                    }
                    break;

                case SM_SENT_SIG2_DATA3_STATE:
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;

                        sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    }
                    else
                    {
                        sentTempNibblArr[ch][6] = val_res;

                        sentClosedTempVal |= ((uint16_t)(val_res) << 8);

                        sentSMstate[ch] = SM_SENT_CRC_STATE;
                    }
                    break;

                case SM_SENT_CRC_STATE:
                    if(val_res > SENT_MAX_INTERVAL)
                    {
                        sentMaxIntervalErr++;
                    }
                    else
                    {
                        sentTempNibblArr[ch][7] = val_res;

                        // Check crc
                        if((uint8_t)(val_res) == sent_crc4((uint8_t*)&sentTempNibblArr[ch][1], 6))
                        {
                            sentOpenThrottleVal = sentOpenTempVal;
                            sentClosedThrottleVal = sentClosedTempVal;
                        }
                        else
                        {
                          //  Increment crc err count
                            sentCrcErrCnt++;
                        }
                    }

                    sentSMstate[ch] = SM_SENT_SYNC_STATE;
                    break;
              }
        }
}
#pragma GCC pop_options
#elif SENT_DEV == SENT_SILABS_SENS
void SENT_ISR_Handler(uint8_t ch, uint16_t val_res)
{
        {
            if(val_res < SENT_MIN_INTERVAL)
            {// sent interval less than min interval
                sentMinIntervalErr++;

                sentSMstate[ch] = SM_SENT_SYNC_STATE;
            }
            else
            {
                val_res -= SENT_OFFSET_INTERVAL;

                {
                    switch(sentSMstate[ch])
                    {
                        case SM_SENT_INIT_STATE:
#if SENT_ERR_PERCENT
                            sentPulseCnt++;
#endif

                            if(val_res == SENT_SYNC_INTERVAL)
                            {// sync interval - 56 ticks
                                sentSMstate[ch] = SM_SENT_STATUS_STATE;
                            }
                            break;

                        case SM_SENT_SYNC_STATE:
#if SENT_ERR_PERCENT
                            sentPulseCnt++;
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
                                err_per = ((float)sentSyncErr/(float)sentPulseCnt)*100;
#endif
                            }
                            break;

                        case SM_SENT_STATUS_STATE:
                            // status interval
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;

                                sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            }
                            else
                            {
                                sentStat[ch] = val_res;

                                if(sentStat[ch])
                                {
                                    sentStatusErr++;
                                }

                                sentSMstate[ch] = SM_SENT_SIG1_DATA1_STATE;
                            }
                            break;

                        case SM_SENT_SIG1_DATA1_STATE:
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;

                                sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            }
                            else
                            {
                                sentCrc[ch] = val_res ^ SENT_CRC4_tbl[SENT_CRC_SEED];

                                sentTempValArr[ch] = ((uint16_t)(val_res) << 8);

                                sentSMstate[ch] = SM_SENT_SIG1_DATA2_STATE;
                            }
                            break;

                        case SM_SENT_SIG1_DATA2_STATE:
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;

                                sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            }
                            else
                            {
                                sentCrc[ch] = val_res ^ SENT_CRC4_tbl[sentCrc[ch]];

                                sentTempValArr[ch] |= ((uint16_t)(val_res) << 4);

                                sentSMstate[ch] = SM_SENT_SIG1_DATA3_STATE;
                            }
                            break;

                        case SM_SENT_SIG1_DATA3_STATE:
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;

                                sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            }
                            else
                            {
                                sentCrc[ch] = val_res ^ SENT_CRC4_tbl[sentCrc[ch]];

                                sentTempValArr[ch] |= (val_res);

                                sentSMstate[ch] = SM_SENT_SIG2_DATA1_STATE;
                            }
                            break;

                        case SM_SENT_SIG2_DATA1_STATE:
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;

                                sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            }
                            else
                            {
                                sentCrc[ch] = val_res ^ SENT_CRC4_tbl[sentCrc[ch]];

                                sentRollCnt[ch] = ((uint8_t)(val_res) << 4);

                                sentSMstate[ch] = SM_SENT_SIG2_DATA2_STATE;
                            }
                            break;

                        case SM_SENT_SIG2_DATA2_STATE:
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;

                                sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            }
                            else
                            {
                                sentCrc[ch] = val_res ^ SENT_CRC4_tbl[sentCrc[ch]];

                                sentRollCnt[ch] |= ((uint8_t)(val_res));

                                sentSMstate[ch] = SM_SENT_SIG2_DATA3_STATE;
                            }
                            break;

                        case SM_SENT_SIG2_DATA3_STATE:
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;

                                sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            }
                            else
                            {
                                sentCrc[ch] = val_res ^ SENT_CRC4_tbl[sentCrc[ch]];

                                sentSMstate[ch] = SM_SENT_CRC_STATE;
                            }
                            break;

                        case SM_SENT_CRC_STATE:
                            if(val_res > SENT_MAX_INTERVAL)
                            {
                                sentMaxIntervalErr++;
                            }
                            else
                            {
                                // Check msg number
                                if(sentRollCnt[ch] != (uint8_t)(sentRollCntPrev[ch] + 1))
                                {// Msg lost
                                    //  Increment msg number err count
                                    sentRollErrCnt++;
                                }

                                sentRollCntPrev[ch] = sentRollCnt[ch];

                                // Check crc
                                if((uint8_t)(val_res) == SENT_CRC4_tbl[sentCrc[ch]])
                                {
                                    sentValArr[ch] = sentTempValArr[ch];
                                }
                                else
                                {
                                  //  Increment crc err count
                                    sentCrcErrCnt++;
                                }
                            }

                            sentSMstate[ch] = SM_SENT_SYNC_STATE;
                            break;
                      }
                  }
            }
        }
}
#endif

uint16_t SENT_GetData(uint8_t ch)
{
        return sentValArr[ch];
}

uint32_t SENT_GetRollErrCnt(void)
{
        return sentRollErrCnt;
}

uint32_t SENT_GetCrcErrCnt(void)
{
        return sentCrcErrCnt;
}

uint32_t SENT_GetMinIntervalErrCnt(void)
{
        return sentMinIntervalErr;
}

uint32_t SENT_GetMaxIntervalErrCnt(void)
{
        return sentMaxIntervalErr;
}

uint32_t SENT_GetSyncErrCnt(void)
{
        return sentSyncErr;
}

uint32_t SENT_GetSyncCnt(void)
{
        return sentPulseCnt;
}

#if SENT_DEV == SENT_GM_ETB
#pragma GCC push_options
#pragma GCC optimize ("O2")
const uint8_t CrcLookup[16] = {0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5};

uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata)
{
        uint8_t calculatedCRC, i;

        calculatedCRC = SENT_CRC_SEED; // initialize checksum with seed "0101"

        for (i = 0; i < ndata; i++)
        {
                calculatedCRC = CrcLookup[calculatedCRC];
                calculatedCRC = (calculatedCRC ^ pdata[i]) & 0x0F;
        }
        // One more round with 0 as input
        //calculatedCRC = CrcLookup[calculatedCRC];

        return calculatedCRC;
}
#pragma GCC pop_options

uint8_t SENT_IsRawData(void)
{
    return sentRawData;
}

void SENT_SetRawDataProp(void)
{
  sentRawData = 1;
}

void SENT_ResetRawDataProp(void)
{
  sentRawData = 0;
}

uint16_t SENT_GetOpenThrottleVal(void)
{
    return sentOpenThrottleVal;
}

uint16_t SENT_GetClosedThrottleVal(void)
{
    return sentClosedThrottleVal;
}

uint32_t SENT_GetIntervalErr(void)
{
    return sentIntervalErr;
}

void SENT_GetRawData(uint8_t* buf)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        buf[i] = sentTempNibblArr[1][i];
    }
}

uint8_t SENT_GetThrottleValPrec(void)
{
        return (100 - ((sentOpenThrottleVal - SENT_THROTTLE_OPEN_VAL) * 100)/(SENT_THROTTLE_CLOSE_VAL - SENT_THROTTLE_OPEN_VAL));
}

#endif
