#include "ch.h"
#include "hal.h"

#include "can.h"
#include "fault.h"
#include "uart.h"
#include "io_pins.h"

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    // Fire up all of our threads

    InitCan();
    InitUart();

    while(true)
    {
        auto fault = GetCurrentFault();

        palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
        chThdSleepMilliseconds(300);
    }
}
