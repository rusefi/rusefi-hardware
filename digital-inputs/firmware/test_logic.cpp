
#include "global.h"
#include "digital_inputs.h"
#include "adc.h"
#include "test_logic.h"
#include "chprintf.h"

#define COUNT 16

extern BaseSequentialStream *chp;
bool haveSeenLow[COUNT];
bool haveSeenHigh[COUNT];

        int cycleDurationMs = 50;
    int cycleCount = 50;

bool runTest(int testLineIndex) {



        memset(haveSeenLow, 0, sizeof(haveSeenLow));
        memset(haveSeenHigh, 0, sizeof(haveSeenHigh));

            setOutputAddrIndex(testLineIndex);


    for (int i = 0;i<cycleCount;i++) {
    int scenarioIndex = i % 2;
                setScenarioIndex(scenarioIndex);

                            float voltage = getAdcValue(0);
                            bool isHigh = voltage > 1.5;
                            if (isHigh) {
                                haveSeenHigh[testLineIndex] = true;
                            } else {
                                haveSeenLow[testLineIndex] = true;
                            }

                            chThdSleepMilliseconds(cycleDurationMs);
//                            chprintf(chp, "scenario=%d: %1.3f V\r\n", scenarioIndex, voltage);


    }

    // test is successful if we saw state toggle
    return haveSeenHigh[testLineIndex] && haveSeenLow[testLineIndex];


}