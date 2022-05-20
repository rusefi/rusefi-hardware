/*
 * sent.h
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#ifndef SENT_H_
#define SENT_H_

#define SENT_CHANNELS_NUM 4 // Number of sent channels

#define SENT_SYNC_INTERVAL  44 // 56 ticks - 12
#define SENT_OFFSET_INTERVAL 12

#define SENT_MIN_INTERVAL 12
#define SENT_MAX_INTERVAL 15

#define SENT_CRC_SEED 0x05

#define SENT_MSG_PAYLOAD_SIZE 6  // Size of payload

#define SENT_ERR_PERCENT 0

enum
{
    SENT_CH1 = 0,
    SENT_CH2,
    SENT_CH3,
    SENT_CH4,
};

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

void SENT_ISR_Handler(uint8_t ch, uint16_t val_res);

uint16_t SENT_GetData(uint8_t ch);
uint16_t SENT_GetRollErrCnt(void);
uint16_t SENT_GetCrcErrCnt(void);
uint16_t SENT_GetIntervalErrCnt(void);
uint16_t SENT_GetSyncErrCnt(void);

#endif /* SENT_H_ */
