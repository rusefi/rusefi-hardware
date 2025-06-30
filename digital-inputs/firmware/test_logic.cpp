
#include "global.h"
#include "adc.h"
#include "test_digital_outputs.h"
#include "test_logic.h"
#include "rusefi/rusefi_math.h"
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

#define LAST_DIGITAL_PIN 39

#define HELLEN_VBATT_MULT 5.835f
#define HELLEN_R 4700
#define ALPHA2CH_R 2700
#define MRE_DEFAULT_AT_PULLUP 2700
#define PROTEUS_R 2700
#define PROTEUS_VBATT_MULT 9.2f

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

#define PULLED_DOWN_RANGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE) /* mult*/1, (0.937f* PULLED_DOWN_VOLTAGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE)), 1.05f*PULLED_DOWN_VOLTAGE(RESISTOR_DIVIDER_LOWER_SIDE, RESISTOR_DIVIDER_UPPER_SIDE, RESISTANCE)

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

#define UP_13B 470.0f
#define DOWN_13B 220.0f

#define UP_14B 820.0f
#define DOWN_14B 510.0f

#define MAP_R 10000.0f

#define STATIC_ASSERT_EQ_FLOAT(v1, v2) (absF(v1 - v2) < 0.000001)

// let's just test our macro here
static_assert(STATIC_ASSERT_EQ_FLOAT(0.631685, PULLED_DOWN_VOLTAGE(DOWN_8B, UP_8B, 10000.0f)));
static_assert(STATIC_ASSERT_EQ_FLOAT(0.683484, PULLED_UP_VOLTAGE(DOWN_8B, UP_8B, 10000.0f)));

#define VOLT_9B 0.8f
#define VOLT_10B 0.9f
#define VOLT_11B 1.1f
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

struct DigitalResult {
public:
    bool haveSeenLow;
    bool haveSeenHigh;
};

constexpr int cycleCount = 4;

