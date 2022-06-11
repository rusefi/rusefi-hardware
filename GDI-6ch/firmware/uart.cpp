#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"

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

    .speed = 115200,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
};

static char printBuffer[200];

static THD_WORKING_AREA(waUartThread, 256);
static void UartThread(void*)
{
    while(true)
    {
        float lambda = 0;
        int lambdaIntPart = 1;
        int lambdaThousandths = 2;

        size_t writeCount = chsnprintf(printBuffer, 200, "%d.%03d\t%d\t%d\r\n", 0, 0, 0, 100);
        uartStartSend(&UARTD1, writeCount, printBuffer);

        chThdSleepMilliseconds(20);
    }
}

void InitUart()
{
    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
