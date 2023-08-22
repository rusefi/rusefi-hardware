
#include "global.h"
#include "adc.h"
#include "test_digital_outputs.h"
#include "test_logic.h"
#include "chprintf.h"
#include "board_id/boards_id.h"

#define COUNT 48
// 7.5% accuracy
#define ANALOG_L (1.0f - 0.075f)
#define ANALOG_H (1.0f + 0.075f)

#define IAT_VALUE (5.0f * 1000/(1000+4700))
#define CLT_VALUE (5.0f * 2000/(2000+4700))


extern BaseSequentialStream *chp;
bool haveSeenLow[COUNT];
bool haveSeenHigh[COUNT];

constexpr int cycleDurationMs = 1;
constexpr int cycleCount = 2500;
bool isGoodCanPackets = true;

BoardConfig boardConfigs[NUM_BOARD_CONFIGS] = {
	{
		"Hellen-Honda125K",
		BOARD_ID_HONDA125_D,
		{
			{ "TPS1_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, 0.6f * ANALOG_L, 0.6f * ANALOG_H },
			{ "CLT", 1.0f, CLT_VALUE * ANALOG_L, CLT_VALUE * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE * ANALOG_L, IAT_VALUE * ANALOG_H },
			{ "BATT", 5.835f, 9.0f, 15.0f },

		}
	},
};

BoardConfig *currentBoard = nullptr;

bool testEcuDigitalOutput(int testLineIndex) {
	memset(haveSeenLow, 0, sizeof(haveSeenLow));
	memset(haveSeenHigh, 0, sizeof(haveSeenHigh));

	setOutputAddrIndex(testLineIndex % 16);
	int adcIndex = testLineIndex / 16;

	bool isGood = false;

	for (int i = 0; i < cycleCount && !isGood; i++) {
		int scenarioIndex = 1; // i % 2;
		setScenarioIndex(scenarioIndex);
		chThdSleepMilliseconds(cycleDurationMs);

		float voltage = getAdcValue(adcIndex);
		bool isHigh = voltage > 1.5;
		if (isHigh) {
			if (!haveSeenHigh[testLineIndex]) {
				chprintf(chp, "  HIGH %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
			}
			haveSeenHigh[testLineIndex] = true;
		} else {
			if (!haveSeenLow[testLineIndex]) {
				chprintf(chp, "  LOW %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
			}
			haveSeenLow[testLineIndex] = true;
		}

		// chprintf(chp, "scenario=%d: %1.3f V\r\n", scenarioIndex, voltage);

		isGood = haveSeenHigh[testLineIndex] && haveSeenLow[testLineIndex];
	}

	// test is successful if we saw state toggle
	return isGood;
}

void canPacketError(const char *msg, ...) {
	va_list vl;
	va_start(vl, msg);
	chvprintf(chp, msg, vl);
	va_end(vl);

	isGoodCanPackets = false;
}
