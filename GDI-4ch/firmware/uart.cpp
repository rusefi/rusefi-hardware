#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "efifeatures.h"

#include "uart.h"
#include "io_pins.h"
#include "persistence.h"
#include "fault.h"
#include "pt2001impl.h"
#include "sent.h"
#include "can.h"

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
extern Pt2001 chips[EFI_PT2001_CHIPS];
extern GDIConfiguration configuration;

static int counter = 0;

static THD_WORKING_AREA(waUartThread, 256);
static void UartThread(void*)
{
    chRegSetThreadName("UART debug");
    while (true) {
        counter = (counter + 1) % 1000;


		chprintf(chp, "Flash=%d %d CAN o/e %d %d\r\n",
			(int)flashState, counter,
			canWriteOk, canWriteNotOk);
        for (size_t i = 0; i < EFI_PT2001_CHIPS; i++) {
            if (chips[i].fault != McFault::None) {
                chprintf(chp, "%d: FAULT fault=%d status=%x\r\n",
                    i,
                    (int)chips[i].fault,
                    chips[i].status
//                    ,
//                    chips[i].status5,
//                    chips[i].status6,
//                    chips[i].status7,
//                    chips[i].status8
                    );
            } else {
                chprintf(chp, "%d: 0x%03x %d %d HAPPY fault=%d status=%x\r\n",
                    i,
                    canGetInputCanIdBase(i),
                    (int)(configuration.PumpPeakCurrent * 1000),
                    configuration.updateCounter,
                    (int)chips[i].fault,
                    chips[i].status);
            }
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
