#include "ch.h"
#include "hal.h"

#include "can.h"
#include "fault.h"
#include "uart.h"
#include "io_pins.h"
#include "can_common.h"
#include "persistence.h"

//#include "PT2000_LoadData.h"
//#include "pt2000_memory_map.h"
#include "pt2000impl.h"

static void InitPins() {
	// UART
    // stm32 TX - dongle RX often White
    palSetPadMode(UART_TX_PORT, UART_TX_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    // stm32 RX - dongle TX often Green
    palSetPadMode(UART_RX_PORT, UART_RX_PIN, PAL_MODE_INPUT_PULLUP );

    // CAN
    palSetPadMode(CAN_TX_PORT, CAN_TX_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL );
    palSetPadMode(CAN_RX_PORT, CAN_RX_PIN, PAL_MODE_INPUT_PULLUP );

	// GD32 errata, PB1 must have certain states for PB2 to work
	//palSetPadMode(GPIOB, 1, PAL_MODE_INPUT);
	// Set debug pins remap mode to use PB4 as normal pin, we need it for DRVEN
	AFIO->MAPR = AFIO_MAPR_SWJ_CFG_JTAGDISABLE;

}

static const SPIConfig spiCfg = {
    .circular = false,
    .end_cb = nullptr,
    .ssport = SPI_CS_PORT,
    .sspad = SPI_CS_PIN,
    .cr1 =
				SPI_CR1_DFF |	// 16-bit
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



bool Pt2000::init() {
	// SPI
	palSetPadMode(SPI_SCK_PORT, SPI_SCK_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(SPI_MISO_PORT, SPI_MISO_PIN, PAL_MODE_INPUT);
    palSetPadMode(SPI_MOSI_PORT, SPI_MOSI_PIN, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
	// PT Specifics
	palSetPadMode(SPI_CS_PORT, SPI_CS_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(PT_DRVEN_PORT, PT_DRVEN_PIN, PAL_MODE_OUTPUT_PUSHPULL);	
    palClearPad(PT_DRVEN_PORT, PT_DRVEN_PIN);
	palSetPadMode(PT_RESET_PORT, PT_RESET_PIN, PAL_MODE_OUTPUT_PUSHPULL);	
    palSetPadMode(PT_FLAG_0_PORT, PT_FLAG_0_PIN, PAL_MODE_INPUT);	
    //palSetPad(PT_FLAG_0_PORT, PT_FLAG_0_PIN);
    palSetPadMode(PT_OA_1_PORT, PT_OA_1_PIN, PAL_MODE_INPUT);	
    palSetPadMode(PT_OA_2_PORT, PT_OA_2_PIN, PAL_MODE_INPUT);	
    palSetPadMode(PT_OA_3_PORT, PT_OA_3_PIN, PAL_MODE_INPUT);	

	driver = &SPID1;
	spiStart(driver, &spiCfg);
	spiUnselect(driver);

	// Wait 1/2 second for things to wake up
	chThdSleepMilliseconds(500);

	return restart();
}


Pt2000 chip;


/*
 * Application entry point.
 */
int main() {
    halInit();
    chSysInit();

    // Fire up all of our threads
    InitPins();
    InitCan();
    InitUart();


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
}
