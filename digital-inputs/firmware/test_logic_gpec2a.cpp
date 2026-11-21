#include "test_logic_config.h"
#include "board_id/boards_id.h"
#include "board_id/boards_dictionary.h"

/**
 * Chrysler/Continental GPEC2A (NXP MPC5674F), rusefi-gpec2a firmware.
 *
 * outputNames must stay in the same order as the ECU's OUTPUTS[] in
 * rusefi-gpec2a firmware/hardware/board_hw_test.cpp
 * The ECU reports three outputs, all low side, and a DC count of 0. Ordinal i
 * is checked on stim test pad i, so the harness wires injectors 1..3 to pads 1..3.
 */
BoardConfig makeGpec2aBoardConfig() {
	return {
		.boardName = "gpec2a",
		// the board's own defaults already route TPS, PPS, CLT and IAT
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_GPEC2A, 0 },
		.channels = {
			// TPS and PPS reach the eQADC with no extra divider
			// (analogInputDividerCoefficient = 1). The GPEC2A input pull network is
			// not documented, so these ranges assume the usual high-impedance input
			// on the standard stim pads. UNVERIFIED on the rig: adjust once measured.
			{ "TPS1_1", PULLED_DOWN_RANGE(DOWN_7B, UP_7B, 680'000) },   // C2-89, AN6
			{ "TPS1_2", PULLED_DOWN_RANGE(DOWN_9B, UP_9B, 680'000) },   // C2-90, AN5
			{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },   // C1-92, AN37
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },   // C1-91, AN11
			{ nullptr, 0, 0, 0 }, // MAP: no MAP input is configured on this board
			// Thermistor pull-ups are the provisional firmware values, not measured.
			{ "CLT", 1.0f, CLT_VALUE(GPEC2A_CLT_R) * ANALOG_L, CLT_VALUE(GPEC2A_CLT_R) * ANALOG_H }, // C2-12, AN35
			{ "IAT", 1.0f, IAT_VALUE(GPEC2A_IAT_R) * ANALOG_L, IAT_VALUE(GPEC2A_IAT_R) * ANALOG_H }, // C2-3, AN32 (channel unconfirmed)
			{ nullptr, 0, 0, 0 }, // BATT: divider not calibrated yet

			{ nullptr, 0, 0, 0 }, // TPS2_1: single throttle on this board
			{ nullptr, 0, 0, 0 }, // TPS2_2
			{ nullptr, 0, 0, 0 }, // AUXL1
			{ nullptr, 0, 0, 0 }, // AUXL2
		},
		// no digital input is part of this QC configuration yet
		.eventExpected = {/*crank*/false, false, /*cam1*/false, /*cam2*/false, /*cam3*/false, /*cam4*/false, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "C2-8 Injector 1",
 "C2-9 Injector 2",
 "C2-10 Injector 3",
		},
		.wboUnitsCount = 0,
		// no DC output is offered, so the DC polarity hack never applies
		.dcHackValue = 0,
		.highSideStartingIndex = 0, .wboStartIndex = 0,
	};
}
