#include "ch.h"
#include "hal.h"
#include "persistence.h"
#include "efifeatures.h"

#include <rusefi/pt2001.h>

GDIConfiguration *getConfiguration();

class Pt2001 : public Pt2001Base {
public:
	// returns true if init successful
	bool init();
	bool init(SPIDriver *spi, const SPIConfig *cfg);

protected:
	void acquireBus() override {
	}

	void releaseBus() override {
	}

	void select() override {
		/* Acquire ownership of the bus. */
		spiAcquireBus(driver);
		/* Setup transfer parameters. */
		spiStart(driver, spiCfg);
		/* Slave Select assertion. */
		spiSelect(driver);
	}

	void deselect() override {
		/* Slave Select de-assertion. */
		spiUnselect(driver);
		/* Ownership release. */
		spiReleaseBus(driver);
	}

	bool errorOnUnexpectedFlag() override {
		return false;
	}

	uint16_t sendRecv(uint16_t tx) override {
		return spiPolledExchange(driver, tx);
	}

	// Send `count` number of 16 bit words from `data`
	void sendLarge(const uint16_t* data, size_t count) override {
		spiSend(driver, count, data);
	}

	// GPIO reset and enable pins
	void setResetB(bool state) override {
#if (EFI_PT2001_CHIPS == 1)
		if (state) {
			palSetPad(GPIOB, 5);
		} else {
			palClearPad(GPIOB, 5);
		}
#else
		//do not drive shared gpio
		(void)state;
#endif
	}

	void setDriveEN(bool state) override {
#if (EFI_PT2001_CHIPS == 1)
		if (state) {
			palSetPad(GPIOB, 4);
		} else {
			palClearPad(GPIOB, 4);
		}
#else
		//do not drive shared gpio
		(void)state;
#endif
	}

	// GPIO inputs for various pins we need
	bool readFlag0() const override {
		return palReadPad(GPIOB, 7);
	}

	// Get battery voltage - only try to init chip when powered
	float getVbatt() const override {
		// TODO return real vbatt
		// TODO: maybe do not bother with getVbatt() at all? only used for conditional restart, this does not seem to justify any effort
		return 12;
	}

	// CONFIGURATIONS: currents, timings, voltages
	float getBoostVoltage() const override {
		return getConfiguration()->BoostVoltage;
	}

	// Currents in amps
	float getBoostCurrent() const override {
		return getConfiguration()->BoostCurrent;
	}

	float getPeakCurrent() const override {
		return getConfiguration()->PeakCurrent;
	}

	float getHoldCurrent() const override {
		return getConfiguration()->HoldCurrent;
	}

	float getPumpPeakCurrent() const override {
		return getConfiguration()->PumpPeakCurrent;
	}

	float getPumpHoldCurrent() const override {
		return getConfiguration()->PumpHoldCurrent;
	}

	// Timings in microseconds
	uint16_t getTpeakOff() const override {
		return getConfiguration()->TpeakOff;
	}

	uint16_t getTpeakTot() const override {
		return getConfiguration()->TpeakDuration;
	}

	uint16_t getTbypass() const override {
		return getConfiguration()->Tbypass;
	}

	uint16_t getTholdOff() const override {
		return getConfiguration()->TholdOff;
	}

	uint16_t getTHoldTot() const override {
		return getConfiguration()->THoldDuration;
	}

	uint16_t getTBoostMin() const override {
		return getConfiguration()->TBoostMin;
	}

	uint16_t getTBoostMax() const override {
		return getConfiguration()->TBoostMax;
	}

	uint16_t getPumpTholdOff() const override {
		return getConfiguration()->PumpTholdOff;
	}

	uint16_t getPumpTholdTot() const override {
		return getConfiguration()->PumpTholdTot;
	}

	// Print out an error message
	void onError(const char* why) override {
		// efiPrintf("PT2001 error: %s", why);
	}

    void sleepMs(size_t durationMs) override {
        chThdSleepMilliseconds(durationMs);
    }

private:
	SPIDriver* driver;
	const SPIConfig* spiCfg;
};
