#include "ch.h"
#include "hal.h"

#include "io_pins.h"

#include "board.h"

void miscInit(void)
{
    // Board ID
    palSetPadMode(BOARD_ID_PORT, BOARD_ID_PIN, PAL_MODE_INPUT);
}

/**
 * return 0 or 1
 */
int boardGetId(void) {
    // you would not believe it: double-exclamation mark is a feature here to guarantee 0 or 1 value
	return !!palReadPad(BOARD_ID_PORT, BOARD_ID_PIN);
}
