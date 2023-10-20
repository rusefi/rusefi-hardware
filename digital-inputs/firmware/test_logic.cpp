
#include "global.h"
#include "adc.h"
#include "test_digital_outputs.h"
#include "test_logic.h"
#include "rusefi/math.h"
#include "can.h"
#include "terminal_util.h"
#include "board_id/boards_id.h"
#include "board_id/boards_dictionary.h"
#include "board_id/qc_stim_meta.h"

#define COUNT 48
// 7.5% accuracy
#define ANALOG_L (1.0f - 0.075f)
#define ANALOG_H (1.0f + 0.075f)
// 10% for low voltage
#define ANALOG_H_FOR_LOW_VOLTAGE (1.0f + 0.12f)

#define HELLEN_VBATT_MULT 5.835f
#define HELLEN_R 4700
#define ALPHA2CH_R 2700
#define PROTEUS_R 2700

#define Vdivider 5.0f
#define IAT_VALUE(r) (Vdivider * 1000/(1000+r))
#define CLT_VALUE(r) (Vdivider * 2000/(2000+r))

// normal atmospheric pressure is 101.3 kPa
// transfer function taken from https://www.nxp.com/docs/en/data-sheet/MPXH6400A.pdf
#define MAP_MPX6400_VALUE (5.0f * (0.002421 * 101.3 - 0.00842))

#define VOLT_7B 0.5f
#define VOLT_8B 0.6f

#define PULLED_UP_VOLTAGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE) (Vdivider * RESISTOR_DIVIDER_LOWER_SIDE / (1.0f/(1.0f/RESISTOR_DIVIDER_UPPER_SIDE + 1.0f/RESISTANCE)+RESISTOR_DIVIDER_LOWER_SIDE))
#define PULLED_DOWN_VOLTAGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE) (Vdivider*(1.0f/(1.0f/RESISTOR_DIVIDER_LOWER_SIDE + 1.0f/RESISTANCE))/(RESISTOR_DIVIDER_UPPER_SIDE + 1.0f/(1.0f/RESISTOR_DIVIDER_LOWER_SIDE + 1.0f/RESISTANCE)))

#define PULLED_DOWN_RANGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE) /* mult*/1, (0.95f* PULLED_DOWN_VOLTAGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE)), 1.05f*PULLED_DOWN_VOLTAGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE)

// usually TPS1
#define UP_7B 820.0f
#define DOWN_7B 100.0f

// usually MAP
#define UP_8B 820.0f
#define DOWN_8B 120.0f

#define UP_9B 510.0f
#define DOWN_9B 100.0f

// uses AV_1
#define UP_10B 470.0f
#define DOWN_10B 100.0f

// uses AV_2
#define UP_11B 820.0f
#define DOWN_11B 220.0f

#define UP_12B 820.0f
#define DOWN_12B 330.0f

#define MAP_R 10000.0f

#define STATIC_ASSERT_EQ_FLOAT(v1, v2) (absF(v1 - v2) < 0.000001)

// let's just test our macro here
static_assert(STATIC_ASSERT_EQ_FLOAT(0.631685, PULLED_DOWN_VOLTAGE(DOWN_8B, UP_8B, 10000.0f)));
static_assert(STATIC_ASSERT_EQ_FLOAT(0.683484, PULLED_UP_VOLTAGE(DOWN_8B, UP_8B, 10000.0f)));

#define VOLT_9B 0.8f
#define VOLT_10B 0.9f
#define VOLT_11B 1.1f
#define VOLT_12B 1.4f
#define VOLT_12B 1.4f
#define VOLT_13B 1.6f
#define VOLT_14B 1.9f

#define VOLT_23C 2.1f
#define VOLT_24C 2.5f
#define VOLT_25C 2.9f
#define VOLT_26C 3.1f
#define VOLT_27C 3.4f
#define VOLT_28C 3.8f

