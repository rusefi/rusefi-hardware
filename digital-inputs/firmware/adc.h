#pragma once

#include "global.h"

typedef uint16_t adcsample_t;

#define ADC_GRP_NUM_CHANNELS   3

void initAnalogInputs();
adcsample_t getAdcValue(int channel);
