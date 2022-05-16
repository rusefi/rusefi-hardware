#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"
#include "sent.h"

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
    .rxhalf_cb = nullptr,
};

static char printBuffer[200];

static THD_WORKING_AREA(waUartThread, 256);

static void UartThread(void*)
{
    while(true)
    {

        size_t writeCount = chsnprintf(printBuffer, 200, "%04d %04d %04d %04d %04d %04d\r\n", SENT_GetData(0), SENT_GetData(1),  SENT_GetData(2),  SENT_GetData(3), SENT_GetRollErrCnt(), SENT_GetCrcErrCnt());
        uartStartSend(&UARTD1, writeCount, printBuffer);

        chThdSleepMilliseconds(20);
    }
}

void InitUart()
{
  // stm32 TX/UART1 - dongle RX often White
  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
  // stm32 RX/UART1 - dongle TX often Green
  palSetPadMode(GPIOA,10, PAL_MODE_INPUT_PULLUP );

    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
