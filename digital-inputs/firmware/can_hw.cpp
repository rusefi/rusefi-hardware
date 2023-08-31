#include "can_hw.h"
#include "global.h"
#include "chprintf.h"
#include "containers/fifo_buffer.h"

extern BaseSequentialStream *chp;

static fifo_buffer_sync<CANTxFrame> txFifo;

void sendCanTxMessage(const CANTxFrame & frame) {
	if (!txFifo.put(frame)) {
		chprintf(chp, "CAN sendCanTxMessage() problems");
	}
}

void sendCanTxMessage(int EID, std::initializer_list<uint8_t> data) {
	CANTxFrame txmsg;
	txmsg.IDE = CAN_IDE_EXT;
	txmsg.EID = EID;
	txmsg.RTR = CAN_RTR_DATA;
	txmsg.DLC = 8;

	size_t idx = 0;
    for (uint8_t v : data) {
		txmsg.data8[idx] = v;
		idx++;
	}
	sendCanTxMessage(txmsg);
}

static THD_WORKING_AREA(can_tx_wa, THREAD_STACK);
static THD_FUNCTION(can_tx, p) {
  CANTxFrame txmsg;

  (void)p;
  chRegSetThreadName("transmitter");

  while (true) {
	if (txFifo.get(txmsg, TIME_MS2I(100))) {
    	canTransmit(&CAND1, CAN_ANY_MAILBOX, &txmsg, TIME_MS2I(100));
    }
  }
}

void initCanHw() {
  chThdCreateStatic(can_tx_wa, sizeof(can_tx_wa), NORMALPRIO + 7,
                    can_tx, NULL);

}