extern BaseSequentialStream *chp;
bool haveSeenLow[COUNT];
bool haveSeenHigh[COUNT];

constexpr int cycleDurationMs = 100;
constexpr int cycleCount = 4;

BoardConfig boardConfigs[] = {
	{
		.boardName = "Hellen-Honda125K",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_HONDA125_A, BOARD_ID_HONDA125_B, BOARD_ID_HONDA125_C, BOARD_ID_HONDA125_D, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, VOLT_8B * ANALOG_L, VOLT_8B * ANALOG_H },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		.eventExpected = {true, false, true, true, false, false, true},
		.buttonExpected = {
		/*BrakePedal todo add wire */false,
		/*ClutchUp*/false,
		/*AcButton*/false},
		.outputNames = {"inj1", "inj2", "inj3", "inj4",
		"A/C relay", "idle air", "intake runner", "Lockout Solenoid",
		"Radiator", "B21 - EVAP", "B23 VTC VVT", "E7 Main Relay",
		"E31 Check Engine", "E1 Fuel Relay", "C11 Aux Low 3", "B18 Alternator Control",
        "E26 Tachometer", "B15 VTEC/VTS Output",
        "A30 - IGN1", "A29 - IGN2", "A28 - IGN3", "A27 - IGN4"
		},
	},
	{
		.boardName = "Proteus",
		.desiredEngineConfig = libPROTEUS_STIM_QC,
		.boardIds = { STATIC_BOARD_ID_PROTEUS_F4, STATIC_BOARD_ID_PROTEUS_F7, STATIC_BOARD_ID_PROTEUS_H7, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, VOLT_8B * ANALOG_L, VOLT_8B * ANALOG_H_FOR_LOW_VOLTAGE },
			{ "CLT", 1.0f, CLT_VALUE(PROTEUS_R) * ANALOG_L, CLT_VALUE(PROTEUS_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(PROTEUS_R) * ANALOG_L, IAT_VALUE(PROTEUS_R) * ANALOG_H },
			{ "BATT", 9.2f, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {true, true, true, true, false, false, true},
		.buttonExpected = {true, false, false},
		.outputNames = {},
	},
	{
		.boardName = "112-17",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_H112_17_A, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ "PPS2", 1, 0.96f, 10.4f },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // "TPS2_1"
			{ nullptr, 0, 0, 0 }, // "TPS2_2"
			{ nullptr, 0, 0, 0 }, // { "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {false, false, /*cam1*/true, false, false, false, /*vss*/true},
		.buttonExpected = {false, false, false},
		.outputNames = {"inj1", "inj2", "inj3", "inj4",
		"coil 1","coil 2","coil 3","coil 4",
		"pin 235", "pin 101", "pin 102", "main Relay",
		"pin 131", "pin 140", "Fan Relay", "Fuel Relay",
		"pin 151", "pin 152" }
	},
	{
		.boardName = "2chan",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_ALPHAX_2CHAN, BOARD_ID_ALPHA2CH_B, BOARD_ID_ALPHA2CH_C, BOARD_ID_ALPHA2CH_D,
			BOARD_ID_ALPHA2CH_E, BOARD_ID_ALPHA2CH_F, BOARD_ID_ALPHA2CH_G, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, MAP_MPX6400_VALUE * ANALOG_L, MAP_MPX6400_VALUE * ANALOG_H },	// internal MAP
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		.eventExpected = {true, true, true, true, true, true, true},
		.buttonExpected = {true, true, true},
		.outputNames = {},
	},
	{
		.boardName = "4chan",
		.desiredEngineConfig = libHELLEN_4CHAN_STIM_QC,
		.boardIds = { BOARD_ID_ALPHA4CH_H, BOARD_ID_ALPHA4CH_G, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, MAP_MPX6400_VALUE * ANALOG_L, MAP_MPX6400_VALUE * ANALOG_H },	// internal MAP
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		.eventExpected = {true, true, true, true, true, true, false},
		.buttonExpected = {false, false, false},
		.outputNames = {},
	},
	// https://github.com/rusefi/rusefi/wiki/Hellen-154-Hyundai
	{
		.boardName = "154HYUNDAI",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_154HYUNDAI_C, BOARD_ID_154HYUNDAI_D, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ "TPS1_2", 1.0f, VOLT_9B * ANALOG_L, VOLT_9B * ANALOG_H },
			{ "PPS1", 1.0f, VOLT_10B * ANALOG_L, VOLT_10B * ANALOG_H },
			{ "PPS2", 1.0f, /*VOLT_11B * ANALOG_L*/0.94, VOLT_11B * ANALOG_H },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, MAP_R)},
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			// 5B
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", 5.835, 9.0f, 15.0f },
		},
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/true, /*cam3*/false, /*cam4*/false, /*vss*//*low priority since CANtrue*/false},
		.buttonExpected = {/*BrakePedal*//*+12v button true*/false, /*ClutchUp*//*+12v button true*/false, /*AcButton*//*+12v button true*/false},
		.outputNames = {"inj1", "inj2", "inj3", "inj4",
		"vvt1", "vvt2",
		"Wastegate", "Fan Relay", "main Relay", "Fuel Relay",
		/*10*/"A/C Relay", "Second Fan Relay",
		"coil 1","coil 2","coil 3","coil 4",
		"MIL", "Tach",
		}
	},
};

