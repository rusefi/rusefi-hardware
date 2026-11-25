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
			// MAP (AN19) now wired: standard high-impedance pressure input on the
			// stim 8B pads, same config as the harley/nissan MAP channel.
			{ "MAP", PULLED_DOWN_RANGE(DOWN_8B, UP_8B, 680'000) }, // AN19
			// CLT (AN6, via stim pin 6) and IAT (AN7) are wired and confirmed on
			// the correct channels on 2026-11-24 (pulling stim pin 6 railed AN6).
			// The CLT_VALUE/IAT_VALUE theoretical math does NOT apply here: its
			// pull-up (5230/2000) and stim-load (2000/1000) constants were
			// inherited from GPEC2A and do not match this GPEC2 rig's resistors.
			// The raw divider voltages are stable, so ranges are calibrated to the
			// measured values (+/-7.5% ANALOG band). Temperature *accuracy* still
			// needs the true physical pull-up (resistance-substitution test) set
			// as bias_resistor in board_configuration.cpp.
			{ "CLT", 1.0f, 1.765f * ANALOG_L, 1.765f * ANALOG_H }, // C2-12 AN6, stim pin 6, measured 1.765 V
			{ "IAT", 1.0f, 1.000f * ANALOG_L, 1.000f * ANALOG_H }, // C2-17 AN7, measured 1.000 V
			{ nullptr, 0, 0, 0 }, // BATT: not part of this QC configuration

			{ nullptr, 0, 0, 0 }, // TPS2_1: single throttle on this board
			{ nullptr, 0, 0, 0 }, // TPS2_2
			{ nullptr, 0, 0, 0 }, // AUXL1
			{ nullptr, 0, 0, 0 }, // AUXL2
		},
		// no digital input is part of this QC configuration yet
		// CKP C2-86/GPIO113 (IRQ7) is wired to the stim; the ECU reports its
		// primary-trigger edge count in EVENT_COUNTERS msg[0]. CMP C2-85/GPIO115
		// is connected physically but NOT yet testable (no SIU IRQ, eTPU-A
		// capture not implemented), so cam1 stays false for now.
		.eventExpected = {/*crank*/true, false, /*cam1*/false, /*cam2*/false, /*cam3*/false, /*cam4*/false, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		// Injector 1/2 cavities are not identified on the bench yet.
		// Ordinals 6..9 are the four VVT solenoid outputs, in the same order as
		// the ECU OUTPUTS[] in rusefi-gpec2 board_hw_test.cpp (vvtPins order:
		// bank1 intake, bank1 exhaust, bank2 intake, bank2 exhaust). These are
		// HIGH-SIDE (battery-sourcing) drivers, not low-side sinks: the ECU's
		// boardGetMetaLowSideOutputsCount() returns 6, so the tester tests
		// ordinals 6..9 with a pull-DOWN pad (isLowSide=0), expecting the pad to
		// be driven UP on assertion. highSideStartingIndex = 6 maps ordinal 6->
		// test pad index 6 (human pad 7), so wire stim pads 7..10 to
		// C2-23 / C2-22 / C2-94 / C2-95 respectively.
		.outputNames = {
 "Injector 1",
 "Injector 2",
 "C2-10 Injector 3",
 "C2-11 Injector 4",
 "C2-79 Injector 5",
 "C2-80 Injector 6",
 "VVT1 C2-23 b1 intake",
 "VVT2 C2-22 b1 exhaust",
 "VVT3 C2-94 b2 intake",
 "VVT4 C2-95 b2 exhaust",
		},
		.wboUnitsCount = 0,
		// DC validation offset: with 10 outputs (6 low-side + 4 high-side) ahead of the DC test,
		// the GPEC2 rig needs dcHackValue = 1 (the common value across boards);
		// 0 failed every DC sub-phase on the bench 2026-11-24.
		.dcHackValue = 1,
		.highSideStartingIndex = 6 /* human index 7: VVT1..4 on pads 7..10 */, .wboStartIndex = 0,
	};
}
