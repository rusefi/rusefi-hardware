
#include "global.h"
#include "test_digital_outputs.h"
#include "test_logic.h"
#include "can.h"
#include "chprintf.h"

#define XOR_MAGIC 1

static io_pin addrPins[] = {
{GPIOC, 8}, // ADR0
{GPIOC, 6},
{GPIOC, 5},
{GPIOA, 6},
};

static io_pin scenarioPins[] = {
{GPIOB, 12}, // OUT0 - this controls pull-up/pull-down on pads  #1-16
{GPIOB, 11}, //                                                 17-32
{GPIOC, 7},
{GPIOC, 4},  // only test 49
};

static io_pin muxOff = {GPIOA, 7};

extern BaseSequentialStream *chp;

void initStimDigitalInputs() {
    for (size_t i = 0;i < efi::size(addrPins);i++) {
        io_pin *pin = &addrPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
    }

    for (size_t i = 0;i < efi::size(scenarioPins);i++) {
        io_pin *pin = &scenarioPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
    }

    palSetPadMode(muxOff.port, muxOff.pin, PAL_MODE_OUTPUT_PUSHPULL);
    palWritePad(muxOff.port, muxOff.pin, 0 ^ XOR_MAGIC);

	// send the request early
	setOutputCountRequest();
}

bool testEcuDigitalOutputs() {
	bool isGood = true;
	int numOutputs = getOutputCount();
	
	// wait for "output meta info" CAN packet
	if (numOutputs < 0)
		return false;

	for (int currentIndex = 0; currentIndex < numOutputs; currentIndex++) {
		bool isThisGood = testEcuDigitalOutput(currentIndex);
		if (isThisGood) {
			chprintf(chp, "GOOD channel %d\r\n", index2human(currentIndex));
		} else {
			chprintf(chp, "!!!!!!!! BAD channel %d !!!!!!!!!!!!!!!\r\n", index2human(currentIndex));
		}
		isGood = isGood && isThisGood;
	}
	return isGood;
}

void setOutputAddrIndex(int index) {
    int param = index;
    chprintf(chp, "Setting ADDR %d\r\n", param);
    for (size_t i = 0;i<efi::size(addrPins);i++) {
        int bitState = (index & 1) ^ XOR_MAGIC;
        index = index / 2;
        io_pin *pin = &addrPins[i];
//        chprintf(chp, "ADDR %d: bit=%d %d\n", param, i, bitState);
        palWritePad(pin->port, pin->pin, bitState);
    }
}

void setScenarioIndex(int index) {
    for (size_t i = 0;i<efi::size(scenarioPins);i++) {
        int bitState = index & 1;
        // please note lack of XOR
        index = index / 2;
        io_pin *pin = &scenarioPins[i];
        palWritePad(pin->port, pin->pin, bitState);
    }
}