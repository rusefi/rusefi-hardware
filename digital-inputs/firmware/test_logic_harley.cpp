#include "test_logic_config.h"
#include "board_id/boards_id.h"
#include "board_id/boards_dictionary.h"

BoardConfig makeHarleyBoardConfig() {
	return {
		.boardName = "hd",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_PROTEUS_HARLEY, STATIC_BOARD_ID_HELLEN_HD, BOARD_ID_HD81_A, BOARD_ID_HD81_B, 0 },
		.channels = {
			// 0
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
//			{ "PPS1",   PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },
//			{ "MAP", 1, 0.62, 0.670109504 },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			// 8
			{ nullptr, 0, 0, 0 }, // { "TPS2_1", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 }, // { "TPS2_2", 1.0f, 0.5f * ANALOG_L, 0.5f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL1", 1.0f, 1.35f * ANALOG_L, 1.35f * ANALOG_H },
			{ nullptr, 0, 0, 0 },//{ "AUXL2", 1.0f, 2.23f * ANALOG_L, 2.23f * ANALOG_H },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },

			// 16
			// 78 - With on-board 680K pull down
			{ "AUX1", PULLED_DOWN_RANGE(DOWN_12B, UP_12B, 680'000) },
			// 32 - With on-board 1K pull up
			//{ "AUX2", PULLED_UP_RANGE(DOWN_13B, UP_13B, 1'000) },
			{
				"AUX2",
				1.0f,
				0.937f * PULLED_UP_VOLTAGE(DOWN_13B, UP_13B, 1'000),
				1.05f * PULLED_UP_VOLTAGE(DOWN_13B, UP_13B, 1'000),
			},
			// 34 - With on-board 4.7K pull up
			//{ "AUX3", PULLED_UP_RANGE(DOWN_14B, UP_14B, 4'700) },
			/* this input after buffer OpAmp and 0.5 divider is routed to two STM32 inputs
			 * One is ADC input that is checked here,
			 * Another one is digital input that have pull-down enabled by default.
			 * This pull-down affects accuracy, so we extend low threshold here */
			{
				"AUX3",
				1.0f,
				0.87f * PULLED_UP_VOLTAGE(DOWN_14B, UP_14B, 4'700),
				1.05f * PULLED_UP_VOLTAGE(DOWN_14B, UP_14B, 4'700),

			},
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			// 55 - with 4.7K pull up. Secondary 2K on 22C
			{ "AT1", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },

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
		.highSideStartingIndex = 0, .wboStartIndex = 0,
	};
}
