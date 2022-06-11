#include "ch.h"
#include "hal.h"

// generic helper methods
void setup_spi();
unsigned short recv_16bit_spi();
void spi_writew(unsigned short param);
unsigned short txrx_16bit_spi(const unsigned short param);

void mcClearDriverStatus();
unsigned short readId();