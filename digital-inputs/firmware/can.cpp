#include "can_hw.h"
#include "io_pins.h"
#include "can.h"
#include "test_logic.h"
#include "can/can_common.h"
#include "global.h"
#include "terminal_util.h"
#include "wideband_can.h"

extern BaseSequentialStream *chp;
extern OutputMode outputMode;

static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};

#define BENCH_HEADER ((int)bench_test_magic_numbers_e::BENCH_HEADER)

static bool isGoodCanPackets = true;
static bool hasReceivedAnalog = false;
static bool hasReceivedBoardId = false;
static CounterStatus counterStatus;
static size_t outputCount = 0;
static size_t dcOutputsCount = 0;
static size_t lowSideOutputCount = 0;

static int boardId;

extern bool globalEverythingHappy;
extern bool isMuted;

static void canPacketError(const char *msg, ...) {
    setErrorLedAndRedText();
	chprintf(chp, " *********************************************** \r\n");

	va_list vl;
	va_start(vl, msg);
	chvprintf(chp, msg, vl);
	va_end(vl);

	chprintf(chp, " *********************************************** \r\n");
	setNormalText();

	isGoodCanPackets = false;
	globalEverythingHappy = false;
}

static bool rawReported[128];

static bool hasSeenWbo1;
static bool hasSeenWbo2;

void startNewCanTest() {
    isGoodCanPackets = true;
    hasReceivedAnalog = false;
    hasReceivedBoardId = false;
    hasSeenWbo1 = false;
    hasSeenWbo2 = false;
    currentBoard = nullptr;
    dcOutputsCount = outputCount = 0;
    lowSideOutputCount = 0;
    // reset
	counterStatus = CounterStatus();
	memset(&rawReported, 0, sizeof(rawReported));
}

bool isHappyCanTest() {
    bool isGoodWbo1 = currentBoard->wboUnitsCount < 1 || hasSeenWbo1;
    bool isGoodWbo2 = currentBoard->wboUnitsCount < 2 || hasSeenWbo2;

    if (!isGoodWbo1 || !isGoodWbo2) {
        setRedText();
		chprintf(chp, "* WBO comms issue\n");
		setNormalText();
    }

    return isGoodWbo1 && isGoodWbo2 && isGoodCanPackets && hasReceivedAnalog;
}

static void handleCounter(Counter *cnt, bool *isHappy, bool *eventExpected, const char *suffix) {
		if (!eventExpected[cnt->canFrameIndex])
			return;
		if (cnt->nonZero) {
		    setGreenText();
        	chprintf(chp, "* HAPPY %s %s counter!\r\n", cnt->name, suffix);
        	setNormalText();
		} else {
		    setErrorLedAndRedText();
			chprintf(chp, "* ZERO %s %s counter!\r\n", cnt->name, suffix);
			setNormalText();
		}
		*isHappy = *isHappy && cnt->nonZero;
}

bool checkDigitalInputCounterStatus() {
	if (currentBoard == nullptr) {
		setErrorLedAndRedText();
		chprintf(chp, "* UNKNOWN BOARD ID [%d] while trying to check digital input event counter!\r\n", boardId);
	    setNormalText();
		return false;
	}

	bool isHappy = true;
	
	for (auto & evtCnt : counterStatus.eventCounters) {
	    handleCounter(&evtCnt, &isHappy, currentBoard->eventExpected, "event");
	}
	
	for (auto & btnCnt : counterStatus.buttonCounters) {
	    handleCounter(&btnCnt, &isHappy, currentBoard->buttonExpected, "button");
	}

	for (auto & btnCnt : counterStatus.auxDigitalCounters) {
	    handleCounter(&btnCnt, &isHappy, currentBoard->auxDigitalExpected, "aux digital");
	}

	return isHappy;
}

int getDigitalOutputStepsCount() {
	return outputCount;
}

int getDigitalDcOutputStepsCount() {
	return dcOutputsCount;
}

int getLowSideOutputCount() {
	return lowSideOutputCount;
}

static bool wasBoardDetectError = false;
int numSecondsSinceReset;

