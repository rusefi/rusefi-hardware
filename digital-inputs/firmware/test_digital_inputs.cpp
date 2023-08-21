
#include "global.h"
#include "test_digital_inputs.h"
#include "chprintf.h"


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

bool testEcuDigitalInputs() {
	for (size_t idx = 0; idx < efi::size(stimOutputPins); idx++) {
		io_pin *pin = &stimOutputPins[idx];
		chprintf(chp, "Toggling port %d\r\n", pin->pin);
		bool bitState = true;
		for (int toggle_i = 0; toggle_i < 10; toggle_i++) {
			palWritePad(pin->port, pin->pin, bitState ? 1 : 0);
			bitState = !bitState;
			chThdSleepMilliseconds(100);
		}
		// turn the pin off for safety reasons
		palWritePad(pin->port, pin->pin, 0);
	}
	return true;
}
