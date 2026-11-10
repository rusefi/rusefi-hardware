#include "test_logic_config.h"
#include "board_id/boards_id.h"
#include "board_id/boards_dictionary.h"

/**
 * GM E78 (NXP MPC5566), rusefi-e78 firmware.
 *
 * outputNames must stay in the same order as the ECU's OUTPUTS[] in
 * rusefi-e78 firmware/hardware/board_hw_test.cpp
 * The ECU reports lowSideCount = 4 and a DC count of 0: the throttle H-bridge
 * drives the plate against its stops, which the DC test cannot observe.
 */
BoardConfig makeE78BoardConfig() {
	return {
		.boardName = "e78",
		// the board's own defaults already route crank, cam, TPS, CLT and IAT
		.desiredEngineConfig = -1,
		.boardIds = { STATIC_BOARD_ID_E78, 0 },
		.channels = {
			// TPS1/TPS2/PPS are on the eQADC with no extra divider
			// (analogInputDividerCoefficient = 1), but the E78's own input pull
			// network is not documented yet: unconnected TPS1/PPS1/PPS2 read ~0 V
			// and TPS2 ~5 V, so there is a pull-down on one group and a pull-up on
			// the other. Until those resistors are known the expected rig voltage
			// cannot be computed, so these stay untested rather than guessed.
			{ nullptr, 0, 0, 0 }, // TPS1_1, AN0 (falls with opening on this board)
			{ nullptr, 0, 0, 0 }, // TPS1_2, AN1 (rises with opening)
			{ nullptr, 0, 0, 0 }, // PPS1, AN5
			{ nullptr, 0, 0, 0 }, // PPS2, AN4
			{ nullptr, 0, 0, 0 }, // MAP: no MAP input is configured on this board
			{ nullptr, 0, 0, 0 }, // CLT: temporarily excluded; bench reads ~5 V, load/pull-up unverified
			// The provisional 2 kohm IAT pull-up predicts 1.67 V, consistent with
			// the observed 1.69 V. Verify the network before treating it as measured.
			{ "IAT", 1.0f, IAT_VALUE(E78_IAT_R) * ANALOG_L, IAT_VALUE(E78_IAT_R) * ANALOG_H },
			{ "BATT", E78_VBATT_MULT, 9.0f, 15.0f },

			{ nullptr, 0, 0, 0 }, // TPS2_1: single throttle on this board
			{ nullptr, 0, 0, 0 }, // TPS2_2
			{ nullptr, 0, 0, 0 }, // AUXL1
			{ nullptr, 0, 0, 0 }, // AUXL2
		},
		// crank is the 60-2 wheel on X2-56 (GPIO146); cam X on X2-57 (GPIO149) is
		// camInputs[0], so it reports as VVT1. Cam W on X2-60 (GPIO148) is
		// camInputs[1], reported as VVT2. Both require a fixture digital wire.
		// There is no VSS input.
		.eventExpected = {/*crank*/true, false, /*cam1*/true, /*cam2*/true, /*cam3*/false, /*cam4*/false, /*vss*/false},
		.buttonExpected = {false, false, false},
		.auxDigitalExpected = {false, false, false, false,
		false, false, false, false},
		.outputNames = {
 "X2-2 Injector 1",
 "X2-10 Exhaust cam phaser",
 "X2-11 Intake cam phaser",
 "X2-14 Thermostat heater",
 "X1-28 Fuel pump relay",
 "X1-27 Starter relay",
 "X2-1 Ignition 1",
		},
		.wboUnitsCount = 0,
		// no DC output is offered, so the DC polarity hack never applies
		.dcHackValue = 0,
		.highSideStartingIndex = 0, .wboStartIndex = 0,
	};
}
