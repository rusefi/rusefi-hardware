
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
#if 0
{GPIOB, 12}, // OUT0 - this controls pull-up/pull-down on pads  #1-16 rev 0.2
#else
{GPIOD, 8}, // OUT0 - this controls pull-up/pull-down on pads  #1-16 rev C
#endif
{GPIOB, 11}, //                                                 17-32
{GPIOC, 7},
{GPIOC, 4},  // only test 49
};

static io_pin muxOff = {GPIOA, 7};

extern BaseSequentialStream *chp;
extern int totalErrorsCounter;
extern bool globalEverythingHappy;

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
}

static void waitForMetaInfo() {
	int timeout = 0;

#define SLEEP_CHUNK 100
	while (getDigitalOutputStepsCount() <= 0 && currentBoard == nullptr && timeout < (5000 / SLEEP_CHUNK)) {
	    chThdSleepMilliseconds(SLEEP_CHUNK);
	    timeout ++;
	}
}

bool testEcuDcOutputs(size_t overallProcessingStepIndex) {
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

bool testEcuDigitalOutputs(size_t overallProcessingStepIndex) {
    waitForMetaInfo();

	bool isGood = true;

	int numOutputs = getDigitalOutputStepsCount();
	int lowSideOutputs = getLowSideOutputCount();

	chprintf(chp, "                      numOutputs %d\r\n", numOutputs);

	// wait for "output meta info" CAN packet
	if (numOutputs <= 0)
		return false;

	for (size_t currentIndex = 0; currentIndex < numOutputs; currentIndex++) {
	    bool isLowSideOutput = currentIndex < lowSideOutputs;
	    size_t testLineIndex;
	    if (currentBoard->highSizeStartingIndex != 0 && !isLowSideOutput) {
	        // sometimes high sides go after low sides, sometimes there is an explicit start index for high side
	        size_t highSideIndex = currentIndex - lowSideOutputs;
	        testLineIndex = currentBoard->highSizeStartingIndex + highSideIndex;
	    } else {
	        testLineIndex = currentIndex;
	    }

		bool isThisGood = testEcuDigitalOutput(testLineIndex, isLowSideOutput);
		if (isThisGood) {
		    setGlobalStatusText();
			chprintf(chp, "%d/%d %s ",
			    overallProcessingStepIndex + currentIndex,
			    totalStepsNumber(),
			    currentBoard == nullptr ? nullptr : currentBoard->getOutputName(currentIndex)
			    );
			setNormalText();
			chprintf(chp, "GOOD channel %d\r\n",
			index2human(currentIndex));
		} else {
		    setErrorLedAndRedText();
		    globalEverythingHappy = false;
            totalErrorsCounter++;
			chprintf(chp, "!!!!!!! BAD channel %d %s !!!!!!!!!!!!!!!\r\n",
			    index2human(currentIndex),
			    currentBoard == nullptr ? nullptr : currentBoard->getOutputName(currentIndex)
			    );
			setNormalText();
			chThdSleepMilliseconds(1000);
		}
		isGood = isGood && isThisGood;
	}
	return isGood;
}

void setOutputAddrIndex(int index) {
    int param = index;
    chprintf(chp, "     Selecting ADDR %d\r\n", param);
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