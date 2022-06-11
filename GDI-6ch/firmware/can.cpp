#include "can.h"
#include "hal.h"

#include "fault.h"

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    0 // TODO: set bit timing! correctly!
};

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    while(1)
    {

        chThdSleepMilliseconds(100);
    }
}


static THD_WORKING_AREA(waCanRxThread, 256);
void CanRxThread(void*)
{
    while(1)
    {
        chThdSleepMilliseconds(100);
    }
}

void InitCan()
{
    canStart(&CAND1, &canConfig500);
    chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO, CanTxThread, nullptr);
    chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO - 4, CanRxThread, nullptr);
}

#define SWAP_UINT16(x) (((x) << 8) | ((x) >> 8))

