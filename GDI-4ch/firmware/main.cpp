#include "ch.h"
#include "hal.h"

#include "can.h"
#include "fault.h"
#include "uart.h"
#include "io_pins.h"

#include "mc33816_control.h"
#include "mc33816_data.h"
#include "mc33816_memory_map.h"

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



const int MC_CK = 6; // PLL x24 / CLK_DIV 4 = 6Mhz

const int MAX_SPI_MODE_A_TRANSFER_SIZE = 31;  //max size for register config transfer

enum {
	CODE_RAM1,
	CODE_RAM2,
	DATA_RAM
};
enum {
	REG_MAIN,
	REG_CH1,
	REG_CH2,
	REG_IO,
	REG_DIAG
};


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

auto driver = &SPID1;

// Read a single word in Data RAM
unsigned short mcReadDram(MC33816Mem addr) {
	unsigned short readValue;
	spiSelect(driver);
	// Select Channel command, Common Page
    spi_writew(0x7FE1);
    spi_writew(0x0004);
    // read (MSB=1) at data ram x9 (SCV_I_Hold), and 1 word
    spi_writew((0x8000 | addr << 5) + 1);
    readValue = recv_16bit_spi();

    spiUnselect(driver);
    return readValue;
}

// Update a single word in Data RAM
void mcUpdateDram(MC33816Mem addr, unsigned short data) {
	spiSelect(driver);
	// Select Channel command, Common Page
    spi_writew(0x7FE1);
    spi_writew(0x0004);
    // write (MSB=0) at data ram x9 (SCV_I_Hold), and 1 word
    spi_writew((addr << 5) + 1);
    spi_writew(data);

    spiUnselect(driver);
}

static short dacEquation(unsigned short current) {
	/*
	Current, given in mA->A
	I = (DAC_VALUE * V_DAC_LSB - V_DA_BIAS)/(G_DA_DIFF * R_SENSEx)
	DAC_VALUE = ((I*G_DA_DIFF * R_SENSEx) + V_DA_BIAS) /  V_DAC_LSB
	V_DAC_LSB is the DAC resolution = 9.77mv
	V_DA_BIAS = 250mV
	G_DA_DIFF = Gain: 5.79, 8.68, [12.53], 19.25
	R_SENSE = 10mOhm soldered on board
	*/
	return (short)(((current/1000.0f * 12.53f * 10) + 250.0f) / 9.77f);
}

static void setTimings() {

	// Convert mA to DAC values
	// mcUpdateDram(MC33816Mem::Iboost, dacEquation(engineConfiguration->mc33_i_boost));
	// mcUpdateDram(MC33816Mem::Ipeak, dacEquation(engineConfiguration->mc33_i_peak));
	// mcUpdateDram(MC33816Mem::Ihold, dacEquation(engineConfiguration->mc33_i_hold));

	// // uint16_t mc33_t_max_boost; // not yet implemented in microcode

	// // in micro seconds to clock cycles
	// mcUpdateDram(MC33816Mem::Tpeak_off, (MC_CK * engineConfiguration->mc33_t_peak_off));
	// mcUpdateDram(MC33816Mem::Tpeak_tot, (MC_CK * engineConfiguration->mc33_t_peak_tot));
	// mcUpdateDram(MC33816Mem::Tbypass, (MC_CK * engineConfiguration->mc33_t_bypass));
	// mcUpdateDram(MC33816Mem::Thold_off, (MC_CK * engineConfiguration->mc33_t_hold_off));
	// mcUpdateDram(MC33816Mem::Thold_tot, (MC_CK * engineConfiguration->mc33_t_hold_tot));

	// // HPFP solenoid settings
	// mcUpdateDram(MC33816Mem::HPFP_Ipeak,
	// 	     dacEquation(engineConfiguration->mc33_hpfp_i_peak * 1000));
	// mcUpdateDram(MC33816Mem::HPFP_Ihold,
	// 	     dacEquation(engineConfiguration->mc33_hpfp_i_hold * 1000));
	// mcUpdateDram(MC33816Mem::HPFP_Thold_off,
	// 	     std::min(MC_CK * engineConfiguration->mc33_hpfp_i_hold_off,
	// 		      UINT16_MAX));
	// // Note, if I'm reading this right, the use of the short and the given clock speed means
	// // the max time here is approx 10ms.
	// mcUpdateDram(MC33816Mem::HPFP_Thold_tot,
	// 	     std::min(MC_CK * 1000 * engineConfiguration->mc33_hpfp_max_hold,
	// 		      UINT16_MAX));
}

