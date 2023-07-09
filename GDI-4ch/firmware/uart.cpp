#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"
#include "io_pins.h"
#include "persistence.h"
#include "fault.h"
#include "pt2001impl.h"

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
extern mfs_error_t flashState;
extern int canWriteOk;
extern int canWriteNotOk;
extern Pt2001 chip;
extern GDIConfiguration configuration;

static int counter = 0;

static THD_WORKING_AREA(waUartThread, 256);
static void UartThread(void*)
{
    while (true) {
        counter = (counter + 1) % 1000;

        size_t writeCount;
        if (chip.fault != McFault::None) {
            writeCount  = chsnprintf(printBuffer, sizeof(printBuffer), "FAULT fault=%d status=%x status2=%x 0x1A6=%x 0x1A7=%x 0x1A8=%x\r\n",
                (int)chip.fault,
                chip.status,
                chip.status5,
                chip.status6,
                chip.status7,
                chip.status8
            );

        } else {
            writeCount  = chsnprintf(printBuffer, sizeof(printBuffer), "%x %d %d HAPPY fault=%d status=%x status2=%x flash=%d %d CAN o/e %d %d\r\n",
            configuration.inputCanID,
                (int)(configuration.PumpPeakCurrent * 1000),
                configuration.updateCounter,
                (int)chip.fault,
                chip.status,
                chip.status5,
                (int)flashState, counter,
                canWriteOk, canWriteNotOk);

            }
        uartStartSend(&UARTD1, writeCount, printBuffer);

        chThdSleepMilliseconds(200);
    }
}

void InitUart()
{
    uartStart(&UARTD1, &uartCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