BoardConfig *currentBoard = nullptr;
int16_t currentBoardRev = -1;

BoardConfig *getBoardConfigs() {
    return boardConfigs;
}

size_t getBoardsCount() {
    return efi::size(boardConfigs);
}

bool testEcuDigitalOutput(int testLineIndex, bool isLowSide) {
	memset(haveSeenLow, 0, sizeof(haveSeenLow));
	memset(haveSeenHigh, 0, sizeof(haveSeenHigh));

	setOutputAddrIndex(testLineIndex % 16);
	int adcIndex = testLineIndex / 16;

	bool isGood = true;

	for (int i = 0; i < cycleCount
	 //&& isGood
	 ; i++) {
		bool isSet = (i & 1) == 0;
		chprintf(chp, "               sending line=%d value=%d\r\n", index2human(testLineIndex), isSet);
		// toggle the ECU pin for low side mode
		sendCanPinState(testLineIndex, isSet ^ isLowSide);

		int scenarioIndex = 1; // i % 2;
		setScenarioIndex(scenarioIndex);
		// wait for the pin to toggle
		chThdSleepMilliseconds(cycleDurationMs);

		float voltage = getAdcValue(adcIndex);
		// low side sends roughly 2.8 but 5v high side is closer to 1v
		bool isHigh = voltage > 0.7;
		if (isHigh) {
			if (!haveSeenHigh[testLineIndex]) {
				chprintf(chp, "                      ADC says HIGH %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
			}
			haveSeenHigh[testLineIndex] = true;
		} else {
			if (!haveSeenLow[testLineIndex]) {
				chprintf(chp, "                      ADC says LOW %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
			}
			haveSeenLow[testLineIndex] = true;
		}

		// chprintf(chp, "scenario=%d: %1.3f V\r\n", scenarioIndex, voltage);

		bool cycleIsGood = (isHigh == isSet);
		if (!cycleIsGood) {
		    setErrorLedAndRedText();
			chprintf(chp, "ERROR! Cycle %d@%d FAILED! (set %d, received %d %1.3fv)\r\n", 
				index2human(testLineIndex), i, (isSet ? 1 : 0), (isHigh ? 1 : 0), voltage);
			setNormalText();
		}
		isGood = isGood && cycleIsGood;
	}

	// test is successful if we saw state toggle
	return isGood;
}

size_t totalStepsNumber() {
    return getDigitalInputStepsCount() + getDigitalOutputStepsCount();
}