BoardConfig boardConfigs[] = {
	{
		.boardName = "mre-m111",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_MRE_M111, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, VOLT_8B * ANALOG_L, VOLT_8B * ANALOG_H },
			{ "CLT", 1.0f, CLT_VALUE(MRE_DEFAULT_AT_PULLUP) * ANALOG_L, CLT_VALUE(MRE_DEFAULT_AT_PULLUP) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(MRE_DEFAULT_AT_PULLUP) * ANALOG_L, IAT_VALUE(MRE_DEFAULT_AT_PULLUP) * ANALOG_H },
			{ "BATT", 8.23, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "oil", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
			{ "fuel-low", 1.0f, VOLT_12B * ANALOG_L, VOLT_12B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
		},
		.eventExpected = {/*crank*/false, false, /*cam1*/true, false, false, false, false},
		.buttonExpected = {
		/*BrakePedal todo add wire */false,
		/*ClutchUp*/false,
		/*AcButton*/false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {"inj1", "inj2", "inj3", "inj4",
		"boost",
		"VVT LS1",
		"SC clutch LS2",
		"SC Bypass",
		},
		.wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "gold",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_ALPHAX_GOLD, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		true,
		false,
		/*cam1*/true,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "1B - Injector 4",
 "2B - Injector 3",
 "3B - Injector 2",
 "4B - Injector 1",
 "5B Fuel Pump Relay",
 "6B Idle Output",
 "14B - Tachometer Output",
 "1A - Injector 8",
 "2A - Injector 7",
 "3A - Injector 6",
 "4A - Injector 5",
 "5A Main Relay",
 "6A LS3",
 "7A LS4",
 "8A LS5",
 "13B Coil 1",
 "12B Coil 2",
 "11B Coil 3",
 "10B Coil 4",
 "13A Coil 5",
 "12A Coil 6",
 "11A Coil 7",
 "10A Coil 8",
	    },
        .wboUnitsCount = 2,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "subaru",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_PROTEUS_SUBARU_2011, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		true,
		false,
		/*cam1*/false,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "4-10 - Injector 1",
 "4-11 - Injector 2",
 "4-12 - Injector 3",
 "4-13 - Injector 4",
 "5-13 - Main relay",
 "4-14 - AVCS Exhaust LH",
 "4-15 - AVCS Exhaust RH",
 "5-26 - Starter relay",
	    },
        .wboUnitsCount = 1,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "chuma",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_CHUMA, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ nullptr, 0, 0, 0 }, // TPS1_2
			{ nullptr, 0, 0, 0 }, // PPS1
			{ nullptr, 0, 0, 0 }, // PPS2
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, 1.61 /*1.604477632 CLT_VALUE(HELLEN_R) * ANALOG_H */ },
			{ nullptr, 0, 0, 0 }, // { "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		true,
		false,
		/*cam1*/false,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "10 injector",
 "11 fuel pump",
 "2 idle",
 "9 coil",
 "8 tach",
	    },
        .wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "nano",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_NANO, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ nullptr, 0, 0, 0 }, // TPS1_2
			{ nullptr, 0, 0, 0 }, // PPS1
			{ nullptr, 0, 0, 0 }, // PPS2
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, 1.63 /*1.604477632 CLT_VALUE(HELLEN_R) * ANALOG_H */ },
			{ nullptr, 0, 0, 0 }, // { "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		true,
		false,
		/*cam1*/false,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "injector1",
 "OUT_LS",
 "OUT_INJ2",
 "OUT_IDLE",
 "OUT_FUEL_PUMP_REL",
 "OUT_IGN1",
 "OUT_IGN2",
 "OUT_TACH",
	    },
        .wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "e92",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_HELLEN_E92, 0 },
		.channels = {
			{ nullptr, 0, 0, 0 }, // SENT not TPS1_1
			{ nullptr, 0, 0, 0 }, // SENT not TPS1_2
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ "MAP", 1, 0.56 /* 0.597992960 PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) */, 0.670109504},
			{ nullptr, 0, 0, 0 }, // grey { "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, 0.80 /* 0.811403520 IAT_VALUE(HELLEN_R) * ANALOG_L*/, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		true,
		false,
		/*cam1*/true,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "Main relay control output",
 "OIL PUMP COMMAND SOLENOID",
 "CAM SHAFT VVT ACTUATOR LO",
 "ECU wake-up relay control output",
 "A/C-CLUTCH RLY CTRL",
 // high side
 "IGN CTRL 3",
 "IGN CTRL 4",
 "IGN CTRL 5",
 "IGN CTRL 6",
 "IGN CTRL 7",
 "IGN CTRL 8",
 "FUEL PUMP CONTROLLER DATA OUT SIG",
	    },
        .wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSizeStartingIndex = 31 /* human index 32 */
	},
	{
		.boardName = "Hellen-Honda125K",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_HONDA125_A, BOARD_ID_HONDA125_B, BOARD_ID_HONDA125_C, BOARD_ID_HONDA125_D, BOARD_ID_HONDA125_E, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, VOLT_8B * ANALOG_L, VOLT_8B * ANALOG_H },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, 0.8 /* why not 0.8114? IAT_VALUE(HELLEN_R) * ANALOG_L*/, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		.eventExpected = {true, false, true, true, false, false, true},
		.buttonExpected = {
		/*BrakePedal todo add wire */false,
		/*ClutchUp*/false,
		/*AcButton*/false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {"inj1", "inj2", "inj3", "inj4",
		"A/C relay", "idle air", "intake runner", "Lockout Solenoid",
		"Radiator", "B21 - EVAP", "B23 VTC VVT", "E7 Main Relay",
		"E31 Check Engine", "E1 Fuel Relay", "C11 Aux Low 3", "B18 Alternator Control",
        "E26 Tachometer", "B15 VTEC/VTS Output",
        "A30 - IGN1", "A29 - IGN2", "A28 - IGN3", "A27 - IGN4"
		},
		.wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
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
			{ "BATT", PROTEUS_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {true, true, true, true, false, false, true},
		.buttonExpected = {true, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {"LS 1", "LS 2", "LS 3", "LS 4",
		"LS 5", "LS 6", "LS 7", "LS 8",
		"LS 9", "LS 10", "LS 11", "LS 12",
		"LS 13", "LS 14", "LS 15", "LS 16",
		"IGN 1", "IGN 2", "IGN 3", "IGN 4",
		"IGN 5", "IGN 6", "IGN 7", "IGN 8",
		"IGN 9", "IGN 10", "IGN 11", "IGN 12",
		"HS 1", "HS 2", "HS 3", "HS 4",
				},
		.wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "mg1",
		.desiredEngineConfig = -1,
		.boardIds = {
		// a bit of a hack - lazy reuse for most basic pins
		STATIC_BOARD_ID_PROTEUS_SLINGSHOT,
		STATIC_BOARD_ID_PROTEUS_CANAM, STATIC_BOARD_ID_HELLEN_MG1, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
//			{ nullptr, 0, 0, 0 }, // skipping for Proteus
//			{ nullptr, 0, 0, 0 }, // skipping for Proteus
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
// proteus			{ "BATT", PROTEUS_VBATT_MULT, 9.0f, 15.0f },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {/*crank*/true,
		false,
		/*cam1*/true,
		false, false, false,
		/*vss*/true},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "injector 1",
 "injector 2",
 "injector 3",
 "Main Relay Output Control",
 "Starter Relay Output",
 "Intercooler Fan Output",
 "Accessory Relay Output",
		"Coil 1",
		"Coil 2",
		"Coil 3",
 "Fuel Pump 149",
 "Wastegate Solenoid Output",
 "DESS pin A",
 "Fan Relay Output",
 "Headlight Relay Output",

	    },
        .wboUnitsCount = 1,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "4k-gdi",
		.desiredEngineConfig = -1,
		.boardIds = {
		BOARD_ID_GDI4CHAN_A,
		BOARD_ID_GDI4CHAN_B,
		0 },
		.channels = {
			{ "TPS1_1", 1, 0.490196096, /* ideally 0.509172384..* PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) */ 0.570577344},
			{ "TPS1_2", 1, 0.76, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, 0.8 /* 0.81 IAT_VALUE(HELLEN_R) * ANALOG_L*/, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {/*crank*/false,
		false,
		/*cam1*/true,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "14A Ignition 1",
 "13A Ignition 2",
 "12A Ignition 3",
 "11A Ignition 4",
 "5B Main Relay Control",
 "7B Fuel Pump",
 "8B Fan 1",
 "9B Fan 2",
 "29B VVT1",
 "30B VVT2",
 "31B VVT3",
 "32B VVT4",
 "33A Wastegate Solenoid",
 "34A AC Control",
 "3A GDI Injector 4",
 "5A GDI Injector 3",
 "7A GDI Injector 2",
 "9A GDI Injector 1",
	    },
        .wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "polaris",
		.desiredEngineConfig = -1,
		.boardIds = {
		STATIC_BOARD_ID_HELLEN_POLARIS, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {/*crank*/false,
		false,
		/*cam1*/true,
		false, false, false,
		/*vss*/true},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "injector 1",
 "injector 2",
 "injector 3",
 "injector 4",
 "Main Relay Output Control",
 "Coil 1",
 "Coil 2",
 "Coil 3",
 "Coil 4",
 "Starter Relay Output",
 "Fuel Pump Relay Output",
 "Engine Fan PWM Output",
 "lights relay control",
 "AWD coil LO",
 "diff solenoid 204",
	    },
        .wboUnitsCount = 1,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "M73",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_PROTEUS_M73, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },

			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(PROTEUS_R) * ANALOG_L, CLT_VALUE(PROTEUS_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(PROTEUS_R) * ANALOG_L, IAT_VALUE(PROTEUS_R) * ANALOG_H },
			{ "BATT", PROTEUS_VBATT_MULT, 9.0f, 15.0f },

            // see can_bench_test.cpp values_2
			{ "TPS2_1", 1.0f, VOLT_13B * ANALOG_L, VOLT_13B * ANALOG_H },
			{ "TPS2_2", 1.0f, VOLT_14B * ANALOG_L, VOLT_14B * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "oil", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
//			{ "fuel-low", 1.0f, VOLT_12B * ANALOG_L, VOLT_12B * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // todo remove
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
		},
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		.eventExpected = {/*crank*/true,
		false,
		/*cam1*/true,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
 "injector 1",
 "injector 2",
 "injector 3",
 "injector 4",
 "injector 5",
 "injector 6",
 "injector 7",
 "injector 8",
 "injector 9",
 "injector 10",
 "injector 11",
 "injector 12",
	    },
        .wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "hd",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_PROTEUS_HARLEY, STATIC_BOARD_ID_HELLEN_HD, BOARD_ID_HD81_A, BOARD_ID_HD81_B, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
//			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
//			{ "PPS1",   PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
//			{ "MAP", 1, 0.62, 0.670109504 },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		true,
		false,
		/*cam1*/true,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "Injector 2",
 "Injector 1",
"25 Front ACR",
 "8 Rear ACR",
"VVT Control",
"63 Cooling Pump",
"24 Front Coil 2",
"43 Rear Coil 1",
"4 Left Oil Fan / Coolant Pump",
"1 Right Oil Cooling / Coolant Fan",
"44 fan",
	    },
        .wboUnitsCount = 2,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "121vag",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_HELLEN_121VAG, 0 },
		.channels = {
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
//			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
//			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
//			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
//			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
//			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
//			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
//			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
//			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		false,
		false,
		/*cam1*/false,
		false, false, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
	    },
        .wboUnitsCount = 1,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "121nissan",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_NISSAN121_D, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", 1, 0.767938368, /* ideally 0.860549952*/ 0.87 },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", 1, 0.96, 1.2 },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },

