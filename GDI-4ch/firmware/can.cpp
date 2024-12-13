#include "can.h"
#include "hal.h"

#include <cstdint>
#include <cstring>

#include "fault.h"
#include "io_pins.h"
#include "persistence.h"
#include "can_common.h"
#include "pt2001impl.h"
#include "chprintf.h"

#define GDI4_CAN_SET_TAG 0x78
#include <rusefi/manifest.h>

// Decimal hex date presented as hex
static char VERSION[] = {compilationYear() / 100, compilationYear() % 100, compilationMonth(), compilationDay()};

extern GDIConfiguration configuration;
extern Pt2001 chip;
extern bool isOverallHappyStatus;

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
        CAN_BTR_SJW(0) | CAN_BTR_BRP(2)  | CAN_BTR_TS1(12) | CAN_BTR_TS2(1),
};

#define CAN_TX_TIMEOUT_100_MS TIME_MS2I(100)

int canWriteOk = 0;
int canWriteNotOk = 0;
//static char printBuffer[200];

static void countTxResult(msg_t msg) {
	if (msg == MSG_OK) {
		canWriteOk++;
	} else {
		canWriteNotOk++;
	}
}

void SendSomething() {
        CANTxFrame m_frame;

	    m_frame.IDE = CAN_IDE_EXT;
	    m_frame.SID = 0;
	    m_frame.EID = configuration.outputCanID;
	    m_frame.RTR = CAN_RTR_DATA;
	    m_frame.DLC = 8;
	    memset(m_frame.data8, 0, sizeof(m_frame.data8));

	    m_frame.data8[0] = configuration.inputCanID;
	    m_frame.data8[1] = configuration.updateCounter;
	    m_frame.data8[2] = isOverallHappyStatus;
	    m_frame.data8[6] = (int)chip.fault;
	    m_frame.data8[7] = GDI4_MAGIC;

    	msg_t msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, CAN_TX_TIMEOUT_100_MS);
    	countTxResult(msg);
}

static void sendOutConfiguration() {
        CANTxFrame m_frame;

	    m_frame.IDE = CAN_IDE_EXT;
	    m_frame.SID = 0;
	    m_frame.RTR = CAN_RTR_DATA;
	    memset(m_frame.data8, 0, sizeof(m_frame.data8));

        // CanConfiguration1
	    m_frame.DLC = 8;
	    m_frame.data16[0] =                configuration.BoostVoltage;
	    m_frame.data16[1] = float2short128(configuration.BoostCurrent);
	    m_frame.data16[2] =                configuration.TBoostMin;
	    m_frame.data16[3] =                configuration.TBoostMax;
	    m_frame.EID = configuration.outputCanID + 1;
    	msg_t msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, CAN_TX_TIMEOUT_100_MS);
    	countTxResult(msg);

        // CanConfiguration2
	    m_frame.DLC = 8;
	    m_frame.data16[0] = float2short128(configuration.PeakCurrent);
	    m_frame.data16[1] =                configuration.TpeakDuration;
	    m_frame.data16[2] =                configuration.TpeakOff;
	    m_frame.data16[3] =                configuration.Tbypass;
	    m_frame.EID = configuration.outputCanID + 2;
    	msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, CAN_TX_TIMEOUT_100_MS);
    	countTxResult(msg);

	    // CanConfiguration3
	    m_frame.DLC = 8;
	    m_frame.data16[0] = float2short128(configuration.HoldCurrent);
	    m_frame.data16[1] =                configuration.TholdOff;
	    m_frame.data16[2] =                configuration.THoldDuration;
	    m_frame.data16[3] = float2short128(configuration.PumpPeakCurrent);
	    m_frame.EID = configuration.outputCanID + 3;
    	msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, CAN_TX_TIMEOUT_100_MS);
    	countTxResult(msg);

	    // CanConfiguration4
	    m_frame.DLC = 2;
	    m_frame.data16[0] = float2short128(configuration.PumpHoldCurrent);
	    m_frame.EID = configuration.outputCanID + 4;
    	msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, CAN_TX_TIMEOUT_100_MS);
    	countTxResult(msg);
}

