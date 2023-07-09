#include "pt2001impl.h"

#include "can.h"
#include "fault.h"
#include "uart.h"
#include "io_pins.h"
#include "can_common.h"

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
    updateCounter = 20;
    inputCanID = GDI4_BASE_ADDRESS + 0x10;
    outputCanID = GDI4_BASE_ADDRESS;

	BoostVoltage = 65;
	BoostCurrent = 13;
	TBoostMin = 100;
	TBoostMax = 400;

	PeakCurrent = 9.4f;
	TpeakDuration = 700; // 700us = 0.7ms
	TpeakOff = 10;
	Tbypass = 10;

	HoldCurrent = 3.7f;
	TholdOff = 60;
	THoldDuration = 10000; // 10000us = 10ms

    PumpPeakCurrent = 5;
    PumpHoldCurrent = 3;
	PumpTholdOff = 10;
    PumpTholdTot = 10000; // 10000us = 10ms
}

GDIConfiguration configuration;

GDIConfiguration *getConfiguration() {
    return &configuration;
}


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

mfs_error_t flashState;

/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    // Fire up all of our threads
    InitPins();
    flashState = InitFlash();

    ReadOrDefault();

    InitCan();
    InitUart();

	palSetPadMode(LED_BLUE_PORT, LED_BLUE_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(LED_BLUE_PORT, LED_BLUE_PIN);
	palSetPadMode(LED_GREEN_PORT, LED_GREEN_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(LED_GREEN_PORT, LED_GREEN_PIN);

    bool isOverallHappyStatus = false;

    // reminder that +12v is required for PT2001 to start
	isOverallHappyStatus = chip.init();

    while (true) {
        if (isOverallHappyStatus) {
            // happy board - green D21 blinking
            palTogglePad(LED_GREEN_PORT, LED_GREEN_PIN);
        } else {
            palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);
        }

        chThdSleepMilliseconds(100);
    }

    while (true) {
        //auto fault = GetCurrentFault();

        //palTogglePad(LED_BLUE_PORT, LED_BLUE_PIN);
        chThdSleepMilliseconds(100);
    }
}
