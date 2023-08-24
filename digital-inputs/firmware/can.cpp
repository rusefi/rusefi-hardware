#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "io_pins.h"
#include "can.h"
#include "test_logic.h"
#include "can/can_common.h"
#include "global.h"

extern BaseSequentialStream *chp;

static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

static bool isGoodCanPackets = true;
static bool hasReceivedAnalog = false;

static void canPacketError(const char *msg, ...) {
	va_list vl;
	va_start(vl, msg);
	chvprintf(chp, msg, vl);
	va_end(vl);

	isGoodCanPackets = false;
}

void startNewCanTest() {
    isGoodCanPackets = true;
    hasReceivedAnalog = false;
}

bool isHappyCanTest() {
    return isGoodCanPackets && hasReceivedAnalog;
}

static bool wasBoardDetectError = false;

static void receiveBoardStatus(const uint8_t msg[8]) {
	int boardId = (msg[0] << 8) | msg[1];
	int numSecondsSinceReset = (msg[2] << 16) | (msg[3] << 8) | msg[4];

	chprintf(chp, " * BoardStatus: BoardID=%d numSecs=%d\r\n", boardId, numSecondsSinceReset);
	if (currentBoard == nullptr) {
		for (int boardIdx = 0; boardIdx < NUM_BOARD_CONFIGS; boardIdx++) {
			BoardConfig &c = boardConfigs[boardIdx];
			for (int boardRev = 0; c.boardIds[boardRev] > 0; boardRev++) {
				if (boardId == c.boardIds[boardRev]) {
					currentBoard = &c;
					currentBoardRev = boardRev;
					chprintf(chp, " * Board detected: %s rev.%c\r\n", currentBoard->boardName, 'A' + currentBoardRev);
				}
			}
		}
	}
	if (currentBoard == nullptr && !wasBoardDetectError) {
		canPacketError("Error! Couldn't detect, unknown board!\r\n");
		wasBoardDetectError = true;
	}
}

static void receiveRawAnalog(const uint8_t msg[8]) {
	// wait for the BoardStatus package first
	if (currentBoard == nullptr)
		return;
	hasReceivedAnalog = true;

	for (int ch = 0; ch < 8; ch++) {
		// channel not used for this board
		if (currentBoard->channels[ch].name == nullptr)
			continue;
		float voltage = getVoltageFrom8Bit(msg[ch]) * currentBoard->channels[ch].mulCoef;
		// check if in acceptable range for this board
		if (voltage < currentBoard->channels[ch].acceptMin || voltage > currentBoard->channels[ch].acceptMax) {
			canPacketError(" * BAD channel %d (%s): voltage %f (raw %d) not in range (%f..%f)\r\n",
				ch, currentBoard->channels[ch].name, voltage, msg[ch], 
				currentBoard->channels[ch].acceptMin, currentBoard->channels[ch].acceptMax);
		}
	}
}

static void printRxFrame(const CANRxFrame& frame, const char *msg) {
		chprintf(chp, "Processing %s ID=%x/l=%x %x %x %x %x %x %x %x %x\r\n",
		msg,
		        CAN_EID(frame),
		        frame.DLC,
				frame.data8[0], frame.data8[1],
				frame.data8[2], frame.data8[3],
				frame.data8[4], frame.data8[5],
				frame.data8[6], frame.data8[7]);
}

void processCanRxMessage(const CANRxFrame& frame) {
	if (CAN_EID(frame) == BENCH_TEST_BOARD_STATUS) {
	    printRxFrame(frame, "BENCH_TEST_BOARD_STATUS");
		receiveBoardStatus(frame.data8);
	} else if (CAN_EID(frame) == BENCH_TEST_RAW_ANALOG) {
	    printRxFrame(frame, "BENCH_TEST_RAW_ANALOG");
		receiveRawAnalog(frame.data8);
	} else if (CAN_EID(frame) == BENCH_TEST_EVENT_COUNTERS) {
	    printRxFrame(frame, "BENCH_TEST_EVENT_COUNTERS");
	} else if (CAN_EID(frame) == BENCH_TEST_BUTTON_COUNTERS) {
	    printRxFrame(frame, "BENCH_TEST_BUTTON_COUNTERS");
	}
}

static THD_WORKING_AREA(can_tx_wa, THREAD_STACK);
static THD_FUNCTION(can_tx, p) {
  CANTxFrame txmsg;

  (void)p;
  chRegSetThreadName("transmitter");
  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (true) {
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100));
    chThdSleepMilliseconds(500);
  }
}

static THD_WORKING_AREA(can_rx_wa, THREAD_STACK);
static THD_FUNCTION(can_rx, p) {
  CANRxFrame rxmsg;

  (void)p;
  while (true) {
    msg_t result = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_INFINITE);
	if (result != MSG_OK) {
		continue;
	}

    processCanRxMessage(rxmsg);
  }
}

void initCan() {
  palSetPadMode(CAN_PORT,CAN_PIN_RX, PAL_MODE_ALTERNATE(EFI_CAN_AF));
  palSetPadMode(CAN_PORT,CAN_PIN_TX, PAL_MODE_ALTERNATE(EFI_CAN_AF));

  canStart(&CAND1, &cancfg);

  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7,
                    can_tx, NULL);
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7,
                    can_rx, NULL);
}
