#include "ch.h"
#include "hal.h"

#include "rusefi/pt2001_memory_map.h"

// generic helper methods
void setup_spi();
unsigned short recv_16bit_spi();
void spi_writew(unsigned short param);
unsigned short txrx_16bit_spi(const unsigned short param);

void mcClearDriverStatus();
unsigned short readId();

unsigned short mcReadDram(MC33816Mem addr);
void mcUpdateDram(MC33816Mem addr, unsigned short data);