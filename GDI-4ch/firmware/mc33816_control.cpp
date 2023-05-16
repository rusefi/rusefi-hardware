
#include "mc33816_control.h"

static auto spiDriver = &SPID1;

// Receive 16bits
unsigned short recv_16bit_spi() {
	return spiPolledExchange(spiDriver, 0xFFFF);
}

// This could be used to detect if check byte is wrong.. or use a FLAG after init
unsigned short txrx_16bit_spi(const unsigned short param) {
	return spiPolledExchange(spiDriver, param);
}

void mcClearDriverStatus() {
	// Note: There is a config at 0x1CE & 1 that can reset this status config register on read
	// otherwise the reload/recheck occurs with this write
	// resetting it is necessary to clear default reset behavoir, as well as if an issue has been resolved
	setup_spi(); // ensure on common page?
	spiSelect(spiDriver);
	spi_writew((0x0000 | 0x1D2 << 5) + 1); // write, location, one word
	spi_writew(0x0000); // anything to clear
	spiUnselect(spiDriver);
}

void setup_spi() {
	spiSelect(spiDriver);
	// Select Channel command
	spi_writew(0x7FE1);
    // Common Page
	spi_writew(0x0004);


	// Configure SPI command
	spi_writew(0x3901);
	// Mode A + Watchdog timer full
    //spi_writew(0x001F);
	spi_writew(0x009F); // + fast slew rate on miso
	spiUnselect(spiDriver);
}

// Send 16bits
void spi_writew(unsigned short param) {
	//spiSelect(spiDriver);
	spiPolledExchange(spiDriver, param);
	//spiUnselect(spiDriver);
}

unsigned short readId() {
	spiSelect(spiDriver);
	spi_writew(0xBAA1);
	unsigned short ID =  recv_16bit_spi();
	spiUnselect(spiDriver);
	return ID;
}

// Read a single word in Data RAM
unsigned short mcReadDram(MC33816Mem addr) {
	unsigned short readValue;
	spiSelect(spiDriver);
	// Select Channel command, Common Page
    spi_writew(0x7FE1);
    spi_writew(0x0004);
    // read (MSB=1) at data ram x9 (SCV_I_Hold), and 1 word
    spi_writew((0x8000 | addr << 5) + 1);
    readValue = recv_16bit_spi();

    spiUnselect(spiDriver);
    return readValue;
}

// Update a single word in Data RAM
void mcUpdateDram(MC33816Mem addr, uint16_t data) {
	spiSelect(spiDriver);
	// Select Channel command, Common Page
    spi_writew(0x7FE1);
    spi_writew(0x0004);
    // write (MSB=0) at data ram x9 (SCV_I_Hold), and 1 word
    spi_writew((addr << 5) + 1);
    spi_writew(data);

    spiUnselect(spiDriver);
}
