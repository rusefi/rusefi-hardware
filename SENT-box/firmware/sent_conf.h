#pragma once

#include "sent.h"

#if SENT_DEV == SENT_GM_ETB
#define SENT_CHANNELS_NUM 2 // Number of sent channels
#elif SENT_DEV == SENT_SILABS_SENS
#define SENT_CHANNELS_NUM 4 // Number of sent channels
#endif

#define SENT_MSG_DATA_SIZE      6
/* Status + two 12-bit signals + CRC */
#define SENT_MSG_PAYLOAD_SIZE   (1 + SENT_MSG_DATA_SIZE + 1)  // Size of payload
