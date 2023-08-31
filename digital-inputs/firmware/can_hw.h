#include <initializer_list>
#include "ch.h"
#include "hal.h"

void sendCanTxMessage(const CANTxFrame & frame);
void sendCanTxMessage(int EID, std::initializer_list<uint8_t> data);
void initCanHw();
