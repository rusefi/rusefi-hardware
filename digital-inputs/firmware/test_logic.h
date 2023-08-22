
#pragma once

#include "cstring"

#define MAX_ANALOG_CHANNELS 8
#define index2human(x) ((x) + 1)

bool testEcuDigitalOutput(int testLineIndex);

class AnalogChannelConfig {
public:
	const char *name;
	float mulCoef;	// equal to 'dividerCoef' from the firmware
	float acceptMin, acceptMax;
};

class BoardConfig {
public:
	const char *boardName;
	int boardId;
	AnalogChannelConfig channels[MAX_ANALOG_CHANNELS];
};

#define NUM_BOARD_CONFIGS 1
extern BoardConfig boardConfigs[NUM_BOARD_CONFIGS];

extern BoardConfig *currentBoard;
