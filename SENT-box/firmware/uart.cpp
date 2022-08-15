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

#define BUFFER_SIZE 300

static char printBuffer[BUFFER_SIZE];

static THD_WORKING_AREA(waUartThread, 256);

static void UartThread(void*)
{
    while(true)
    {
        size_t writeCount = 0;
#if SENT_DEV == SENT_GM_ETB
        if (1)
        {
            uint8_t ptr[8];

            SENT_GetRawNibbles(ptr);
            writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
                "nibbles: ST=%x D=%x:%x:%x:%x:%x:%x CRC=%x",
                ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7]
                );

            writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
                " Errs Short: %06d Long: %06d Sync: %06d CRC: %06d (%03d %%). Frames: %06d\r\n",
                SENT_GetShortIntervalErrCnt(), SENT_GetLongIntervalErrCnt(), SENT_GetSyncErrCnt(), SENT_GetCrcErrCnt(),
                SENT_GetCrcErrCnt() * 100 / SENT_GetFrameCnt(0),
                SENT_GetFrameCnt(0)
                );

        }
        if(SENT_IsRawData())
        {
            #if 0
            writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
                " Si7215: %04d * 0.1 mT cnt %03d. Tick = %04d nS. Errs Short: %06d Long: %06d Sync: %06d CRC: %06d. Frames: %06d\r\n",
                Si7215_GetMagneticField(0), Si7215_GetCounter(0),
                SENT_GetTickTimeNs(),
                SENT_GetShortIntervalErrCnt(), SENT_GetLongIntervalErrCnt(), SENT_GetSyncErrCnt(), SENT_GetCrcErrCnt(),
                SENT_GetFrameCnt(0)
                );
            #else
            uint32_t gm_pressure = gm_GetPressure(0);
            writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
                " GM: St %x, Sig0 %04d, Sig1 %04d, %d.%03d Atm Tick = %04d nS.\r\n",
                gm_GetStat(0), gm_GetSig0(0), gm_GetSig1(0), gm_pressure / 1000, gm_pressure % 1000,
                SENT_GetTickTimeNs()
                );

            uint16_t mask = SENT_GetSlowMessagesFlags(0);
            int i;
            for (i = 0; i < 16; i++) {
                if (mask & (1 << i)) {  
                    uint16_t id = SENT_GetSlowMessageID(0, i);
                    uint16_t msg = SENT_GetSlowMessage(0, i);
                    if ((id == 16) || (id == 22))
                    {
                        /* these messages looks loke temperature */
                        writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
                            "  msg %d: 0x%04x (%d), T = %d.%05dC ?\r\n",
                            id, msg, msg, msg / 32, (msg % 32) * 3125
                            );
                    }
                    else
                    {
                        writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
                            "  msg %d: 0x%04x (%d)\r\n",
                            SENT_GetSlowMessageID(0, i), SENT_GetSlowMessage(0, i), SENT_GetSlowMessage(0, i)
                            );
                    }
                }
            }
            #endif
        }
        else
        {
            writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
                "ETB %04d %04d pos=%03d err %06d %06d s_e=%06d c_err=%06d sync=%06d rate=%04d\r\n",
                SENT_GetOpenThrottleVal(), SENT_GetClosedThrottleVal(), SENT_GetThrottleValPrec(),
                SENT_GetShortIntervalErrCnt(), SENT_GetLongIntervalErrCnt(), SENT_GetSyncErrCnt(), SENT_GetCrcErrCnt(),
                SENT_GetSyncCnt(),
                SENT_GetErrPercent()
                );
        }
#elif SENT_DEV == SENT_SILABS_SENS
        writeCount += chsnprintf(printBuffer + writeCount, BUFFER_SIZE - writeCount,
            "%04d %04d %04d %04d err %06d %06d %06d %06d %06d\r\n",
            SENT_GetData(SENT_CH1), SENT_GetData(SENT_CH2),  SENT_GetData(SENT_CH3),  SENT_GetData(SENT_CH4),
            SENT_GetMinIntervalErrCnt(), SENT_GetMaxIntervalErrCnt(), SENT_GetSyncErrCnt(),
            SENT_GetCrcErrCnt(), SENT_GetSyncCnt()
            );
#endif
        uartStartSend(&UARTD1, writeCount, printBuffer);

        chThdSleepMilliseconds(100);
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
