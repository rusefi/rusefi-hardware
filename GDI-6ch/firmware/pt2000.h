/*
 * @file pt2000.h
 *
 * chris is altering the original pt2001 for gdi6 without internal submodules, will merge as one peak-hold class at a later date
 */

#pragma once

#include <cstddef>
#include <cstdint>

#include "pt2000_memory_map.h"
#include "ch.h"
#include "hal.h"


//extern "C" void __cxa_pure_virtual(void) { while(1); }


enum class McFault : uint8_t
{
    None = 0,
    NoFlash = 1,
    UnderVoltageAfter = 2,
    NoComm = 3,
    flag0 = 4,
    UnderVoltage5 = 5,
    Driven = 6,
    UnderVoltage7 = 7,
};

class Pt2000Base {
public:
	// Reinitialize the PT2000 chip, returns true if successful
	// or do we check the return is within range to prevent overflow
	bool restart();

	// Disable the PT2001 chip.
	void shutdown();

	void onError(McFault p_fault) {
	    fault = p_fault;
	}

	// Re-read timing configuration and reconfigure the chip. This is safe to call while operating.
	void setTimings();

	// Set the boost voltage target. This is safe to call while operating.
	void setBoostVoltage(float volts);

    McFault fault = McFault::None;
    uint16_t status = 0;

	uint16_t readStatus(int reg);

private:
	// SPI tx/rx helpers
	void send(uint16_t tx) {
		sendRecv(tx);
	}

	uint16_t recv() {
		return sendRecv(0xFFFF);
	}

	// Chip init logic
	void setupSpi();
	uint16_t readReg(int reg);
	void writeReg(int reg, uint16_t data);
	uint16_t readId();

	void enableFlash();
	bool checkFlash();

	void downloadRam(int target);
	void downloadRegister(int target);

	// Chip IO helpers
	uint16_t readDram(MC33PT2000Mem addr);
	void writeDram(MC33PT2000Mem addr, uint16_t data);
	uint16_t readDriverStatus();
	void clearDriverStatus();

protected:
	// The consuming app must implement these functions!
	virtual void select() = 0;
	virtual void deselect() = 0;
	virtual uint16_t sendRecv(uint16_t tx) = 0;
	// Send `count` number of 16 bit words from `data`
	virtual void sendLarge(const uint16_t* data, size_t count) = 0;

	// GPIO reset and enable pins
	virtual void setResetB(bool state) = 0;
	virtual void setDriveEN(bool state) = 0;

	// GPIO inputs for various pins we need
	virtual bool readFlag0() const = 0;

	// Get battery voltage - only try to init chip when powered
	virtual float getVbatt() const = 0;

	// CONFIGURATIONS: currents, timings, voltages
	virtual float getBoostVoltage() const = 0;

	// Currents in amps
	virtual float getBoostCurrent() const = 0;
	virtual float getPeakCurrent() const = 0;
	virtual float getHoldCurrent() const = 0;

	virtual float getPumpPeakCurrent() const = 0;
	virtual float getPumpHoldCurrent() const = 0;

	// Timings in microseconds
	virtual uint16_t getTpeakOff() const = 0;
	virtual uint16_t getTpeakTot() const = 0;
	virtual uint16_t getTbypass() const = 0;
	virtual uint16_t getTholdOff() const = 0;
	virtual uint16_t getTHoldTot() const = 0;
	virtual uint16_t getTBoostMin() const = 0;
	virtual uint16_t getTBoostMax() const = 0;

	virtual uint16_t getPumpTholdOff() const = 0;
	virtual uint16_t getPumpTholdTot() const = 0;

	// Print out an error message
	virtual void onError(const char* why) = 0;

	// Sleep for some number of milliseconds
	virtual void sleepMs(size_t ms) = 0;
};
