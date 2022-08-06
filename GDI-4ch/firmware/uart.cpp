#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"
#include "io_pins.h"

static const UARTConfig uartCfg =
{
    .txend1_cb = nullptr,
    .txend2_cb = nullptr,
    .rxend_cb = nullptr,
    .rxchar_cb = nullptr,
    .rxerr_cb = nullptr,
    .timeout_cb = nullptr,

#ifdef STM32F0XX
    .timeout = 0,
#endif

    .speed = UART_BAUD_RATE,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
    .rxhalf_cb = nullptr,
};

static char printBuffer[200];

extern bool isOverallHappyStatus;

static THD_WORKING_AREA(waUartThread, 256);
static void UartThread(void*)
{
    while(true)
    {

        size_t writeCount = chsnprintf(printBuffer, 200, "%d %d %d %d\r\n", isOverallHappyStatus, 0, 0, 100);
        uartStartSend(&UARTD1, writeCount, printBuffer);

        chThdSleepMilliseconds(20);
    }
}

void InitUart()
{
    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
