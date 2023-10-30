#pragma once

#include "global.h"

void initStimDigitalInputs();
bool testEcuDigitalOutputs(size_t startStepIndex);
bool testEcuDcOutputs(size_t startStepIndex);

/**
 * controls what input channel we are sensing
 */
void setOutputAddrIndex(int index);
void setScenarioIndex(int index);


