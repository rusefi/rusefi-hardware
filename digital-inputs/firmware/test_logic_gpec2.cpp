#include "test_logic_config.h"
#include "board_id/boards_id.h"
#include "board_id/boards_dictionary.h"

/**
 * Chrysler/Continental GPEC2 (NXP MPC5566), rusefi-gpec2 firmware.
 *
 * outputNames must stay in the same order as the ECU's OUTPUTS[] in
 * rusefi-gpec2 firmware/hardware/board_hw_test.cpp
 * The ECU reports six outputs, all low side, and a DC count of 1. Ordinal i
 * is checked on stim test pad i, so the harness wires injectors 1..6 to pads
 * 1..6. DC 0 is the throttle H-bridge: its two motor terminals (C2-3, C2-4)
 * go to the last two digital pads, as on every other board with a DC output.
 */
BoardConfig makeGpec2BoardConfig() {
	return {
		.boardName = "gpec2",
		// the board's own defaults already route PPS, CLT and IAT
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_GPEC2, 0 },
		.channels = {
			// The throttle body is not part of the QC harness.
			{ nullptr, 0, 0, 0 }, // TPS1_1, AN27
			{ nullptr, 0, 0, 0 }, // TPS1_2, AN25
			// PPS reaches the eQADC with no extra divider
			// (analogInputDividerCoefficient = 1). The GPEC2 input pull network is
			// not documented, so these ranges assume the usual high-impedance input
			// on the standard stim pads. UNVERIFIED on the rig: adjust once measured.
			{ "PPS1", PULLED_DOWN_RANGE(DOWN_10B, UP_10B, 680'000) },   // AN11
			{ "PPS2", PULLED_DOWN_RANGE(DOWN_11B, UP_11B, 680'000) },   // AN37
			{ nullptr, 0, 0, 0 }, // MAP: not part of this QC configuration
			// Thermistor pull-ups are the provisional firmware values, not measured.
			{ "CLT", 1.0f, CLT_VALUE(GPEC2_CLT_R) * ANALOG_L, CLT_VALUE(GPEC2_CLT_R) * ANALOG_H }, // C2-12, AN6
			{ "IAT", 1.0f, IAT_VALUE(GPEC2_IAT_R) * ANALOG_L, IAT_VALUE(GPEC2_IAT_R) * ANALOG_H }, // C2-17, AN7
			{ nullptr, 0, 0, 0 }, // BATT: not part of this QC configuration

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
		// Injector 1/2 cavities are not identified on the bench yet.
		.outputNames = {
 "Injector 1",
 "Injector 2",
 "C2-10 Injector 3",
 "C2-11 Injector 4",
 "C2-79 Injector 5",
 "C2-80 Injector 6",
		},
		.wboUnitsCount = 0,
		// DC polarity is empirical and UNVERIFIED on the rig: if both directions
		// read inverted, swap the two motor leads or set this to 1.
		.dcHackValue = 0,
		.highSideStartingIndex = 0, .wboStartIndex = 0,
	};
}
