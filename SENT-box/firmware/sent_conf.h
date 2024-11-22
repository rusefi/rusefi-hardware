#pragma once

#include "sent.h"

#if SENT_DEV == SENT_GM_ETB
#define SENT_CHANNELS_NUM 2 // Number of sent channels
#elif SENT_DEV == SENT_SILABS_SENS
#define SENT_CHANNELS_NUM 4 // Number of sent channels
#endif

#include "sent_constants.h"
