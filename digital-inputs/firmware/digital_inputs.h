#pragma once

#include "global.h"

void initDigitalInputs();

struct io_pin {
ioportid_t port;
ioportmask_t pin;
};