//			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
//			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
//			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
//			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		/*crank*/true,
		false,
		/*cam1 bank 1*/true, /*cam2 bank 1*/false,
		/*cam1 bank 2*/true, false,
		/*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "111 Main Relay",
 "113 Fuel Pump Relay",
 "21 - INJ_5",
 "22 - INJ_3",
 "23 - INJ_1",
 "40 - INJ_6",
 "41 - INJ_4",
 "42 - INJ_2",
 "10 - VTC Left",
 "11 - VTC Right",
	    },
        .wboUnitsCount = 2,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "112-17",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_H112_17_A, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ "PPS2", 1, 0.96f, 1.4f },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // "TPS2_1"
			{ nullptr, 0, 0, 0 }, // "TPS2_2"
			{ nullptr, 0, 0, 0 }, // { "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		.eventExpected = {/*crank*/true, false, /*cam1*/true, false, false, false, /*vss*/true},
		.buttonExpected = {false, false, false},
				.auxDigitalExpected = {false, false, false, false,
        		false, false, false, false},

		.outputNames = {"inj1", "inj2", "inj3", "inj4",
		"coil 1","coil 2","coil 3","coil 4",
		"pin 235", "pin 101", "pin 102", "main Relay",
		"pin 131", "pin 140", "Fan Relay", "Fuel Relay",
		"pin 151", "pin 152", "pin 246" },
		.wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "uaefi",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_UAEFI_A, BOARD_ID_UAEFI_B, 0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ "PPS2", 1, 0.96f, 1.4f },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // "TPS2_1"
			{ nullptr, 0, 0, 0 }, // "TPS2_2"
			{ nullptr, 0, 0, 0 }, // { "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		/* crank neg C19 goes to 24C for 2.5v virtual GND. Crank positive 22B with a 4.7K pull up to 5v */
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/true, false, false, /*vss*/true},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "B1 injector output 6",
 "B2 injector output 5",
 "B3 injector output 4",
 "B4 injector output 3",
 "B5 injector output 2",
 "B6 injector output 1",
 "B7 Low Side output 1",
 "B8 Weak Low Side output 2",
 "B9 Weak Low Side output 1",
 "B16 Low Side output 4",
 "B17 Low Side output 3",
 "B18 Low Side output 2",
 "B10 Coil 6",
 "B11 Coil 4",
 "B12 Coil 3",
 "B13 Coil 5",
 "B14 Coil 2",
 "B15 Coil 1",
        },
        .wboUnitsCount = 1,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
 	},
	{
		.boardName = "uaefi121",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_UAEFI121_A, BOARD_ID_UAEFI121_B, 0 },
		.channels = {
			{ nullptr, 0, 0, 0 }, // { "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "PPS2", 1, 0.96f, 1.4f },
			{ nullptr, 0, 0, 0 }, // { "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // "TPS2_1"
			{ nullptr, 0, 0, 0 }, // "TPS2_2"
			{ nullptr, 0, 0, 0 }, // { "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		/* crank neg C19 goes to 24C for 2.5v virtual GND. Crank positive 22B with a 4.7K pull up to 5v */
		.eventExpected = {/*crank*/false, false, /*cam1*/true, /*cam2*/true, false, false, /*vss*/true},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "44a INJ_1",
 "45a INJ_2",
 "46a INJ_3",
 "47a INJ_4",
 "48a INJ_5",
 "49a INJ_6",
 "50a INJ_7",
 "51a INJ_8",
 "14a LS1",
 "15a LS2",
 "16a LS3",
 "88a LS4",
 "ls5",
 "ls6",
 "high side",
 "Coil 1",
 "Coil 2",
 "Coil 3",
 "Coil 4",
 "Coil 5",
 "Coil 6",

        },
        .wboUnitsCount = 2,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
 	},
	{
		.boardName = "uaefi121-sbc",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_UAEFU121_SBC, 0 },
		.channels = {
			{ nullptr, 0, 0, 0 }, // { "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "PPS2", 1, 0.96f, 1.4f },
			{ nullptr, 0, 0, 0 }, // { "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // "TPS2_1"
			{ nullptr, 0, 0, 0 }, // "TPS2_2"
			{ nullptr, 0, 0, 0 }, // { "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {/*crank*/true, false, /*cam1*/false, /*cam2*/false, false, false, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "44a INJ_1",
 "45a INJ_2",
 "46a INJ_3",
 "47a INJ_4",
 "48a INJ_5",
 "49a INJ_6",
 "50a INJ_7",
 "51a INJ_8",
 "fuel pump",
 "Coil 1",
 "Coil 2",
        },
        .wboUnitsCount = 2,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
 	},
	{
		.boardName = "super-uaefi",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_SUPER_UAEFI, 0 },
		.channels = {
			{ nullptr, 0, 0, 0 }, // { "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "PPS2", 1, 0.96f, 1.4f },
			{ nullptr, 0, 0, 0 }, // { "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ nullptr, 0, 0, 0 }, // { "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // "TPS2_1"
			{ nullptr, 0, 0, 0 }, // "TPS2_2"
			{ nullptr, 0, 0, 0 }, // { "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		/* crank neg C19 goes to 24C for 2.5v virtual GND. Crank positive 22B with a 4.7K pull up to 5v */
		.eventExpected = {/*crank*/false, false, /*cam1*/true, /*cam2*/true, false, false, /*vss*/true},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "INJ_1",
        },
        .wboUnitsCount = 2,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
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
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {},
		.wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "4chan",
		.desiredEngineConfig = libHELLEN_4CHAN_STIM_QC,
		.boardIds = { BOARD_ID_ALPHA4CH_H, BOARD_ID_ALPHA4CH_G, BOARD_ID_ALPHA4CH_I, 0 },
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
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "A8 - Injector 1",
 "B8 - Injector 2",
 "D1 - Injector 3",
 "E1 - Injector 4",
 "F2 - VVT#1 rev G",
 "F4 - VVT#2 rev G",
 "F1 - Idle2",
 "C8 - Idle",
		},
		.wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "8chan",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_ALPHA8CH_C,
		315/*?!?!*/,
		0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },//{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/true,/*cam3*/ true, /*cam4*/true, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
		"inj1", "inj2", "inj3", "inj4",
		"inj5", "inj6", "inj7", "vvt1",
		"fuel", "fan", "main", "nos",
		"inj8", "vvt2", "tach", "ls1",
		"inj9", "vvt3", "vvt4", "boost",
		"ls2", "inj12", "inj11", "inj10",
		"ls3", "ls4", "ls5", "ls6",
		/*34C > 9C*/"ls7", /*5C > */"hs1", "hs2", "hs3",
        "ign1", "ign3", "ign5",
		},
		.wboUnitsCount = 2,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "8chan-gm-gen4",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_PLATINUM_GM_GEN4,
		0 },
		.channels = {
			{ nullptr, 0, 0, 0 },//{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ nullptr, 0, 0, 0 },//{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ nullptr, 0, 0, 0 },//{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ nullptr, 0, 0, 0 },//{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ nullptr, 0, 0, 0 },//{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, MAP_R)},
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/false,/*cam3*/ false, /*cam4*/false, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
		"inj1", "inj2", "inj3", "inj4",
		"inj5", "inj6", "inj7", "inj8",
		"main relay", "fan1",
		},
		.wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "hellen-e38",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_HELLEN_E38,
		0 },
		.channels = {
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, MAP_R)},
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		/* crank neg goes to https://rusefi.com/docs/pinouts/stim/?connector=main&pin=24C 2.5v source, crank positive 22B with a 4.7K pull up */
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/false,/*cam3*/ false, /*cam4*/false, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
		"inj1", "inj2", "inj3", "inj4",
		"inj5", "inj6", "inj7", "inj8",
		"main relay",
		"fan1",
 "Evap ctrl (Low side)",
 "AC Ctrl (low side)",
 "Skip Shift Sol (LOW side)",
 "J1-67 Starter Relay (LOW side)",
 "CEL (low Side)",
 "Evap purge sol (Low side)",
 "J1-52 Start Relay (High side)",
 "VVT",
 "FP1",
 		},
		.wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "SILVER-A",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_ALPHAX_SILVER_A, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, 1.05/*MAP_MPX6400_VALUE * ANALOG_L*/, MAP_MPX6400_VALUE * ANALOG_H },	// internal MAP
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		.eventExpected = {false, false, false, false, false, false, false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "A4 - Injector 1",
 "A3 - Injector 2",
 "A2 - Injector 3",
 "A1 - Injector 4",
"Fuel Pump",
"Idle Output",
"Fan Relay",
 "14A Tach Output",
 "13A Coil 1",
 "12A Coil 2",
 "11A Coil 3",
 "10A Coil 4",
		},
		.wboUnitsCount = 1,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "SILVER",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_ALPHAX_SILVER_B, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", 1.0f, 1.05/*MAP_MPX6400_VALUE * ANALOG_L*/, MAP_MPX6400_VALUE * ANALOG_H },	// internal MAP
			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		.eventExpected = {false, false, false, false, false, false, false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "A4 - Injector 1",
 "A3 - Injector 2",
 "A2 - Injector 3",
 "A1 - Injector 4",
"Fuel Pump",
"Idle Output",
"Fan Relay",
 "14A Tach Output",
 "13A Coil 1",
 "12A Coil 2",
 "11A Coil 3",
 "10A Coil 4",
		},
		.wboUnitsCount = 1,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "obd1",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_HONDA_OBD1_C, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },
		},
		.eventExpected = {/*crank*/true, false, /*cam1*/false,
		false, false, false, false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "A1 INJ_1",
 "A2 INJ_4",
 "A3 INJ_2",
 "A5 INJ_3",
 "Fuel Pump Relay",
 "A9 IAC",
 "A13 MIL",
 "A21 ICM Coil Control",
 "Radiator Fan Control Module",
 "A/C compressor clutch relay",
 "IAB intake manifold butterflies solenoid",
 "VTEC Solenoid Valve",
 		},
		.wboUnitsCount = 1,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	// https://github.com/rusefi/rusefi/wiki/Hellen-154-Hyundai
	{
		.boardName = "154HYUNDAI",
		.desiredEngineConfig = -1,
		.boardIds = { BOARD_ID_154HYUNDAI_C, BOARD_ID_154HYUNDAI_D, BOARD_ID_154HYUNDAI_E, 0 },
		.channels = {
			{ "TPS1_1", 1.0f, VOLT_7B * ANALOG_L, VOLT_7B * ANALOG_H },
			{ "TPS1_2", 1.0f, VOLT_9B * ANALOG_L, VOLT_9B * ANALOG_H },
			{ "PPS1", 1.0f, VOLT_10B * ANALOG_L, /*VOLT_10B * ANALOG_H 0.96750*/ 0.99},
			{ "PPS2", 1.0f, /*VOLT_11B * ANALOG_L*/0.94, VOLT_11B * ANALOG_H },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, MAP_R)},