static void receiveBoardStatus(const uint8_t msg[CAN_FRAME_SIZE]) {
	numSecondsSinceReset = (msg[2] << 16) | (msg[3] << 8) | msg[4];
	if (hasReceivedBoardId) {
	    return;
	}
	hasReceivedBoardId = true;

	boardId = (msg[0] << 8) | msg[1];
	int engineType = (msg[5] << 8) | msg[6];

	if (outputMode.displayCanReceive) {
	    chprintf(chp, "       CAN RX BoardStatus: BoardID=%d numSecs=%d\r\n", boardId, numSecondsSinceReset);
	}
	if (currentBoard == nullptr) {
		for (size_t boardIdx = 0; boardIdx < getBoardsCount(); boardIdx++) {
			BoardConfig &c = getBoardConfigs()[boardIdx];
			for (int boardRev = 0; c.boardIds[boardRev] > 0; boardRev++) {
				if (boardId == c.boardIds[boardRev]) {
					currentBoard = &c;
					currentBoardRev = boardRev;
					// index in the list does not directly map to board revision
					chprintf(chp, " * Board detected: %s rev index %d\r\n", currentBoard->boardName, currentBoardRev);

					if (c.desiredEngineConfig != -1 && c.desiredEngineConfig != engineType) {
					    sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (int)bench_test_io_control_e::CAN_BENCH_SET_ENGINE_TYPE, c.desiredEngineConfig });

					    chprintf(chp, " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
					    chprintf(chp, " !!!!!!!!!!!!!!!!!!!!!!!!!!! changing engine type !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
					    chprintf(chp, " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
					}
				}
			}
		}
	}
	if (currentBoard == nullptr && !wasBoardDetectError) {
		canPacketError("Error! Couldn't detect, unknown board!\r\n");
		wasBoardDetectError = true;
	}
}

static void receiveOutputMetaInfo(const uint8_t msg[CAN_FRAME_SIZE]) {
	if (msg[0] == BENCH_HEADER) {
		outputCount = msg[2];
		lowSideOutputCount = msg[3];
		dcOutputsCount = msg[4];
    	if (outputMode.displayCanReceive && !isMuted) {
    	    chprintf(chp, "       CAN ECU says: total=%d outputs of which low side=%d also %d DC\r\n", outputCount, lowSideOutputCount, dcOutputsCount);
    	}
	}
}

static void receiveRawAnalog(const uint8_t msg[CAN_FRAME_SIZE], size_t offset) {
	// wait for the BoardStatus package first
	if (currentBoard == nullptr)
		return;
	hasReceivedAnalog = true;


	for (size_t byteIndex = 0; byteIndex < CAN_FRAME_SIZE; byteIndex++) {
        size_t ch = offset + byteIndex;
		// channel not used for this board
		if (currentBoard->channels[ch].name == nullptr)
			continue;
		float voltage = getVoltageFrom8Bit(msg[byteIndex]) * currentBoard->channels[ch].mulCoef;


		// check if in acceptable range for this board
		if (voltage < currentBoard->channels[ch].acceptMin || voltage > currentBoard->channels[ch].acceptMax) {
			canPacketError(" * BAD channel %d (%s): voltage %f (raw %d) not in range (%f..%f)\r\n",
				ch, currentBoard->channels[ch].name, voltage, msg[byteIndex],
				currentBoard->channels[ch].acceptMin, currentBoard->channels[ch].acceptMax);
		} else {
		     if (!rawReported[ch]) {
		        rawReported[ch] = true;
        		setGreenText();
        		chprintf(chp, " ************* %s analog %d %d voltage=%f within range %f/%f\r\n",
        		    currentBoard->channels[ch].name,
        		    offset,
        		    byteIndex,
        		    voltage,
        		    currentBoard->channels[ch].acceptMin, currentBoard->channels[ch].acceptMax);
        		setNormalText();
             }
        }
	}
}

static void receiveEventCounters(const uint8_t msg[CAN_FRAME_SIZE]) {
	for (auto & evtCnt : counterStatus.eventCounters) {
		evtCnt.nonZero = evtCnt.nonZero || (msg[evtCnt.canFrameIndex] > 0);
	}
}

static void receiveButtonCounters(const uint8_t msg[CAN_FRAME_SIZE]) {
	for (auto & btnCnt : counterStatus.buttonCounters) {
		btnCnt.nonZero = btnCnt.nonZero || (msg[btnCnt.canFrameIndex] > 0);
	}
}

static void receiveAuxDigitalCounters(const uint8_t msg[CAN_FRAME_SIZE]) {
	for (auto & cnt : counterStatus.auxDigitalCounters) {
		cnt.nonZero = cnt.nonZero || (msg[cnt.canFrameIndex] > 0);
	}
}

static void printRxFrame(const CANRxFrame& frame, const char *msg) {
    if (!outputMode.displayCanReceive || isMuted) {
        return;
    }
		chprintf(chp, "                          Processing %s ID=%x/l=%x %x %x %x %x %x %x %x %x\r\n",
		msg,
		        CAN_EID(frame),
		        frame.DLC,
				frame.data8[0], frame.data8[1],
				frame.data8[2], frame.data8[3],
				frame.data8[4], frame.data8[5],
				frame.data8[6], frame.data8[7]);
}

void processCanRxMessage(const CANRxFrame& frame) {
#if 0
		setGreenText();
		chprintf(chp, " ************* GOT CAN %x\r\n",
		    CAN_EID(frame));
		setNormalText();
#endif

    int standardId = CAN_SID(frame);
    int extendedId = CAN_EID(frame);

	if (extendedId == (int)bench_test_packet_ids_e::BOARD_STATUS) {
        printRxFrame(frame, "BENCH_TEST_BOARD_STATUS");
		receiveBoardStatus(frame.data8);
	} else if (extendedId == (int)bench_test_packet_ids_e::RAW_ANALOG_1) {
	    printRxFrame(frame, "BENCH_TEST_RAW_ANALOG_1");
		receiveRawAnalog(frame.data8, 0);
	} else if (extendedId == (int)bench_test_packet_ids_e::RAW_ANALOG_2) {
	    printRxFrame(frame, "BENCH_TEST_RAW_ANALOG_2");
        receiveRawAnalog(frame.data8, 8);
	} else if (extendedId == (int)bench_test_packet_ids_e::EVENT_COUNTERS) {
	    printRxFrame(frame, "EVENT_COUNTERS");
	    receiveEventCounters(frame.data8);
	} else if (extendedId == (int)bench_test_packet_ids_e::BUTTON_COUNTERS) {
	    printRxFrame(frame, "BUTTON_COUNTERS");
	    receiveButtonCounters(frame.data8);
	} else if (extendedId == (int)bench_test_packet_ids_e::AUX_DIGITAL_COUNTERS) {
	    printRxFrame(frame, "AUX_DIGITAL_COUNTERS");
	    receiveAuxDigitalCounters(frame.data8);
	} else if (extendedId == (int)bench_test_packet_ids_e::IO_META_INFO) {
	    printRxFrame(frame, "BENCH_TEST_IO_META_INFO");
	    receiveOutputMetaInfo(frame.data8);
	} else if (standardId == WB_DATA_BASE_ADDR) {
	    if (!hasSeenWbo1) {
	        setCyanText();
	        chprintf(chp, " ***** WBO1 packet\n");
	        hasSeenWbo1 = true;
	        setNormalText();
	    }
	} else if (standardId == (WB_DATA_BASE_ADDR + 2)) {
	    if (!hasSeenWbo1) {
	        setCyanText();
	        chprintf(chp, " ***** WBO2 packet\n");
	        hasSeenWbo2 = true;
	        setNormalText();
	    }
	}
}

void sendCanPinState(uint8_t pinIdx, bool isSet) {
	sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (uint8_t)(isSet ? (int)bench_test_io_control_e::CAN_QC_OUTPUT_CONTROL_SET : (int)bench_test_io_control_e::CAN_QC_OUTPUT_CONTROL_CLEAR), pinIdx });
}

void sendCanDcState(uint8_t dcIndex, bool isSet) {
	sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (int)bench_test_io_control_e::CAN_QC_ETB, dcIndex, isSet });
}

void setOutputCountRequest() {
	sendCanTxMessage((int)bench_test_packet_ids_e::IO_CONTROL, { BENCH_HEADER, (int)bench_test_io_control_e::CAN_BENCH_GET_COUNT });
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

  initCanHw();
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7,
                    can_rx, NULL);
}