void setBoostVoltage(float volts)
{
	// Sanity checks, Datasheet says not too high, nor too low
	if (volts > 65.0f) {
		// firmwareError(OBD_PCM_Processor_Fault, "DI Boost voltage setpoint too high: %.1f", volts);
		return;
	}
	if (volts < 10.0f) {
		// firmwareError(OBD_PCM_Processor_Fault, "DI Boost voltage setpoint too low: %.1f", volts);
		return;
	}
	// There's a 1/32 divider on the input, then the DAC's output is 9.77mV per LSB.  (1 / 32) / 0.00977 = 3.199 counts per volt.
	unsigned short data = volts * 3.2;
	mcUpdateDram(MC33816Mem::Vboost_high, data+1);
	mcUpdateDram(MC33816Mem::Vboost_low, data /* -1 */);
	// Remember to strobe driven!!
}

static bool check_flash() {
	spiSelect(driver);

	// ch1
	// read (MSB=1) at location, and 1 word
    spi_writew((0x8000 | 0x100 << 5) + 1);
    if (!(recv_16bit_spi() & (1<<5))) {
    	spiUnselect(driver);
    	return false;
    }

    // ch2
	// read (MSB=1) at location, and 1 word
    spi_writew((0x8000 | 0x120 << 5) + 1);

    if (!(recv_16bit_spi() & (1<<5))) {
    	spiUnselect(driver);
    	return false;
    }

    spiUnselect(driver);
	return true;
}

static unsigned short readDriverStatus(){
	unsigned short driverStatus;
	setup_spi(); // ensure on common page?
	spiSelect(driver);
    	spi_writew((0x8000 | 0x1D2 << 5) + 1);
    	driverStatus = recv_16bit_spi();
	spiUnselect(driver);
	return driverStatus;
}

static bool checkUndervoltVccP(unsigned short driverStatus){
	return (driverStatus  & (1<<0));
}

static bool checkUndervoltV5(unsigned short driverStatus){
	return (driverStatus  & (1<<1));
}

static bool checkOverTemp(unsigned short driverStatus){
	return (driverStatus  & (1<<3));
}

static bool checkDrivenEnabled(unsigned short driverStatus){
	return (driverStatus  & (1<<4));
}

static void enable_flash() {
	spiSelect(driver);
    spi_writew(0x2001); //ch1
    spi_writew(0x0018); //enable flash
    spi_writew(0x2401); //ch2
    spi_writew(0x0018); // enable flash
    spiUnselect(driver);
}

static void download_RAM(int target) {
   uint16_t memory_area = 0;         // memory area
   uint16_t start_address = 0;      // start address
   uint16_t codeWidthRegAddr = 0;   // code width register address
   uint16_t size = 0;               // size of RAM data
   uint16_t command = 0;            // command data
   const uint16_t *RAM_ptr;               // pointer to array of data to be sent to the chip


   //Why Again? For Every time, just in case?
   setup_spi();

   switch(target)            // selects target
   {
   case CODE_RAM1:
      memory_area = 0x1;
      start_address = 0;
      codeWidthRegAddr = 0x107;
      RAM_ptr = MC33816_code_RAM1;
      size = sizeof(MC33816_code_RAM1) / 2;
      break;

   case CODE_RAM2:
      memory_area = 0x2;
      start_address = 0;
      codeWidthRegAddr = 0x127;
      RAM_ptr = MC33816_code_RAM2;
      size = sizeof(MC33816_code_RAM2) / 2;
      break;

   case DATA_RAM: // ch1 only?
      memory_area = 0x4;
      start_address = 0;
      RAM_ptr = MC33816_data_RAM;
      size = sizeof(MC33816_data_RAM) / 2;
      break;
// optional, both data_rams with 0x3, writes same code to both
   default:
      break;
   }

   // Chip-Select high
   spiSelect(driver);

   if (target != DATA_RAM)
   {
	   command = codeWidthRegAddr << 5;   // control width register address
	   command |= 1;                      // number of words to follow
	   spi_writew(command);           // sends code_width command
   	   spi_writew(size);              // sends size (Code Width)
   }

   // Select Channel command
   spi_writew(0x7FE1);
   // RAM1, RAM2, or Common Page (Data RAM)
   spi_writew(memory_area);

   // "Command" of starting address
   // up to 0x03FE of code ram
   // up to 0x0080 of data ram
   command = start_address << 5;
   spi_writew(command);           // sends start address command

   spiSend(driver, size, RAM_ptr);
   spiUnselect(driver);
}