//			{ "CLT", 1.0f, CLT_VALUE(ALPHA2CH_R) * ANALOG_L, CLT_VALUE(ALPHA2CH_R) * ANALOG_H }, // rev A-B-C
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H }, // rev D
			// 5B
//			{ "IAT", 1.0f, IAT_VALUE(ALPHA2CH_R) * ANALOG_L, IAT_VALUE(ALPHA2CH_R) * ANALOG_H }, // rev A-B-C
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H }, // rev D
			{ "BATT", 5.835, 9.0f, 15.0f },
		},
		// https://github.com/rusefi/hellen154hyundai/issues/120
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/true, /*cam3*/false, /*cam4*/false, /*vss*//*low priority since CANtrue*/false},
		.buttonExpected = {/*BrakePedal*//*+12v button true*/false, /*ClutchUp*//*+12v button true*/false, /*AcButton*//*+12v button true*/false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "INJ_1 k25",
 "INJ_2 k26",
 "INJ_3 k27",
 "INJ_4 k28",
 "VVT1", // 5
 "VVT2",
 "K47 BK1 Wastegate Solenoid",
 "Fan Relay Low",
 "Main Relay K64",
 "Fuel Pump K70", // 10
 "K87 AC Relay",
 "Second Fan Relay",
 "Coil 1",
 "Coil 2",
 "Coil 3", // 15
 "Coil 4",
 "MIL",
 "Tach",
 "X8 AuxLS1",
		},
		.wboUnitsCount = 0,
		.dcHackValue = 0,
		.highSizeStartingIndex = 0
	},
	{
		.boardName = "m74.9",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_M74_9, 0 },
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
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/false, /*cam3*/false, /*cam4*/false, /*vss*/true},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "Injector 1",
 "Injector 2",
 "Injector 3",
 "Injector 4",
 		},
		.wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSizeStartingIndex = 0
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

