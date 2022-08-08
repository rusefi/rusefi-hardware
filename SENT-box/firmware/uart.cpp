#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"
#include "sent.h"
#include "io_pins.h"
#include "mcu-util.h"

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

static char printBuffer[300];

static THD_WORKING_AREA(waUartThread, 256);

extern uint8_t sentTempNibblArr[SENT_CHANNELS_NUM][SENT_MSG_PAYLOAD_SIZE];

static void UartThread(void*)
{
    size_t writeCount;

    while(true)
    {

     uint32_t revCode = ARM_REV_CODE();

#if SENT_DEV == SENT_GM_ETB
      if(SENT_IsRawData())
      {
          uint8_t TempRawBuf[8];

          SENT_GetRawData(TempRawBuf);

          writeCount = chsnprintf(printBuffer, 200, "raw %03d %03d %03d %03d %03d %03d %03d %03d err %06d %06d %06d %06d %06d\r\n",
                                                TempRawBuf[0], TempRawBuf[1], TempRawBuf[2], TempRawBuf[3], TempRawBuf[4], TempRawBuf[5], TempRawBuf[6], TempRawBuf[7],
                                                SENT_GetIntervalErr(), SENT_GetMaxIntervalErrCnt(), SENT_GetSyncErrCnt(), SENT_GetCrcErrCnt(),
                                                SENT_GetSyncCnt());
      }
      else
      {
      int n1 = sentTempNibblArr[0][1];
      int n2 = sentTempNibblArr[0][2];
      int n3 = sentTempNibblArr[0][3];
      int n4 = sentTempNibblArr[0][4];
      int n5 = sentTempNibblArr[0][5];
      int n6 = sentTempNibblArr[0][6];
          writeCount = chsnprintf(printBuffer, 200, "[%x] %02d %02d %02d %02d %02d %02d ETB %04d %04d pos=%03d err %06d %06d s_e=%06d c_err=%06d sync=%06d rate=%04d\r\n",
          revCode,
          n1,
          n2,
          n3,
          n4,
          n5,
          n6,
                                      SENT_GetOpenThrottleVal(), SENT_GetClosedThrottleVal(), SENT_GetThrottleValPrec(),
                                      SENT_GetIntervalErr(), SENT_GetMaxIntervalErrCnt(), SENT_GetSyncErrCnt(), SENT_GetCrcErrCnt(),
                                      SENT_GetSyncCnt(),
                                      SENT_GetErrPercent()
                                      );
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
  palSetPadMode(UART_GPIO_PORT, UART_TX_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
  // stm32 RX/UART1 - dongle TX often Green
  palSetPadMode(UART_GPIO_PORT,UART_RX_PIN, PAL_MODE_INPUT_PULLUP );

    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
