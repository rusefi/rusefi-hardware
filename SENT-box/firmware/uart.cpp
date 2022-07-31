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

    .speed = 38400,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
    .rxhalf_cb = nullptr,
};

static char printBuffer[200];

static THD_WORKING_AREA(waUartThread, 256);

static void UartThread(void*)
{
    size_t writeCount;

    while(true)
    {

#if SENT_DEV == SENT_GM_ETB
      if(SENT_IsRawData())
      {
          uint8_t TempRawBuf[8];

          SENT_GetRawData(TempRawBuf);

          writeCount = chsnprintf(printBuffer, 200, "%03d %03d %03d %03d %03d %03d %03d %03d err %06d %06d %06d %06d %06d\r\n",
                                                TempRawBuf[0], TempRawBuf[1], TempRawBuf[2], TempRawBuf[3], TempRawBuf[4], TempRawBuf[5], TempRawBuf[6], TempRawBuf[7],
                                                SENT_GetIntervalErr(), SENT_GetMaxIntervalErrCnt(), SENT_GetSyncErrCnt(), SENT_GetCrcErrCnt(),
                                                SENT_GetSyncCnt());
      }
      else
      {
          writeCount = chsnprintf(printBuffer, 200, "%04d %04d %03d err %06d %06d %06d %06d %06d\r\n",
                                      SENT_GetOpenThrottleVal(), SENT_GetClosedThrottleVal(), SENT_GetThrottleValPrec(),
                                      SENT_GetIntervalErr(), SENT_GetMaxIntervalErrCnt(), SENT_GetSyncErrCnt(), SENT_GetCrcErrCnt(),
                                      SENT_GetSyncCnt());
      }
#elif SENT_DEV == SENT_SILABS_SENS
        size_t writeCount = chsnprintf(printBuffer, 200, "%04d %04d %04d %04d err %06d %06d %06d %06d %06d %06d\r\n",
                                         SENT_GetData(SENT_CH1), SENT_GetData(SENT_CH2),  SENT_GetData(SENT_CH3),  SENT_GetData(SENT_CH4),
                                         SENT_GetMinIntervalErrCnt(), SENT_GetMaxIntervalErrCnt(), SENT_GetSyncErrCnt(),
                                         SENT_GetRollErrCnt(), SENT_GetCrcErrCnt(), SENT_GetSyncCnt());
#endif
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