typedef void (*CanRequestSender) (int testLineIndex, bool value);

static int getCycleDurationMs() {
extern int boardId;
    if (boardId == STATIC_BOARD_ID_UAEFU121_SBC) {
        // real relay needs some time to click
        return 100;
    }

//    if (currentBoard!=nullptr)

//    .boardName
    return 2;
//    // huh? until some point this was working with only '2' ms?!
//    constexpr int cycleDurationMs = 50;
//    return cycleDurationMs;
}


static bool doTestEcuDigitalOutput2(size_t testLineIndex,
    size_t ecuLineIndex,
    bool isLowSide, CanRequestSender sender, bool expectation) {
	static DigitalResult result;
	memset(&result, 0, sizeof(result));

	setOutputAddrIndex(testLineIndex % DIGITAL_INPUT_BANK_SIZE);
	int bankIndex = testLineIndex / DIGITAL_INPUT_BANK_SIZE;

	bool isGood = true;

	for (int i = 0; i < cycleCount
	 //&& isGood
	 ; i++) {
		bool isSet = (i & 1) == 0;
		chprintf(chp, "               sending line=%d@%d value=%d\r\n", index2human(testLineIndex), i, isSet);
		// toggle the ECU pin for low side mode
		sender(ecuLineIndex, isSet ^ isLowSide);

        // at the moment we test both high-side and low-side in pull-up mode only
        // effectively we could have just used constant 1111b pullUpDownPinsBitmap
        // see also https://github.com/rusefi/rusefi-hardware/issues/252
		int pullUpDownPinsBitmap = isLowSide << bankIndex; // i % 2
		setScenarioIndex(pullUpDownPinsBitmap);
		chThdSleepMilliseconds(getCycleDurationMs());

		float voltage = getAdcValue(bankIndex);
		// low side sends roughly 2.8 but 5v high side is closer to 1v
		bool isHigh = voltage > 0.7;
		if (isHigh) {
			if (!result.haveSeenHigh) {
			    setCyanText();
				chprintf(chp, "                      ADC says HIGH %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
				setNormalText();
			}
			result.haveSeenHigh = true;
		} else {
			if (!result.haveSeenLow) {
			    setCyanText();
				chprintf(chp, "                      ADC says LOW %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
				setNormalText();
			}
			result.haveSeenLow = true;
		}

        bool actualResult = isHigh == isSet;
//		chprintf(chp, "actualResult=%d expectation=%d\r\n", actualResult, expectation);

		bool cycleIsGood = actualResult == expectation;
		if (!cycleIsGood) {
		    setErrorLedAndRedText();
			chprintf(chp, "ERROR! Line %d@%d FAILED! (set %d, received %d %1.3fv)\r\n",
				index2human(testLineIndex), i, (isSet ? 1 : 0), (isHigh ? 1 : 0), voltage);
			setNormalText();
		}
		isGood = isGood && cycleIsGood;
	}

	// test is successful if we saw state toggle
	return isGood;
}

static bool doTestEcuDigitalOutput(int testLineIndex, bool isLowSide, CanRequestSender sender, bool expectation) {
    return doTestEcuDigitalOutput2(testLineIndex, testLineIndex, isLowSide, sender, expectation);
}

bool testEcuDigitalOutput(int testLineIndex, bool isLowSide) {
    return testEcuDigitalOutput2(testLineIndex, testLineIndex, isLowSide);
}

bool testEcuDigitalOutput2(int testLineIndex, size_t ecuLineIndex, bool isLowSide) {
    return doTestEcuDigitalOutput2(testLineIndex, ecuLineIndex, isLowSide, [](int testLineIndex, bool value) {
		sendCanPinState(testLineIndex, value);
    }, true);
}

// lazy way to get value into lambda
static int globalDcIndex = 0;

bool testDcOutput(size_t dcIndex) {
    globalDcIndex = dcIndex;

	chprintf(chp, "sending DC %d\r\n", globalDcIndex);

    CanRequestSender sender = [](int testLineIndex, bool value) {
		sendCanDcState(globalDcIndex, value);
    };

    bool isGood = true;

    int temp = currentBoard->dcHackValue;

    isGood = isGood & doTestEcuDigitalOutput(LAST_DIGITAL_PIN - 2 * globalDcIndex, temp ^ /*isLowSide*/0, sender, false);
	isGood = isGood & doTestEcuDigitalOutput(LAST_DIGITAL_PIN - 2 * globalDcIndex, temp ^ /*isLowSide*/1, sender, true);
    isGood = isGood & doTestEcuDigitalOutput(LAST_DIGITAL_PIN - 2 * globalDcIndex - 1, temp ^ /*isLowSide*/0, sender, true);
	isGood = isGood & doTestEcuDigitalOutput(LAST_DIGITAL_PIN - 2 * globalDcIndex - 1, temp ^ /*isLowSide*/1, sender, false);

	return isGood;
}

size_t totalStepsNumber() {
    return getDigitalInputStepsCount()
    + getDigitalOutputStepsCount()
    + getDigitalDcOutputStepsCount()
    ;
}
