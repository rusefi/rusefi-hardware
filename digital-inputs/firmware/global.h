#pragma once

#include "ch.h"
#include "hal.h"

#include "io_pins.h"
#include "efilib.h"

#define LED_GREEN LINE_LED1
#define LED_BLUE LINE_LED2
#define LED_RED LINE_LED3

#define THREAD_STACK 512

struct OutputMode {
    bool displayCanTransmit = true;
    bool displayCanReceive = true;
    bool verboseDigitalOutputs = true;
};

void setErrorLedAndRedText();