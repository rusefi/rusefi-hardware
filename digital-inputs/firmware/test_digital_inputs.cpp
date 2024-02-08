
#include "global.h"
#include "test_digital_inputs.h"
#include "terminal_util.h"
#include "test_logic.h"

/**
 * inputs on ECU side (crank/cam/etc)
 */

// todo: reuse pin_repository in this project
static io_pin stimOutputPins[] = {
	{ GPIOE, 0 }, // DIG_0
	{ GPIOE, 7 },
	{ GPIOE, 8 },
	{ GPIOE, 9 },
	{ GPIOE, 10 },
	{ GPIOE, 11 },
	{ GPIOE, 12 },
	{ GPIOE, 13 },
};

extern BaseSequentialStream *chp;

void initStimDigitalOutputs() {
    for (size_t i = 0; i < efi::size(stimOutputPins); i++) {
        io_pin *pin = &stimOutputPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
       	palWritePad(pin->port, pin->pin, 0);
    }
}

size_t getDigitalInputStepsCount() {
    return efi::size(stimOutputPins);
}

void stimulateEcuDigitalInputs(size_t startStepIndex) {
	bool bitState = true;
	for (int toggle_i = 0; toggle_i < 10; toggle_i++) {

	    setGlobalStatusText();
		chprintf(chp, "%d/%d", toggle_i, totalStepsNumber());
		setNormalText();
		chprintf(chp, "      Toggling %d ports %d\r\n", getDigitalInputStepsCount(), bitState);

		chThdSleepMilliseconds(50);
        for (size_t idx = 0; idx < getDigitalInputStepsCount(); idx++) {
		    io_pin *pin = &stimOutputPins[idx];
			palWritePad(pin->port, pin->pin, bitState ? 1 : 0);
		}

		bitState = !bitState;
	}

    for (size_t idx = 0; idx < getDigitalInputStepsCount(); idx++) {
        io_pin *pin = &stimOutputPins[idx];
		// turn the pin off for safety reasons
		palWritePad(pin->port, pin->pin, 0);
	}
}
