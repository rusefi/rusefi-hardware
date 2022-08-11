/*
 * sent.cpp
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#include "ch.h"
#include "hal.h"

#include "sent.h"

struct sent_channel {
    SM_SENT_enum state;
    uint8_t nibbles[SENT_MSG_PAYLOAD_SIZE];
    /* Sync interval is CPU clocks */
    uint32_t syncClocks;

#if SENT_ERR_PERCENT
    /* stats */
    uint32_t PulseCnt;
    uint32_t ShortIntervalErr;
    uint32_t LongIntervalErr;
    uint32_t SyncErr;
    uint32_t CrcErrCnt;
#endif // SENT_ERR_PERCENT
};

static struct sent_channel channels[SENT_CHANNELS_NUM];

/* Si7215 decoded data */
int32_t si7215_magnetic[SENT_CHANNELS_NUM];
int32_t si7215_counter[SENT_CHANNELS_NUM];

#if SENT_DEV == SENT_GM_ETB

uint16_t sentOpenThrottleVal = 0;
uint16_t sentClosedThrottleVal = 0;

uint16_t sentOpenTempVal = 0;
uint16_t sentClosedTempVal = 0;

uint8_t sentRawData = 1;

uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata);

#define SENT_TICK (5 * 72) // 5@72MHz us

int SENT_Decoder(struct sent_channel *ch, uint16_t val_res)
{
    int ret = 0;
    int interval;

    #if SENT_ERR_PERCENT
    ch->PulseCnt++;
    #endif

    interval = (val_res + SENT_TICK / 2) / SENT_TICK - SENT_OFFSET_INTERVAL;

    if (interval < 0) {
        ch->ShortIntervalErr++;
        ch->state = SM_SENT_INIT_STATE;
        return -1;
    }

    if ((interval > SENT_SYNC_INTERVAL) ||
        ((interval > 15) && (interval < SENT_SYNC_INTERVAL)))
    {
        ch->LongIntervalErr++;
        ch->state = SM_SENT_INIT_STATE;
        return -1;
    }

    switch(ch->state)
    {
        case SM_SENT_INIT_STATE:
            if (interval == SENT_SYNC_INTERVAL)
            {// sync interval - 56 ticks
                ch->state = SM_SENT_STATUS_STATE;
            }
            break;

        case SM_SENT_SYNC_STATE:
            if (interval == SENT_SYNC_INTERVAL)
            {// sync interval - 56 ticks
                ch->syncClocks = val_res;
                ch->state = SM_SENT_STATUS_STATE;
            }
            else
            {
                #if SENT_ERR_PERCENT
                //  Increment sync interval err count
                ch->SyncErr++;
                #endif
            }
            break;

        case SM_SENT_STATUS_STATE:
        case SM_SENT_SIG1_DATA1_STATE:
        case SM_SENT_SIG1_DATA2_STATE:
        case SM_SENT_SIG1_DATA3_STATE:
        case SM_SENT_SIG2_DATA1_STATE:
        case SM_SENT_SIG2_DATA2_STATE:
        case SM_SENT_SIG2_DATA3_STATE:
        case SM_SENT_CRC_STATE:
            if(interval <= SENT_MAX_INTERVAL)
            {
                ch->nibbles[ch->state - SM_SENT_STATUS_STATE] = interval;

                if (ch->state != SM_SENT_CRC_STATE)
                {
                    /* TODO: refactor */
                    ch->state = (SM_SENT_enum)((int)ch->state + 1);
                }
                else
                {
                    /* CRC check */
                    if(ch->nibbles[7] == sent_crc4(ch->nibbles, 7))
                    {
                        // Full packet has been received
                        ret = 1;
                    }
                    else
                    {
                        ch->CrcErrCnt++;
                        ret = -1;
                    }
                    ch->state = SM_SENT_SYNC_STATE;
                }
            }
            else
            {
                ch->LongIntervalErr++;

                ch->state = SM_SENT_INIT_STATE;
            }
            break;
    }

    return ret;
}

