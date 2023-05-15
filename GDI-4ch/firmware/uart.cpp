#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"
#include "io_pins.h"
#include "persistence.h"
#include "fault.h"

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
extern mfs_error_t flashStartState;
extern int canWriteOk;
extern int canWriteNotOk;

static int counter = 0;

static THD_WORKING_AREA(waUartThread, 256);
static void UartThread(void*)
{
    while (true) {
        counter = (counter + 1) % 1000;

        size_t writeCount = chsnprintf(printBuffer, sizeof(printBuffer), "happy=%d fault=%d flash=%d %d CAN o/e %d %d\r\n", HasFault(), (int)GetCurrentFault(), (int)flashStartState, counter,
            canWriteOk, canWriteNotOk);
        uartStartSend(&UARTD1, writeCount, printBuffer);

        chThdSleepMilliseconds(20);
    }
}

void InitUart()
{
    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
