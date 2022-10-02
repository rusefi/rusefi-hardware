
#include "global.h"
#include "digital_inputs.h"
#include "adc.h"
#include "test_logic.h"
#include "chprintf.h"

#define COUNT 48

extern BaseSequentialStream *chp;
bool haveSeenLow[COUNT];
bool haveSeenHigh[COUNT];

        int cycleDurationMs = 1;
    int cycleCount = 2500;

bool runTest(int testLineIndex) {



        memset(haveSeenLow, 0, sizeof(haveSeenLow));
        memset(haveSeenHigh, 0, sizeof(haveSeenHigh));

  setOutputAddrIndex(testLineIndex % 16);
  int adcIndex = testLineIndex / 16;


  bool isGood = false;

    for (int i = 0;i<cycleCount && !isGood;i++) {
        int scenarioIndex = 1; // i % 2;
                setScenarioIndex(scenarioIndex);
                            chThdSleepMilliseconds(cycleDurationMs);

                            float voltage = getAdcValue(adcIndex);
                            bool isHigh = voltage > 1.5;
                            if (isHigh) {
                                if (!haveSeenHigh[testLineIndex]) {
                                    chprintf(chp, "  HIGH %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
                                }
                                haveSeenHigh[testLineIndex] = true;
                            } else {
                                if (!haveSeenLow[testLineIndex]) {
                                    chprintf(chp, "  LOW %d@%d %1.3fv\r\n", index2human(testLineIndex), i, voltage);
                                }
                                haveSeenLow[testLineIndex] = true;
                            }

//                            chprintf(chp, "scenario=%d: %1.3f V\r\n", scenarioIndex, voltage);

        isGood = haveSeenHigh[testLineIndex] && haveSeenLow[testLineIndex];
    }

    // test is successful if we saw state toggle
    return isGood;


}