static void download_register(int r_target) {
	   uint16_t r_start_address = 0;  // start address
	   uint16_t r_size = 0;           // size of configuration data
	   uint16_t r_command = 0;        // command data
	   uint16_t remainder_size = 0;   // remainder size
	   const uint16_t *reg_ptr;            // pointer to array of data to be sent to the chip

	   switch(r_target)                     // selects target
	   {
	   case REG_CH1:                              // channel 1 configurations
	      r_start_address = 0x100;
	      reg_ptr = MC33816_ch1_config;
	      r_size = sizeof(MC33816_ch1_config) / 2;  // gets number of words to be sent
	      break;

	   case REG_CH2:                              // channel 2 configurations
	      r_start_address = 0x120;
	      reg_ptr = MC33816_ch2_config;
	      r_size = sizeof(MC33816_ch2_config) / 2;  // gets number of words to be sent
	      break;

	   case REG_DIAG:                              // diagnostic configurations
	      r_start_address = 0x140;
	      reg_ptr = MC33816_diag_config;
	      r_size = sizeof(MC33816_diag_config) / 2; // gets number of words to be sent
	      break;

	   case REG_IO:                              // IO configurations
	      r_start_address = 0x180;
	      reg_ptr = MC33816_io_config;
	      r_size = sizeof(MC33816_io_config) / 2;   // gets number of words to be sent
	      break;

	   case REG_MAIN:                              // main configurations
	      r_start_address = 0x1C0;
	      reg_ptr = MC33816_main_config;
	      r_size = sizeof(MC33816_main_config) / 2; // gets number of words to be sent
	      break;

	   default:
	      break;
	   }

	   //for location < size(remainder?)
	   // is location == 0? or past max xfer, send command + expected size
	   // if location = max xfer
	   //
	   // retrieve data, send it, increase pointer
	   // increase

	   if (r_size > MAX_SPI_MODE_A_TRANSFER_SIZE)   //if size is too large, split into two sections ... MULTIPLE sections..
	   {
	      remainder_size = r_size - MAX_SPI_MODE_A_TRANSFER_SIZE;  // creates remaining size
	      r_size = MAX_SPI_MODE_A_TRANSFER_SIZE;                   // sets first size
	   }

	   r_command = r_start_address << 5;      // start address
	   r_command += r_size;                   // number of words to follow

	   spiSelect(driver);						// Chip

	   spi_writew(r_command);             // sends address and number of words to be sent

	   spiSend(driver, r_size, reg_ptr);

	   if (remainder_size > 0)                 // if remainder size is greater than 0, download the rest
	   {
	      r_start_address += r_size;          // new start address
	      r_command = r_start_address << 5;   // start address
	      r_command += remainder_size;        // number of words to follow

	      spi_writew(r_command);          // sends address and number of words to be sent
	      spiSend(driver, remainder_size, reg_ptr + r_size);
	   }
	   spiUnselect(driver);
}

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

    spiStart(driver, &spiCfg);
    spiUnselect(driver);

    palClearPad(GPIOB, 5);  // reset
    chThdSleepMilliseconds(10);
    palSetPad(GPIOB, 5);    // take out of reset
    chThdSleepMilliseconds(10);

    setup_spi();

    mcClearDriverStatus(); // Initial clear necessary
    auto mcDriverStatus = readDriverStatus();
    if (checkUndervoltV5(mcDriverStatus)) {
        // TODO
    }

    download_RAM(CODE_RAM1);        // transfers code RAM1
    download_RAM(CODE_RAM2);        // transfers code RAM2
    download_RAM(DATA_RAM);         // transfers data RAM
    /**
     * current configuration of REG_MAIN would toggle flag0 from LOW to HIGH
     */
    download_register(REG_MAIN);    // download main register configurations

    download_register(REG_CH1);     // download channel 1 register configurations
    download_register(REG_CH2);     // download channel 2 register configurations
    download_register(REG_IO);      // download IO register configurations
    download_register(REG_DIAG);    // download diag register configuration

    setTimings();

    // Finished downloading, let's run the code
    enable_flash();

	// Set boost voltage
    setBoostVoltage(65);

    // TURN ON THE BOOST CONVERTER!
    palSetPad(GPIOB, 4);

    while (true) {
        auto id = readId();

        palSetPadMode(LED_BLUE_PORT, LED_BLUE_PIN, PAL_MODE_OUTPUT_PUSHPULL);

        if ((id >> 8) == 0x9D) {
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
