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
			// MAP C2-45 (map.sensor.hwChannel). Measured-provisional: a stim pad
			// reads ~0.706 V raw on the 2026-11-26 bench; model/divider TBD.
			{ "MAP", 1.0f, 0.65f, 0.76f },
			// CLT (C2-12 AN35) and IAT (C2-44, firmware setGpec2aThermistor, not
			// C2-3) are wired. The CLT_VALUE/IAT_VALUE theoretical math does NOT
			// apply on this stim rig: the stim analog stimulus is an op-amp-
			// buffered voltage source (OPA4197 U1, github.com/rusefi/stim) -- a
			// stiff source, not the resistive thermistor emulation those macros
			// model -- so no pull-up/stim-load constant brackets the reading
			// (that is why IAT false-failed at 3250: modeled 1.18 V vs live
			// 1.06 V). Ranges are calibrated to the measured raw divider voltages
			// (+/-7.5% ANALOG band), same approach as makeGpec2BoardConfig.
			// Temperature *accuracy* still needs the true physical pull-up
			// (firmware uses 3250; resistance-substitution test) as bias_resistor
			// in rusefi-gpec2a board_configuration.cpp.
			{ "CLT", 1.0f, 1.804f * ANALOG_L, 1.804f * ANALOG_H }, // C2-12 AN35, measured 1.804 V (2026-11-26)
			{ "IAT", 1.0f, 1.059f * ANALOG_L, 1.059f * ANALOG_H }, // C2-44, measured 1.059 V (2026-11-26)
			// VBatt C2 supply via ADC13; mult matches firmware vbattDividerCoeff.
			// Standard 9-15 V window: use a normal (~13.5 V) bench supply for this
			// check -- >15 V fails the window and >~16.75 V rails the 5 V ADC.
			{ "BATT", GPEC2A_VBATT_MULT, 9.0f, 15.0f },

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
