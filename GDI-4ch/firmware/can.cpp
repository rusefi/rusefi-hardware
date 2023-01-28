#include "can.h"
#include "hal.h"

#include <cstdint>
#include <cstring>

#include "fault.h"
#include "io_pins.h"
#include "persistence.h"
#include "can_common.h"

// Decimal hex date presented as hex
static char VERSION[] = {0x20, 0x22, 0x12, 0x20};

extern GDIConfiguration configuration;

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
        CAN_BTR_SJW(0) | CAN_BTR_BRP(2)  | CAN_BTR_TS1(12) | CAN_BTR_TS2(1),
};

void SendSomething()
{
        CANTxFrame m_frame;

	    m_frame.IDE = CAN_IDE_STD;
	    m_frame.EID = 0;
	    m_frame.SID = GDI4_BASE_ADDRESS;
	    m_frame.RTR = CAN_RTR_DATA;
	    m_frame.DLC = 8;
	    memset(m_frame.data8, 0, sizeof(m_frame.data8));

	    m_frame.data8[0] = configuration.inputCanID;
	    m_frame.data8[1] = configuration.updateCounter;
	    m_frame.data8[6] = 0x33;
	    m_frame.data8[7] = 0x66;

    	canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, TIME_IMMEDIATE);
}

static void sendOutConfiguration() {
        CANTxFrame m_frame;

	    m_frame.IDE = CAN_IDE_STD;
	    m_frame.EID = 0;
	    m_frame.SID = GDI4_BASE_ADDRESS + 1;
	    m_frame.RTR = CAN_RTR_DATA;
	    m_frame.DLC = 8;
	    memset(m_frame.data8, 0, sizeof(m_frame.data8));
	    m_frame.data16[0] = float2short100(configuration.BoostVoltage);
	    m_frame.data16[1] = float2short100(configuration.BoostCurrent);
	    m_frame.data16[2] = float2short100(configuration.PeakCurrent);
	    m_frame.data16[3] = float2short100(configuration.HoldCurrent);

    	canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, TIME_IMMEDIATE);
}

static void sendOutVersion() {
        CANTxFrame m_frame;

	    m_frame.IDE = CAN_IDE_STD;
	    m_frame.EID = 0;
	    m_frame.SID = GDI4_BASE_ADDRESS + 2;
	    m_frame.RTR = CAN_RTR_DATA;
	    m_frame.DLC = sizeof(VERSION);
	    memcpy(m_frame.data8, VERSION, sizeof(VERSION));
}

#define CAN_TX_PERIOD_MS 100

static int intTxCounter = 0;

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    while (1) {
        intTxCounter++;
        if (intTxCounter % (1000 / CAN_TX_PERIOD_MS) == 0) {
            sendOutConfiguration();
        }
        if (intTxCounter % (1000 / CAN_TX_PERIOD_MS) == 0) {
            sendOutVersion();
        }

        SendSomething();

        chThdSleepMilliseconds(1000 / CAN_TX_PERIOD_MS);
    }
}


static float getFloat(CANRxFrame *frame, int offset) {
      int value = frame->data8[offset + 1] * 256 + frame->data8[offset];
       return short2float100(value);
}

static THD_WORKING_AREA(waCanRxThread, 256);
void CanRxThread(void*)
{
    while (1) {
            CANRxFrame frame;
            msg_t msg = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);

            // Ignore non-ok results...
            if (msg != MSG_OK)
            {
                continue;
            }

            // Ignore std frames, only listen to ext
            if (frame.IDE != CAN_IDE_EXT)
            {
                continue;
            }

            if (frame.EID == configuration.inputCanID && frame.DLC == 7 && frame.data8[0] == 0x88) {
                configuration.BoostVoltage = getFloat(&frame, 1);
                configuration.BoostCurrent = getFloat(&frame, 3);
                configuration.PeakCurrent = getFloat(&frame, 3);
                saveConfiguration();
            } else if (frame.EID == configuration.inputCanID + 1 && frame.DLC == 7 && frame.data8[0] == 0x88) {
                configuration.HoldCurrent = getFloat(&frame, 1);
                configuration.TpeakDuration = getFloat(&frame, 3);
                configuration.THoldDuration = getFloat(&frame, 3);
                saveConfiguration();
            }


        chThdSleepMilliseconds(100);
    }
}

void InitCan()
{
    canStart(&CAND1, &canConfig500);

    // CAN TX
    palSetPadMode(CAN_GPIO_PORT,CAN_TX_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    // CAN RX
    palSetPadMode(CAN_GPIO_PORT,CAN_RX_PIN, PAL_MODE_INPUT_PULLUP );

    chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO, CanTxThread, nullptr);
    chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO - 4, CanRxThread, nullptr);
}

//#define SWAP_UINT16(x) (((x) << 8) | ((x) >> 8))

