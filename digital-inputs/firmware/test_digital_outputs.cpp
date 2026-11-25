
#include "global.h"
#include "test_digital_outputs.h"
#include "test_logic.h"
#include "can.h"
#include "terminal_util.h"
#include "adc.h"

// Diagnostic: when an ECU output tests BAD, drive that ECU ordinal low vs high
// and sweep every mux pad to find which stim pad (if any) actually responds.
// Distinguishes "wired to a different stim pad" (fixable with outputTestLineMap)
// from "not landed at all" (bench wiring). Prints the responding pad(s).
static void discoverOutputPad(size_t ecuLineIndex, const char *name);

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
	// we need both the IO_META_INFO packet and the BOARD_STATUS packet: they arrive independently
	while ((getDigitalOutputStepsCount() <= 0 || currentBoard == nullptr) && timeout < (5000 / SLEEP_CHUNK)) {
	    chThdSleepMilliseconds(SLEEP_CHUNK);
	    timeout ++;
	}
}

// dereferencing null currentBoard does not fault on STM32 (address 0 aliases flash), it silently
// reads garbage - e.g. garbage highSideStartingIndex once sent a wild index into getAdcValue()
// which hard faulted and froze the whole tester
static bool isBoardDetected() {
	if (currentBoard != nullptr) {
	    return true;
	}
	setErrorLedAndRedText();
	chprintf(chp, "                      BOARD NOT DETECTED no BOARD_STATUS, cannot test outputs\r\n");
	setNormalText();
	return false;
}

bool testEcuDcOutputs(size_t overallProcessingStepIndex) {
    waitForMetaInfo();
    if (!isBoardDetected()) {
        return false;
    }

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
    if (!isBoardDetected()) {
        return false;
    }

	bool isGood = true;

	int numOutputs = getDigitalOutputStepsCount();
	int lowSideOutputs = getLowSideOutputCount();


	// wait for "output meta info" CAN packet
	if (numOutputs <= 0) {
		    setErrorLedAndRedText();
	        chprintf(chp, "                      ZERO OUTPUTS CONFIGURED\r\n");
		return false;
    }
	if (currentBoard->outputTestLineMap != nullptr &&
	    (currentBoard->outputTestLineMapCount != static_cast<size_t>(numOutputs) ||
	     (currentBoard->outputTestLowSideCount >= 0 &&
	      currentBoard->outputTestLowSideCount != lowSideOutputs))) {
	    setErrorLedAndRedText();
	    chprintf(chp, "ERROR! Output test-pad map does not match ECU output metadata\r\n");
	    return false;
	}

	chprintf(chp, "                      numOutputs %d\r\n", numOutputs);


	for (size_t currentIndex = 0; currentIndex < numOutputs; currentIndex++) {
	    bool isLowSideOutput = currentIndex < lowSideOutputs;
//	    chprintf(chp, "temp %d/%d %d\r\n", currentIndex, lowSideOutputs, isLowSideOutput);
	    size_t testLineIndex;
	    if (currentBoard->highSideStartingIndex != 0 && !isLowSideOutput) {
	        // sometimes high sides go after low sides, sometimes there is an explicit start index for high side
	        size_t highSideIndex = currentIndex - lowSideOutputs;
	        testLineIndex = currentBoard->highSideStartingIndex + highSideIndex;
//chprintf(chp, "highSideIndex %d/%d\r\n", highSideIndex, testLineIndex);

	    } else {
	        testLineIndex = currentIndex;
	    }
	    if (currentBoard->outputTestLineMap != nullptr) {
	        testLineIndex = currentBoard->outputTestLineMap[currentIndex];
	    }

		bool isThisGood = testEcuDigitalOutput2(testLineIndex, currentIndex, isLowSideOutput);
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
			discoverOutputPad(currentIndex,
			    currentBoard == nullptr ? "" : currentBoard->getOutputName(currentIndex));
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

#define DISCOVERY_PAD_COUNT (DIGITAL_INPUT_BANK_SIZE * ADC_GRP_NUM_CHANNELS)

static void discoverOutputPad(size_t ecuLineIndex, const char *name) {
    float lo[DISCOVERY_PAD_COUNT];
    float hi[DISCOVERY_PAD_COUNT];
    setScenarioIndex(0x0F); // pull-ups on for all banks so an idle pad floats high
    for (int drive = 0; drive < 2; drive++) {
        sendCanPinState((uint8_t)ecuLineIndex, drive != 0);
        chThdSleepMilliseconds(80);
        for (int bank = 0; bank < ADC_GRP_NUM_CHANNELS; bank++) {
            for (int addr = 0; addr < DIGITAL_INPUT_BANK_SIZE; addr++) {
                setOutputAddrIndex(addr);
                chThdSleepMilliseconds(3);
                int pad = bank * DIGITAL_INPUT_BANK_SIZE + addr;
                float v = getAdcValue(bank);
                if (drive == 0) lo[pad] = v; else hi[pad] = v;
            }
        }
    }
    sendCanPinState((uint8_t)ecuLineIndex, false);
    chprintf(chp, "   >>> PAD-DISCOVERY ECU out %d (%s): pads that respond to its drive:\r\n",
        index2human(ecuLineIndex), name);
    bool any = false;
    for (int pad = 0; pad < DISCOVERY_PAD_COUNT; pad++) {
        float d = hi[pad] - lo[pad];
        if (d < 0) d = -d;
        if (d > 0.8f) {
            any = true;
            chprintf(chp, "   >>>   stim pad %d (addr %d bank %d): low=%1.2fv high=%1.2fv delta=%1.2fv\r\n",
                index2human(pad), pad % DIGITAL_INPUT_BANK_SIZE, pad / DIGITAL_INPUT_BANK_SIZE,
                lo[pad], hi[pad], d);
        }
    }
    if (!any) {
        chprintf(chp, "   >>>   NONE - ECU out %d reaches no stim pad (open / not landed)\r\n",
            index2human(ecuLineIndex));
    }
}
