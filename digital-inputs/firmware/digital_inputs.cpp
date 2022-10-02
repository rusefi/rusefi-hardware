
#include "global.h"
#include "digital_inputs.h"
#include "chprintf.h"

#define XOR_MAGIC 1

static io_pin addrPins[] = {
{GPIOC, 8}, // ADR0
{GPIOC, 6},
{GPIOC, 5},
{GPIOA, 6},
};

static io_pin scenarioPins[] = {
{GPIOB, 12}, // OUT0 - this controls pull-up/pull-down on pads  #1-16
{GPIOB, 11}, //                                                 17-32
{GPIOC, 7},
{GPIOC, 4},  // only test 49
};

static io_pin muxOff = {GPIOA, 7};

extern BaseSequentialStream *chp;

void initDigitalInputs() {
    for (size_t i = 0;i < efi::size(addrPins);i++) {
        io_pin *pin = &addrPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
    }

    for (size_t i = 0;i < efi::size(scenarioPins);i++) {
        io_pin *pin = &scenarioPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
    }

    palSetPadMode(muxOff.port, muxOff.pin, PAL_MODE_OUTPUT_PUSHPULL);
    palWritePad(muxOff.port, muxOff.pin, 0 ^ XOR_MAGIC);
}

void setOutputAddrIndex(int index) {
    int param = index;
    chprintf(chp, "Setting ADDR %d\r\n", param);
    for (size_t i = 0;i<efi::size(addrPins);i++) {
        int bitState = (index & 1) ^ XOR_MAGIC;
        index = index / 2;
        io_pin *pin = &addrPins[i];
//        chprintf(chp, "ADDR %d: bit=%d %d\n", param, i, bitState);
        palWritePad(pin->port, pin->pin, bitState);
    }
}

void setScenarioIndex(int index) {
    for (size_t i = 0;i<efi::size(scenarioPins);i++) {
        int bitState = index & 1;
        // please note lack of XOR
        index = index / 2;
        io_pin *pin = &scenarioPins[i];
        palWritePad(pin->port, pin->pin, bitState);
    }
}