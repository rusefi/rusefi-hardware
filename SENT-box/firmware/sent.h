/*
 * sent.h
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#ifndef SENT_H_
#define SENT_H_

typedef enum
{
        SM_SENT_INIT_STATE = 0,
        SM_SENT_SYNC_STATE,
        SM_SENT_STATUS_STATE,
        SM_SENT_SIG1_DATA1_STATE,
        SM_SENT_SIG1_DATA2_STATE,
        SM_SENT_SIG1_DATA3_STATE,
        SM_SENT_SIG2_DATA1_STATE,
        SM_SENT_SIG2_DATA2_STATE,
        SM_SENT_SIG2_DATA3_STATE,
        SM_SENT_CRC_STATE,
}SM_SENT_enum;


void InitSent();
uint16_t SentGetPeriodValue(void);

uint16_t SENT_GetData(uint8_t ch);
uint16_t SENT_GetRollErrCnt(void);
uint16_t SENT_GetCrcErrCnt(void);
uint16_t SENT_GetIntervalErrCnt(void);
uint16_t SENT_GetSyncErrCnt(void);

#endif /* SENT_H_ */