uint8_t sent_crc4(uint8_t* pdata, uint16_t ndata)
{
    size_t i;
    uint8_t crc = SENT_CRC_SEED; // initialize checksum with seed "0101"
    const uint8_t CrcLookup[16] = {0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5};

    for (i = 0; i < ndata; i++)
    {
        crc = crc ^ pdata[i];
        crc = CrcLookup[crc];
    }

    return crc;
}

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

/* Stat counters */
uint32_t SENT_GetShortIntervalErrCnt(void)
{
    return channels[0].ShortIntervalErr;
}

uint32_t SENT_GetLongIntervalErrCnt(void)
{
    return channels[0].LongIntervalErr;
}

uint32_t SENT_GetCrcErrCnt(void)
{
    return channels[0].CrcErrCnt;
}

uint32_t SENT_GetSyncErrCnt(void)
{
    return channels[0].SyncErr;
}

uint32_t SENT_GetSyncCnt(void)
{
    return channels[0].PulseCnt;
}

uint32_t SENT_GetTickTimeNs(void)
{
    return channels[0].syncClocks * 1000 / 72 / (SENT_OFFSET_INTERVAL + SENT_SYNC_INTERVAL);
}

/* Debug */
void SENT_GetRawNibbles(uint8_t* buf)
{
    for(uint8_t i = 0; i < SENT_MSG_PAYLOAD_SIZE; i++)
    {
        buf[i] = channels[0].nibbles[i];
    }
}

uint8_t SENT_GetThrottleValPrec(void)
{
    return (100 - ((sentOpenThrottleVal - SENT_THROTTLE_OPEN_VAL) * 100)/(SENT_THROTTLE_CLOSE_VAL - SENT_THROTTLE_OPEN_VAL));
}

#endif

uint32_t SENT_GetErrPercent(void)
{
    // cast float to int
    return 100 * channels[0].SyncErr / channels[0].PulseCnt;;
}

int32_t Si7215_GetMagneticField(uint32_t n)
{
    return si7215_magnetic[n];
}

int32_t Si7215_GetCounter(uint32_t n)
{
    return si7215_counter[n];
}

/* 4 per channel should be enougth */
#define SENT_MB_SIZE        (4 * SENT_CH_MAX)

static msg_t sent_mb_buffer[SENT_MB_SIZE];
static MAILBOX_DECL(sent_mb, sent_mb_buffer, SENT_MB_SIZE);

static THD_WORKING_AREA(waSentDecoderThread, 256);

void SENT_ISR_Handler(uint8_t ch, uint16_t val_res)
{
    /* encode to fin msg_t */
    msg_t msg = (ch << 16) | val_res;
    chMBPostI(&sent_mb, msg);
}

static void SentDecoderThread(void*)
{
    msg_t msg;
    while(true)
    {
        msg_t ret;

        ret = chMBFetchTimeout(&sent_mb, &msg, TIME_INFINITE);
        /* TODO: handle ret */
        if (ret == MSG_OK) {
            uint16_t tick = msg & 0xffff;
            uint8_t n = (msg >> 16) & 0xff;
            struct sent_channel *ch = &channels[n];

            if (SENT_Decoder(ch, tick) > 0) {
                /* decode Si7215 packet */
                if (((~ch->nibbles[1 + 5]) & 0x0f) == ch->nibbles[1 + 0]) {
                    si7215_magnetic[n] =
                        ((ch->nibbles[1 + 0] << 8) |
                         (ch->nibbles[1 + 1] << 4) |
                         (ch->nibbles[1 + 2] << 0)) - 2048;
                    si7215_counter[n] =
                        (ch->nibbles[1 + 3] << 4) |
                        (ch->nibbles[1 + 4] << 0);
                }
            }
        }
    }
}

void SentDecoder_Init(void)
{
    /* init interval mailbox */
    chMBObjectInit(&sent_mb, sent_mb_buffer, SENT_MB_SIZE);

    chThdCreateStatic(waSentDecoderThread, sizeof(waSentDecoderThread), NORMALPRIO, SentDecoderThread, nullptr);
}
