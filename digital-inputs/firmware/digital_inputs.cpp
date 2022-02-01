
#include "digital_inputs.h"

io_pin addrPins[] = {
{GPIOC, 8},
{GPIOC, 6},
{GPIOC, 4},
{GPIOA, 6},
};

io_pin scenarioPins[] = {
{GPIOB, 12},
{GPIOB, 11},
{GPIOC, 7},
{GPIOC, 4},
};

void initDigitalInputs() {
    for (size_t i = 0;i < efi::size(addrPins);i++) {
        io_pin *pin = &addrPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
    }

    for (size_t i = 0;i < efi::size(scenarioPins);i++) {
        io_pin *pin = &scenarioPins[i];
       	palSetPadMode(pin->port, pin->pin, PAL_MODE_OUTPUT_PUSHPULL);
    }
}

void setOutputIndex(int index) {
    for (size_t i = 0;i<efi::size(addrPins);i++) {
        int bitState = index & 1;
        index = index / 2;
        io_pin *pin = &addrPins[i];
        palWritePad(pin->port, pin->pin, bitState);
    }
}

void setScenarioIndex(int index) {
    for (size_t i = 0;i<efi::size(scenarioPins);i++) {
        int bitState = index & 1;
        index = index / 2;
        io_pin *pin = &addrPins[i];
        palWritePad(pin->port, pin->pin, bitState);
    }
}