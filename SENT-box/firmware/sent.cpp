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

// Sensor status arr
uint8_t sentStat[SENT_CHANNELS_NUM] = {0};

// Error counters
uint32_t sentMaxIntervalErr = 0;
uint32_t sentSyncErr = 0;
uint32_t sentStatusErr = 0;
uint32_t sentRollErrCnt = 0;
uint32_t sentCrcErrCnt = 0;

#if SENT_ERR_PERCENT
// Received msg counter
uint32_t sentPulseCnt = 0;

float err_per = 0;
#endif // SENT_ERR_PERCENT

#if SENT_DEV == SENT_GM_ETB
// Received nibbles arr
uint8_t sentTempNibblArr[SENT_CHANNELS_NUM][SENT_MSG_PAYLOAD_SIZE] = {0};

uint16_t sentOpenThrottleVal = 0;
uint16_t sentClosedThrottleVal = 0;

uint16_t sentOpenTempVal = 0;
uint16_t sentClosedTempVal = 0;

uint8_t sentRawData = 0;

uint32_t sentIntervalErr = 0;

#define UNEXPECTED_VALUE 0xFF

const uint8_t sentLookupTable[] =
{
        UNEXPECTED_VALUE,   // 0
        UNEXPECTED_VALUE,   // 1
        UNEXPECTED_VALUE,   // 2
        UNEXPECTED_VALUE,   // 3
        UNEXPECTED_VALUE,   // 4
        UNEXPECTED_VALUE,   // 5
        UNEXPECTED_VALUE,   // 6
        UNEXPECTED_VALUE,   // 7
        UNEXPECTED_VALUE,   // 8
        UNEXPECTED_VALUE,   // 9
        UNEXPECTED_VALUE,   // 10
        UNEXPECTED_VALUE,   // 11
        UNEXPECTED_VALUE,   // 12
        UNEXPECTED_VALUE,   // 13
        UNEXPECTED_VALUE,   // 14
        UNEXPECTED_VALUE,   // 15
        UNEXPECTED_VALUE,   // 16
        UNEXPECTED_VALUE,   // 17
        UNEXPECTED_VALUE,   // 18
        UNEXPECTED_VALUE,   // 19
        UNEXPECTED_VALUE,   // 20
        UNEXPECTED_VALUE,   // 21
        UNEXPECTED_VALUE,   // 22
        UNEXPECTED_VALUE,   // 23
        UNEXPECTED_VALUE,   // 24
        UNEXPECTED_VALUE,   // 25
        UNEXPECTED_VALUE,   // 26
        UNEXPECTED_VALUE,   // 27
        UNEXPECTED_VALUE,   // 28
        UNEXPECTED_VALUE,   // 29
        UNEXPECTED_VALUE,   // 30
        UNEXPECTED_VALUE,   // 31
        UNEXPECTED_VALUE,   // 32
        UNEXPECTED_VALUE,   // 33
        UNEXPECTED_VALUE,   // 34
        UNEXPECTED_VALUE,   // 35
        UNEXPECTED_VALUE,   // 36
        UNEXPECTED_VALUE,   // 37
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
        UNEXPECTED_VALUE,   // 53
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
        UNEXPECTED_VALUE,   // 66
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
        UNEXPECTED_VALUE,   // 79
        13,     // 80
        13,     // 81
        13,     // 82
        14,     // 83
        14,     // 84
        14,     // 85
        15,     // 86
        15,     // 87
        15,     // 88
        UNEXPECTED_VALUE,   // 89
        UNEXPECTED_VALUE,   // 90
        UNEXPECTED_VALUE,   // 91
        UNEXPECTED_VALUE,   // 92
        UNEXPECTED_VALUE,   // 93
        UNEXPECTED_VALUE,   // 94
        UNEXPECTED_VALUE,   // 95
        UNEXPECTED_VALUE,   // 96
        UNEXPECTED_VALUE,   // 97
        UNEXPECTED_VALUE,   // 98
        UNEXPECTED_VALUE,   // 99
        UNEXPECTED_VALUE,   // 100
        UNEXPECTED_VALUE,   // 101
        UNEXPECTED_VALUE,   // 102
        UNEXPECTED_VALUE,   // 103
        UNEXPECTED_VALUE,   // 104
        UNEXPECTED_VALUE,   // 105
        UNEXPECTED_VALUE,   // 106
        UNEXPECTED_VALUE,   // 107
        UNEXPECTED_VALUE,   // 108
        UNEXPECTED_VALUE,   // 109
        UNEXPECTED_VALUE,   // 110
        UNEXPECTED_VALUE,   // 111
        UNEXPECTED_VALUE,   // 112
        UNEXPECTED_VALUE,   // 113
        UNEXPECTED_VALUE,   // 114
        UNEXPECTED_VALUE,   // 115
        UNEXPECTED_VALUE,   // 116
        UNEXPECTED_VALUE,   // 117
        UNEXPECTED_VALUE,   // 118
        UNEXPECTED_VALUE,   // 119
        UNEXPECTED_VALUE,   // 120
        UNEXPECTED_VALUE,   // 121
        UNEXPECTED_VALUE,   // 122
        UNEXPECTED_VALUE,   // 123
        UNEXPECTED_VALUE,   // 124
        UNEXPECTED_VALUE,   // 125
        UNEXPECTED_VALUE,   // 126
        UNEXPECTED_VALUE,   // 127
        UNEXPECTED_VALUE,   // 128
        UNEXPECTED_VALUE,   // 129
        UNEXPECTED_VALUE,   // 130
        UNEXPECTED_VALUE,   // 131
        UNEXPECTED_VALUE,   // 132
        UNEXPECTED_VALUE,   // 133
        UNEXPECTED_VALUE,   // 134
        UNEXPECTED_VALUE,   // 135
        UNEXPECTED_VALUE,   // 136
        UNEXPECTED_VALUE,   // 137
        UNEXPECTED_VALUE,   // 138
        UNEXPECTED_VALUE,   // 139
        UNEXPECTED_VALUE,   // 140
        UNEXPECTED_VALUE,   // 141
        UNEXPECTED_VALUE,   // 142
        UNEXPECTED_VALUE,   // 143
        UNEXPECTED_VALUE,   // 144
        UNEXPECTED_VALUE,   // 145
        UNEXPECTED_VALUE,   // 146
        UNEXPECTED_VALUE,   // 147
        UNEXPECTED_VALUE,   // 148
        UNEXPECTED_VALUE,   // 149
        UNEXPECTED_VALUE,   // 150
        UNEXPECTED_VALUE,   // 151
        UNEXPECTED_VALUE,   // 152
        UNEXPECTED_VALUE,   // 153
        UNEXPECTED_VALUE,   // 154
        UNEXPECTED_VALUE,   // 155
        UNEXPECTED_VALUE,   // 156
        UNEXPECTED_VALUE,   // 157
        UNEXPECTED_VALUE,   // 158
        UNEXPECTED_VALUE,   // 159
        UNEXPECTED_VALUE,   // 160
        UNEXPECTED_VALUE,   // 161
        UNEXPECTED_VALUE,   // 162
        UNEXPECTED_VALUE,   // 163
        UNEXPECTED_VALUE,   // 164
        UNEXPECTED_VALUE,   // 165
        UNEXPECTED_VALUE,   // 166
        UNEXPECTED_VALUE,   // 167
        UNEXPECTED_VALUE,   // 168
        UNEXPECTED_VALUE,   // 169
        UNEXPECTED_VALUE,   // 170
        UNEXPECTED_VALUE,   // 171
        UNEXPECTED_VALUE,   // 172
        UNEXPECTED_VALUE,   // 173
        UNEXPECTED_VALUE,   // 174
        UNEXPECTED_VALUE,   // 175
        UNEXPECTED_VALUE,   // 176
        UNEXPECTED_VALUE,   // 177
        UNEXPECTED_VALUE,   // 178
        UNEXPECTED_VALUE,   // 179
        SENT_SYNC_INTERVAL, // 180
        SENT_SYNC_INTERVAL, // 181
        SENT_SYNC_INTERVAL, // 182
};

uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata);

#pragma GCC push_options
#pragma GCC optimize ("O2")

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
                            // if CRC is good commit packet value as official result
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

uint32_t SENT_GetRollErrCnt(void)
{
        return sentRollErrCnt;
}

uint32_t SENT_GetCrcErrCnt(void)
{
        return sentCrcErrCnt;
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
