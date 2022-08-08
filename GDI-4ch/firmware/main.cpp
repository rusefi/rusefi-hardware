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
		// keep voltage safely low for now...
		return 40;
		// return 65;
	}

	// Currents in amps
	float getBoostCurrent() const override {
		return 13;
	}

	float getPeakCurrent() const override {
		return 9.4f;
	}

	float getHoldCurrent() const override {
		return 3.7f;
	}

	float getPumpPeakCurrent() const override {
		return 5;
	}

	float getPumpHoldCurrent() const override {
		return 3;
	}

	// Timings in microseconds
	uint16_t getTpeakOff() const override {
		return 10;
	}

	uint16_t getTpeakTot() const override {
		return 700;
	}

	uint16_t getTbypass() const override {
		return 10;
	}

	uint16_t getTholdOff() const override {
		return 60;
	}

	uint16_t getTHoldTot() const override {
		return 10000;
	}

	uint16_t getTBoostMin() const override {
		return 100;
	}

	uint16_t getTBoostMax() const override {
		return 400;
	}

	uint16_t getPumpTholdOff() const override {
		return 10;
	}

	uint16_t getPumpTholdTot() const override {
		return 10000;
	}

	// Print out an error message
	void onError(const char* why) override {
		// efiPrintf("PT2001 error: %s", why);
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

	palSetPadMode(GPIOB, 4, PAL_MODE_OUTPUT_PUSHPULL);	// DRVEN
	palClearPad(GPIOB, 4);

	palSetPadMode(GPIOB, 5, PAL_MODE_OUTPUT_PUSHPULL);	// reset
	palSetPad(GPIOB, 5);

	driver = &SPID1;
	spiStart(driver, &spiCfg);
	spiUnselect(driver);

	return restart();
}

Pt2001 chip;

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    // Fire up all of our threads

    InitPins();
    InitFlash();
    InitCan();
    InitUart();

	palSetPadMode(LED_BLUE_PORT, LED_BLUE_PIN, PAL_MODE_OUTPUT_PUSHPULL);

	bool isOverallHappyStatus = chip.init();

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
