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
    bool displayCanReceive = false;
    bool verboseDigitalOutputs = true;
};

#define CORE_CLOCK STM32_SYSCLK
#define US_TO_NT_MULTIPLIER (CORE_CLOCK / 1000000)

/**
 * Get a monotonically increasing (but wrapping) 32-bit timer value
 * Implemented at port level, based on timer or CPU tick counter
 * Main source of EFI clock, SW-extended to 64bits
 */
uint32_t getTimeNowLowerNt();

void setErrorLedAndRedText();
