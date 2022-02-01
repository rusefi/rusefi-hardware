#pragma once

#include "global.h"

struct io_pin {
ioportid_t port;
ioportmask_t pin;
};

void initDigitalInputs();
void setOutputIndex(int index);
void setScenarioIndex(int index);
