/**
 file adc.h
 */

#pragma once

#include "global.h"

typedef uint16_t adcsample_t;

#define ADC_GRP_NUM_CHANNELS   3
#define ADC_VREF 				(3.300f)

void initAnalogInputs();
adcsample_t getAdcRawValue(int channel);
/*
 * value in volts
 */
float getAdcValue(int channel);
