
#include "digital_inputs.h"

io_pin addrPins[] = {
{GPIOC, 8},
{GPIOC, 6},
{GPIOC, 4},
{GPIOA, 6},
};

void initDigitalInputs() {
    for (size_t i = 0;i<efi::size(addrPins);i++) {
        io_pin *pin = &addrPins[i];
    }
}
