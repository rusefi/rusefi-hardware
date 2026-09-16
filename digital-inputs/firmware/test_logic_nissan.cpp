#include "test_logic_config.h"
#include "board_id/boards_id.h"
#include "board_id/boards_dictionary.h"

BoardConfig makeNissan121BoardConfig() {
	return {
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
		.highSideStartingIndex = 0, .wboStartIndex = 0,
	};
}

BoardConfig makeNissanZ31BoardConfig() {
	return {
		// Nissan Z31 300ZX (VG30E/ET) plug-n-play, mega-uaefi module + mcu100-f7
		.boardName = "nissan-z31",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_UAEFI_Z31, 0 },
		.channels = {
 			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ nullptr, 0, 0, 0 }, // TPS1_2
			{ nullptr, 0, 0, 0 }, // PPS1
			{ nullptr, 0, 0, 0 }, // PPS2
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
 			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // "TPS2_1"
			{ nullptr, 0, 0, 0 }, // "TPS2_2"
			{ nullptr, 0, 0, 0 }, // { "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
		},
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/false, false, false, /*vss*/true},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "101 INJ_1",
 "102 INJ_2",
 "103 INJ_3",
 "104 INJ_4",
 "105 INJ_5",
 "106 INJ_6",
"2 - Idle Air Valve",
"20 - Fuel Pump Relay",
"6 - Main Relay",
"J1.8 - Low Side 1",
"J1.7 - Low Side 3",
"5 - Ignition",
"J1.9 - Ignition Aux 3",
"J1.17 - Ignition Aux 4",
"J1.25 - Ignition Aux 5",
        },
        .wboUnitsCount = 1,
		.dcHackValue = 1,
		.highSideStartingIndex = 0, .wboStartIndex = 0,
	};
}
