#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "uart.h"
#include "io_pins.h"
#include "persistence.h"
#include "fault.h"
#include "pt2001impl.h"
#include "sent.h"

/**
 * @brief Global variables
 */
BaseSequentialStream *chp = (BaseSequentialStream *) &SD1;

static const SerialConfig serialCfg =
{
  UART_BAUD_RATE,
  0,
  USART_CR2_STOP1_BITS,
  0
};

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

        if (chip.fault != McFault::None) {
            chprintf(chp, "FAULT fault=%d status=%x status2=%x 0x1A6=%x 0x1A7=%x 0x1A8=%x\r\n",
                (int)chip.fault,
                chip.status,
                chip.status5,
                chip.status6,
                chip.status7,
                chip.status8);
        } else {
            chprintf(chp, "%x %d %d HAPPY fault=%d status=%x status2=%x flash=%d %d CAN o/e %d %d\r\n",
                configuration.inputCanID,
                (int)(configuration.PumpPeakCurrent * 1000),
                configuration.updateCounter,
                (int)chip.fault,
                chip.status,
                chip.status5,
                (int)flashState, counter,
                canWriteOk, canWriteNotOk);
        }

        sentDebug();

        chThdSleepMilliseconds(200);
    }
}

void InitUart()
{
    sdStart(&SD1, &serialCfg);

    chThdCreateStatic(waUartThread, sizeof(waUartThread), NORMALPRIO, UartThread, nullptr);
}
