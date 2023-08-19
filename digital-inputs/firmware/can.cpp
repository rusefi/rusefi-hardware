#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "can.h"
#include "../../ext/libfirmware/can/can_common.h"


extern BaseSequentialStream *chp;

static const CANConfig cancfg = {
  CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
  CAN_BTR_SJW(0) | CAN_BTR_TS2(1) |
  CAN_BTR_TS1(8) | CAN_BTR_BRP(6)
};


void receiveBoardStatus(const uint8_t msg[8]) {
	int boardId = (msg[0] << 8) | msg[1];
	int numSecondsSinceReset = (msg[2] << 16) | (msg[3] << 8) | msg[4];

	// todo: process board status
	chprintf(chp, " * BoardStatus: BoardID=%d numSecs=%d\r\n", boardId, numSecondsSinceReset);
}

void processCanRxMessage(const CANRxFrame& frame) {
	if (CAN_EID(frame) == BENCH_TEST_BOARD_STATUS) {
		receiveBoardStatus(frame.data8);
	}

}

static THD_WORKING_AREA(can_tx_wa, 256);
static THD_FUNCTION(can_tx, p) {
  CANTxFrame txmsg;

  (void)p;
  chRegSetThreadName("transmitter");
  txmsg.IDE = CAN_IDE_EXT;
  txmsg.EID = 0x01234567;
  txmsg.RTR = CAN_RTR_DATA;
  txmsg.DLC = 8;
  txmsg.data32[0] = 0x55AA55AA;
  txmsg.data32[1] = 0x00FF00FF;

  while (true) {
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100));
    chThdSleepMilliseconds(500);
  }
}

static THD_WORKING_AREA(can_rx_wa, 256);
static THD_FUNCTION(can_rx, p) {
  CANRxFrame rxmsg;

  (void)p;
  while (true) {
    msg_t result = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &rxmsg, TIME_INFINITE);
	if (result != MSG_OK) {
		continue;
	}

    processCanRxMessage(rxmsg);
  }
}


// we are lucky - all CAN pins use the same AF
#define EFI_CAN_AF 9

void initCan() {
  palSetPadMode(GPIOA,11, PAL_MODE_ALTERNATE(EFI_CAN_AF));
  palSetPadMode(GPIOA,12, PAL_MODE_ALTERNATE(EFI_CAN_AF));

  canStart(&CAND1, &cancfg);

  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7,
                    can_tx, NULL);
  chThdCreateStatic(can_rx_wa, sizeof(can_rx_wa), NORMALPRIO + 7,
                    can_rx, NULL);
}
