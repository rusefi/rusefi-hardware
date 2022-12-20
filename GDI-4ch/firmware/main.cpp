#include "ch.h"
#include "hal.h"

#include "can.h"
#include "fault.h"
#include "uart.h"
#include "io_pins.h"
#include "persistence.h"

#include <rusefi/pt2001.h>

#include <algorithm>

static void InitPins() {
    // stm32 TX - dongle RX often White
    palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    // stm32 RX - dongle TX often Green
    palSetPadMode(GPIOA,10, PAL_MODE_INPUT_PULLUP );

    // CAN TX
    palSetPadMode(GPIOA,12, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    // CAN RX
    palSetPadMode(GPIOA,11, PAL_MODE_INPUT_PULLUP );
}

bool isOverallHappyStatus = false;

static const SPIConfig spiCfg = {
    .circular = false,
    .end_cb = nullptr,
    .ssport = GPIOB,
    .sspad = 2,
    .cr1 =
				SPI_CR1_DFF |
				SPI_CR1_MSTR |
		SPI_CR1_CPHA | SPI_CR1_BR_1 | SPI_CR1_SPE,
		.cr2 = SPI_CR2_SSOE
};

void GDIConfiguration::resetToDefaults() {
    version = PERSISTENCE_VERSION;
	BoostVoltage = 65;

	BoostCurrent = 13;
	PeakCurrent = 9.4f;
	HoldCurrent = 3.7f;

	TpeakOff = 10;
	TpeakTot = 700;
	Tbypass = 10;

	TholdOff = 60;
	THoldTot = 10000;

	TBoostMin = 100;
	TBoostMax = 400;

    PumpPeakCurrent = 5;
    PumpHoldCurrent = 3;
	PumpTholdOff = 10;
    PumpTholdTot = 10000;
}

GDIConfiguration configuration;

class Pt2001 : public Pt2001Base {
public:
	// returns true if init successful
	bool init();

protected:
	void select() override {
		spiSelect(driver);
	}

	void deselect() override {
		spiUnselect(driver);
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
		if (state) {
			palSetPad(GPIOB, 5);
		} else {
			palClearPad(GPIOB, 5);
		}
	}

	void setDriveEN(bool state) override {
		if (state) {
			palSetPad(GPIOB, 4);
		} else {
			palClearPad(GPIOB, 4);
		}
	}

	// GPIO inputs for various pins we need
	bool readFlag0() const override {
		return palReadPad(GPIOB, 7);
	}

	// Get battery voltage - only try to init chip when powered
	float getVbatt() const override {
		// TODO return real vbatt
		return 12;
	}

	// CONFIGURATIONS: currents, timings, voltages
	float getBoostVoltage() const override {
		return configuration.BoostVoltage;
	}

	// Currents in amps
	float getBoostCurrent() const override {
		return configuration.BoostCurrent;
	}

	float getPeakCurrent() const override {
		return configuration.PeakCurrent;
	}

	float getHoldCurrent() const override {
		return configuration.HoldCurrent;
	}

	float getPumpPeakCurrent() const override {
		return configuration.PumpPeakCurrent;
	}

	float getPumpHoldCurrent() const override {
		return configuration.PumpHoldCurrent;
	}

	// Timings in microseconds
	uint16_t getTpeakOff() const override {
		return configuration.TpeakOff;
	}

	uint16_t getTpeakTot() const override {
		return configuration.TpeakTot;
	}

	uint16_t getTbypass() const override {
		return configuration.Tbypass;
	}

	uint16_t getTholdOff() const override {
		return configuration.TholdOff;
	}

	uint16_t getTHoldTot() const override {
		return configuration.THoldTot;
	}

	uint16_t getTBoostMin() const override {
		return configuration.TBoostMin;
	}

	uint16_t getTBoostMax() const override {
		return configuration.TBoostMax;
	}

	uint16_t getPumpTholdOff() const override {
		return configuration.PumpTholdOff;
	}

	uint16_t getPumpTholdTot() const override {
		return configuration.PumpTholdTot;
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
};

bool Pt2001::init() {
	palSetPadMode(GPIOA, 5, PAL_MODE_STM32_ALTERNATE_PUSHPULL);    // sck
	palSetPadMode(GPIOA, 6, PAL_MODE_INPUT);    // miso
	palSetPadMode(GPIOA, 7, PAL_MODE_STM32_ALTERNATE_PUSHPULL);    // mosi

	// GD32 errata, PB1 must have certain states for PB2 to work
	palSetPadMode(GPIOB, 1, PAL_MODE_INPUT);
	palSetPadMode(GPIOB, 2, PAL_MODE_OUTPUT_PUSHPULL);	// chip select
	palSetPad(GPIOB, 2);

	// Set debug pins remap mode to use PB4 as normal pin
	AFIO->MAPR = AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	palSetPadMode(GPIOB, 4, PAL_MODE_OUTPUT_PUSHPULL);	// DRVEN
	palClearPad(GPIOB, 4);

	palSetPadMode(GPIOB, 5, PAL_MODE_OUTPUT_PUSHPULL);	// reset
	palClearPad(GPIOB, 5);

	palSetPadMode(GPIOB, 7, PAL_MODE_INPUT_PULLDOWN);	// flag0

	driver = &SPID1;
	spiStart(driver, &spiCfg);
	spiUnselect(driver);

	// Wait 1/2 second for things to wake up
	chThdSleepMilliseconds(500);

	return restart();
}

Pt2001 chip;

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    configuration.resetToDefaults();

    // Fire up all of our threads

    InitPins();
    bool isFlashOk = InitFlash();
    InitCan();
    InitUart();

	palSetPadMode(LED_BLUE_PORT, LED_BLUE_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(LED_BLUE_PORT, LED_BLUE_PIN);
	palSetPadMode(LED_GREEN_PORT, LED_GREEN_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(LED_GREEN_PORT, LED_GREEN_PIN);

    bool isOverallHappyStatus = false;
    if (isFlashOk) {
	    isOverallHappyStatus = chip.init();
	}

    while (true) {
        if (isOverallHappyStatus) {
            // happy board - green D21 blinking
            palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
        } else {
            palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);
        }

        chThdSleepMilliseconds(100);
    }

    while(true)
    {
        //auto fault = GetCurrentFault();

        //palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);
        chThdSleepMilliseconds(100);
    }
}
