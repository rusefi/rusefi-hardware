#pragma once

#include "global.h"

void initStimDigitalInputs();
bool testEcuDigitalOutputs();

/**
 * controls what input channel we are sensing
 */
void setOutputAddrIndex(int index);
void setScenarioIndex(int index);


