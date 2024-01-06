// test_logic.h

#pragma once

#include "cstring"

#define MAX_ANALOG_CHANNELS 16
#define MAX_BOARD_REVS 32
#define MAX_OUTPUT_NAMES 64
#define index2human(x) ((x) + 1)
#define CAN_FRAME_SIZE 8
/**
 * we use 16 channel chips on the board
 */
#define DIGITAL_INPUT_BANK_SIZE 16

bool testEcuDigitalOutput(int testLineIndex, bool isLowSide);
bool testDcOutput(size_t dcIndex);

class Counter {
public:
	int canFrameIndex;
	const char *name;
	bool nonZero = false;
};

#define EVENT_ENUM_SIZE 7
#define BUTTON_ENUM_SIZE 3

class CounterStatus {
public:
	Counter eventCounters[EVENT_ENUM_SIZE] = {
		{ 0, "Primary Trigger" },
		{ 1, "Secondary Trigger" },
		{ 2, "VVT1 Sensor" },
		{ 3, "VVT2 Sensor" },
		{ 4, "VVT3 Sensor" },
		{ 5, "VVT4 Sensor" },
		{ 6, "VSS" },					
	};

	Counter buttonCounters[BUTTON_ENUM_SIZE] = {
		{ 0, "BrakePedal" },
		{ 1, "ClutchUp" },
		{ 2, "AcButton" },
	};
};

class AnalogChannelConfig {
public:
	const char *name;
	float mulCoef;	// equal to 'dividerCoef' from the firmware
	float acceptMin, acceptMax;
};

class BoardConfig {
public:
	const char *boardName;
	int desiredEngineConfig;
	uint16_t boardIds[MAX_BOARD_REVS];
	AnalogChannelConfig channels[MAX_ANALOG_CHANNELS];
	bool eventExpected[EVENT_ENUM_SIZE];
	bool buttonExpected[BUTTON_ENUM_SIZE];
	const char *outputNames[MAX_OUTPUT_NAMES];
	int wboUnitsCount;
    // do we have some defect in the logic or loose state? does DC validation depend on if we have just finished testing low-side or high-side pins?
	int dcHackValue;

	const char *getOutputName(size_t index) {
	    return outputNames[index];
//	    for (size_t i = 0;i<MAX_OUTPUT_NAMES;i++) {
//	        if (outputNames[i] == nullptr) {
//	            return nullptr;
//	        }
//	        if (i == index)
//	    }
//        return nullptr;

	}
};

BoardConfig *getBoardConfigs();
size_t getBoardsCount();

extern BoardConfig *currentBoard;
extern int16_t currentBoardRev;

size_t getDigitalInputStepsCount();
size_t totalStepsNumber();