static void sendOutVersion() {
        CANTxFrame m_frame;

	    m_frame.IDE = CAN_IDE_EXT;
	    m_frame.SID = 0;
	    m_frame.EID = configuration.outputCanID + 5;
	    m_frame.RTR = CAN_RTR_DATA;
	    m_frame.DLC = sizeof(VERSION);
	    memcpy(m_frame.data8, VERSION, sizeof(VERSION));
    	msg_t msg = canTransmitTimeout(&CAND1, CAN_ANY_MAILBOX, &m_frame, CAN_TX_TIMEOUT_100_MS);
    	countTxResult(msg);
}

#define CAN_TX_PERIOD_MS 100

static int intTxCounter = 0;

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    while (1) {
        intTxCounter++;
        chThdSleepMilliseconds(1000 / CAN_TX_PERIOD_MS);

        if (configuration.outputCanID == 0) {
            continue; // we were told to be silent
        }

        if (intTxCounter % (1000 / CAN_TX_PERIOD_MS) == 0) {
            sendOutConfiguration();
        }
        if (intTxCounter % (1000 / CAN_TX_PERIOD_MS) == 0) {
            sendOutVersion();
        }

        SendSomething();
    }
}

static int getInt(CANRxFrame *frame, int offset) {
      return frame->data8[offset + 1] * 256 + frame->data8[offset];
}

static float getFloat(CANRxFrame *frame, int offset) {
      int value = getInt(frame, offset);
      return short2float128(value);
}

#define ASSIGN_IF_CHANGED(oldValue, newValue) \
                if ((oldValue) != (newValue)) { \
                    oldValue = (newValue); \
                    withNewValue = true; \
                }

static THD_WORKING_AREA(waCanRxThread, 256);
void CanRxThread(void*)
{
    while (1) {
            CANRxFrame frame;
            msg_t msg = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &frame, TIME_INFINITE);

            // Ignore non-ok results...
            if (msg != MSG_OK) {
                continue;
            }
            size_t writeCount = 0;

//                writeCount  = chsnprintf(printBuffer, sizeof(printBuffer), "eid=%d data[0]=%d dlc=%d\n\n\n\n\n\n\n",
//                frame.EID,
//                frame.data8[0],
//                frame.DLC);


            // Ignore std frames, only listen to ext
            if (frame.IDE != CAN_IDE_EXT) {
                continue;
            }

            // ignore packets not starting with magic byte or of unexpected length
            if (frame.data8[0] != GDI4_CAN_SET_TAG || frame.DLC != 7) {
                continue;
            }

            bool withNewValue = false;
            if (frame.EID == configuration.inputCanID) {
                ASSIGN_IF_CHANGED(configuration.BoostVoltage,  getInt(&frame,   1));
                ASSIGN_IF_CHANGED(configuration.BoostCurrent,  getFloat(&frame, 3));
                ASSIGN_IF_CHANGED(configuration.TBoostMin,     getInt(&frame,   5));
            } else if (frame.EID == configuration.inputCanID + 1) {
                ASSIGN_IF_CHANGED(configuration.TBoostMax,     getInt(&frame,   1));
                ASSIGN_IF_CHANGED(configuration.PeakCurrent,   getFloat(&frame, 3));
                ASSIGN_IF_CHANGED(configuration.TpeakDuration, getInt(&frame,   5));
            } else if (frame.EID == configuration.inputCanID + 2) {
                ASSIGN_IF_CHANGED(configuration.TpeakOff,      getInt(&frame,   1));
                ASSIGN_IF_CHANGED(configuration.Tbypass,       getInt(&frame,   3));
                ASSIGN_IF_CHANGED(configuration.HoldCurrent,   getFloat(&frame, 5));
            } else if (frame.EID == configuration.inputCanID + 3) {
                ASSIGN_IF_CHANGED(configuration.TholdOff,      getInt(&frame,   1));
                ASSIGN_IF_CHANGED(configuration.THoldDuration, getInt(&frame,   3));
                ASSIGN_IF_CHANGED(configuration.PumpPeakCurrent,   getFloat(&frame, 5));
            } else if (frame.EID == configuration.inputCanID + 4) {
                ASSIGN_IF_CHANGED(configuration.PumpHoldCurrent,   getFloat(&frame, 1));
                ASSIGN_IF_CHANGED(configuration.outputCanID, getInt(&frame,   3));
            }
            if (withNewValue) {
                saveConfiguration();
                chip.restart();
            }
//            if (writeCount > 0)
//                uartStartSend(&UARTD1, writeCount, printBuffer);

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

