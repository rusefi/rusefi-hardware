

/*
todo all these arrays of [SENT_CHANNELS_NUM]
class sent_state {
    uint8_t currentCrc = 0;
};

sent_state states[SENT_CHANNELS_NUM];
*/

uint8_t sentCrc[SENT_CHANNELS_NUM] = {0};

// Value sensor arr
uint16_t sentValArr[SENT_CHANNELS_NUM] = {0};

//
uint16_t sentTempValArr[SENT_CHANNELS_NUM] = {0};

// Roll counter arr
uint8_t sentRollCnt[SENT_CHANNELS_NUM] = {0};
uint8_t sentRollCntPrev[SENT_CHANNELS_NUM] = {0};

uint16_t SENT_GetData(uint8_t ch)
{
        return sentValArr[ch];
}

uint32_t sentMinIntervalErr = 0;

uint32_t SENT_GetMinIntervalErrCnt(void)
{
        return sentMinIntervalErr;
}

static const uint8_t SENT_CRC4_tbl[16] =
{
        0, 13, 7, 10, 14, 3, 9, 4, 1, 12, 6, 11, 15, 2, 8, 5
};


#if SENT_DEV == SENT_SILABS_SENS
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
