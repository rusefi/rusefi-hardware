#include "ch.h"
#include "hal.h"

#include "can.h"
#include "fault.h"
#include "uart.h"
#include "io_pins.h"

static void InitPins() {
    // stm32 TX - dongle RX often White
    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    // stm32 RX - dongle TX often Green
    palSetPadMode(GPIOA,10, PAL_MODE_INPUT_PULLUP );

    // CAN TX
    palSetPadMode(GPIOA,12, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    // CAN RX
    palSetPadMode(GPIOA,11, PAL_MODE_INPUT_PULLUP );
}

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    // Fire up all of our threads

    InitPins();
    InitCan();
    InitUart();

    while(true)
    {
        auto fault = GetCurrentFault();

        palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
        chThdSleepMilliseconds(300);
    }
}
