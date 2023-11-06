
#include "global.h"
#include "test_digital_outputs.h"
#include "test_logic.h"
#include "can.h"
#include "terminal_util.h"

/**
 * ECU outputs
 */

#define XOR_MAGIC 1

static io_pin addrPins[] = {
{GPIOC, 8}, // ADR0
{GPIOC, 6},
{GPIOC, 5},
{GPIOA, 6},
};

static io_pin pullUpDownPins[] = {
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

    for (size_t i = 0;i < efi::size(pullUpDownPins);i++) {
        io_pin *pin = &pullUpDownPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
    }

    palSetPadMode(muxOff.port, muxOff.pin, PAL_MODE_OUTPUT_PUSHPULL);
    palWritePad(muxOff.port, muxOff.pin, 0 ^ XOR_MAGIC);

	// send the request early
	setOutputCountRequest();
}

extern bool globalEverythingHappy;

static void waitForMetaInfo() {
	int timeout = 0;

#define SLEEP_CHUNK 100
	while (getDigitalOutputStepsCount() <= 0 && currentBoard == nullptr && timeout < (5000 / SLEEP_CHUNK)) {
	    chThdSleepMilliseconds(SLEEP_CHUNK);
	    timeout ++;
	}
}

bool testEcuDcOutputs(size_t startStepIndex) {
    waitForMetaInfo();

	bool isGood = true;

	int dcOutputs = getDigitalDcOutputStepsCount();
	for (size_t dcIndex = 0; dcIndex < dcOutputs; dcIndex++) {
	    bool dcResult = testDcOutput(dcIndex);

        if (dcResult) {
            setGlobalStatusText();
            chprintf(chp, "GOOD DC=%d\n", dcIndex);
            setNormalText();
        }

	    isGood = isGood & dcResult;
	}
	return isGood;
}

bool testEcuDigitalOutputs(size_t startStepIndex) {
    waitForMetaInfo();

	bool isGood = true;

	int numOutputs = getDigitalOutputStepsCount();
	int lowSideOutputs = getLowSideOutputCount();

	chprintf(chp, "                      numOutputs %d\r\n", numOutputs);

	// wait for "output meta info" CAN packet
	if (numOutputs <= 0)
		return false;

	for (size_t currentIndex = 0; currentIndex < numOutputs; currentIndex++) {
		bool isThisGood = testEcuDigitalOutput(currentIndex, currentIndex < lowSideOutputs);
		if (isThisGood) {
		    setGlobalStatusText();
			chprintf(chp, "%d/%d %s ",
			    startStepIndex + currentIndex,
			    totalStepsNumber(),
			    currentBoard == nullptr ? nullptr : currentBoard->getOutputName(currentIndex)
			    );
			setNormalText();
			chprintf(chp, "GOOD channel %d\r\n",
			index2human(currentIndex));
		} else {
		    setErrorLedAndRedText();
		    globalEverythingHappy = false;
			chprintf(chp, "!!!!!!! BAD channel %d %s !!!!!!!!!!!!!!!\r\n",
			index2human(currentIndex),
			currentBoard == nullptr ? nullptr : currentBoard->getOutputName(currentIndex));
			setNormalText();
			chThdSleepMilliseconds(1000);
		}
		isGood = isGood && isThisGood;
	}
	return isGood;
}

void setOutputAddrIndex(int index) {
    int param = index;
    chprintf(chp, "               Setting ADDR %d\r\n", param);
    for (size_t i = 0;i<efi::size(addrPins);i++) {
        int bitState = (index & 1) ^ XOR_MAGIC;
        index = index / 2;
        io_pin *pin = &addrPins[i];
//        chprintf(chp, "ADDR %d: bit=%d %d\n", param, i, bitState);
        palWritePad(pin->port, pin->pin, bitState);
    }
}

void setScenarioIndex(int pullUpDownPinsBitmap) {
    for (size_t i = 0;i<efi::size(pullUpDownPins);i++) {
        int bitState = pullUpDownPinsBitmap & 1;
        // please note lack of XOR
        pullUpDownPinsBitmap = pullUpDownPinsBitmap / 2;
        io_pin *pin = &pullUpDownPins[i];
        palWritePad(pin->port, pin->pin, bitState);
    }
}