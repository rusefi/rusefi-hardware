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

			// 24
			// on-module digital barometer
			BARO_CHANNEL,
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


BoardConfig makeHarley54BoardConfig() {
	return {
		.boardName = "hd54",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_HARLEY54, 0 },
		.channels = {
			// Analog expectations assume the same stim signals as the HD81 harness.
			// 0
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.920940928 },
			{ "PPS2", 1, 0.97, 1.13 /*PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000)*/ },
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) },
			// The default 2021 tune publishes head temperature as CLT.
			{ nullptr, 0, 0, 0 }, //{ "CLT", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },
			{ "IAT", 1.0f, IAT_VALUE(HELLEN_R) * ANALOG_L, IAT_VALUE(HELLEN_R) * ANALOG_H },
			{ "BATT", HELLEN_VBATT_MULT, 9.0f, 15.0f },

			// 8
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },

			// 16
			{ nullptr, 0, 0, 0 }, // AuxAnalog1: no analog gear sensor
			{ nullptr, 0, 0, 0 }, // { "AUX2", PULLED_UP_RANGE(DOWN_13B, UP_13B, 1'000) },
			{ nullptr, 0, 0, 0 }, // AuxAnalog3: no analog JSS sensor
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			{ nullptr, 0, 0, 0 },
			// The default 2021 tune publishes coolant temperature as AuxAnalog8.
			{ nullptr, 0, 0, 0 }, //{ "Coolant", 1.0f, CLT_VALUE(HELLEN_R) * ANALOG_L, CLT_VALUE(HELLEN_R) * ANALOG_H },

			// 24
			// on-module digital barometer
			BARO_CHANNEL,
		},
		// HD54 has no cam input.
		.eventExpected = {/*crank*/true, false, /*cam1*/false, false, false, false, /*vss*/false},
		.buttonExpected = {false, false, false},
		// JSS is digital and is not published through the ECU's auxiliary digital QC counters.
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		// Must match fw-iws/firmware/hardware/board_hw_test.cpp OUTPUTS order,
		// which differs from the generated connector output list.
		.outputNames = {
			"J4.5 Front Injector 2",
			"J4.6 Rear Injector 1",
			"J2.4 ACR",
			"J2.1 Chassis Fan",
			"J2.3 Coolant Pump",
			"J4.2 Front Coil 2",
			"J4.1 Rear Coil 1",
			"J1.1 Left Cooling Fan",
			"J1.5 Right Cooling Fan",
		},
		.wboUnitsCount = 2,
		.dcHackValue = 1,
		.highSideStartingIndex = 0, .wboStartIndex = 0,
	};
}

/**
 * Minimal Harley (GPEC2A running the HARLEY engine type, which reports this
 * board id): TPS1/2, PPS1/2, MAP, two injectors, two ACR solenoids and two coils.
 *
 * outputNames must stay in the same order as the ECU's HARLEY_OUTPUTS[] in
 * rusefi-gpec2a firmware/hardware/board_hw_test.cpp: a Harley is a 2-cylinder,
 * so injector outputs 3 and 4 re repurposed as ACR1/ACR2.
 */
// The hd81/Harley harness pairs the two injectors (and the two ACRs) swapped
// relative to the ECU's sequential cavity order: hd81 pins read "6 OUT FRONT
// INJ 2" before "7 OUT REAR INJ 1", and the Proteus "hd" config likewise lists
// Injector 2 before Injector 1. ECU-ordinal -> stim-pad verified live by
// PAD-DISCOVERY on this GPEC2A bench (2026-11-27): Inj1(ord0)->line1,
// Inj2(ord1)->line0, ACR1(ord2)->line3, ACR2(ord3)->line2, and with coils
// wired Coil1(ord4)->line6, Coil2(ord5)->line7 (hd81 coils land on the next
// pad bank, not pads 5/6). Values are 0-based stim line indices (= pad addr in
// bank 0); the console prints pads 1-based.
static constexpr uint8_t MINIMAL_HARLEY_OUTPUT_TEST_LINE_MAP[] = {1, 0, 3, 2, 6, 7};

BoardConfig makeMinimalHarleyBoardConfig() {
	return {
		.boardName = "hd-min",
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_MINIMAL_HARLEY, 0 },
		.channels = {
			// Analog expectations assume the same stim signals as the HD81 harness.
			// 0
			// Bands recalibrated to the live GPEC2A bench readings (2026-11-27,
			// direct hd81->C-pin jumpers through the superseal adapter). The pad
			// model (PULLED_DOWN_RANGE) assumes a stiff source; the real ECU inputs
			// have open-detect bias networks (C2-89/TPS1_1 pulls up to 5V, reads
			// high; C2-90/TPS1_2 pulls down) so the measured voltage is tugged off
			// the nominal pad through the adapter/superseal series R. These explicit
			// bands bracket the measured values with ~1-2 LSB (8-bit CAN, ~20mV/LSB)
			// margin.
			{ "TPS1_1", 1, 0.55, 0.72 },  // 7B 0.5V pad, biased up -> live 0.627-0.647 V (C2-89)
			{ "TPS1_2", 1, 0.79, 0.90 },  // 9B 0.8V pad -> live 0.863 V (C2-90)
			// PPS1 10B 0.9V pad -> live 0.928 V (C1-92); 0.95 = pad +~5.5%.
			{ "PPS1", 1, 0.79 /*0.821830144*/, 0.95 },
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },  // 11B 1.1V -> live 1.078 V (C1-91), in range
			{ "MAP", 1, 0.60, 0.72 },     // 8B pad -> live 0.686 V (C2-45)
			// CLT, IAT, BATT and everything else: not checked
		},
		// crank wired to GPEC2A; cam is NOT checked — a Harley uses MAP
		// V-twin VVT (no cam sensor) and cam is not wired on this bench.
		.eventExpected = {/*crank*/true, false, /*cam1*/false, false, false, false, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false, false, false, false, false},
		.outputNames = {
			"Injector 1",
			"Injector 2",
			"ACR1",
			"ACR2",
			"Coil 1",
			"Coil 2",
		},
		.wboUnitsCount = 0,
		.dcHackValue = 1,
		.highSideStartingIndex = 0,
		.wboStartIndex = 0,
		.outputTestLineMap = MINIMAL_HARLEY_OUTPUT_TEST_LINE_MAP,
		.outputTestLineMapCount = sizeof(MINIMAL_HARLEY_OUTPUT_TEST_LINE_MAP) / sizeof(MINIMAL_HARLEY_OUTPUT_TEST_LINE_MAP[0]),
	};
}
