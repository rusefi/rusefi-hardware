#include "ch.h"
#include "hal.h"

#include "io_pins.h"

#include "board.h"

void miscInit(void)
{
    // Board ID
    palSetPadMode(BOARD_ID_PORT, BOARD_ID_PIN, PAL_MODE_INPUT);
}

int boardGetId(void)
{
	return !!palReadPad(BOARD_ID_PORT, BOARD_ID_PIN);
}
