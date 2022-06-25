#pragma once

#include "global.h"

struct io_pin {
ioportid_t port;
ioportmask_t pin;
};

void initDigitalInputs();
/**
 * controls what input channel we are sensing
 */
void setOutputAddrIndex(int index);
void setScenarioIndex(int index);
