/*
 * @file pt2001.cpp
 *
 * The NXP PT2001 is a programmable gate driver IC for precision solenoid control applications.
 *
 * Useful wires:
 * 5v, 3(3.3v), Gnd, 12v, VccIO(3v) SPI, DRVEN, RSTB
 *
 * For MC33816 vs PT2000 differences see
 * https://www.nxp.com/docs/en/application-note/AN5203.pdf
 *
 * @date May 3, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 * @author Matthew Kennedy
 */

#include "pt2000.h"
//#include <rusefi/arrays.h>

#include "PT2000_LoadData.h"

const int MC_CK = 6; // PLL x24 / CLK_DIV 4 = 6Mhz

const int MAX_SPI_MODE_A_TRANSFER_SIZE = 31;  //max size for register config transfer

enum {
	CODE_RAM1,
	CODE_RAM2,
	CODE_RAM3,
	DATA_RAM
};
enum {
	REG_MAIN,
	REG_CH1,
	REG_CH2,
	REG_CH3,
	REG_IO,
	REG_DIAG
};

static bool validateChipId(uint16_t id) {
	return (id >> 8) == 0xAE;
}

void Pt2000Base::setupSpi() {
	select();
	// Select Channel command
	send(0x7FE1); // 0x3FF << 5
	// Common Page
	send(0x0100);

	// Configure SPI command
	send(0x1A9 << 5 | 1); // spi_config
	// Mode A + Watchdog timer full
	//send(0x001F);
	send(0x009F); // + fast slew rate on miso
	deselect();
}

uint16_t Pt2000Base::readReg(int reg) {
	setupSpi(); // ensure on common page?
	select();
	send((0x8000 | reg << 5) + 1);
	uint16_t ret = recv();
	deselect();
	return ret;
}

void Pt2000Base::writeReg(int reg, uint16_t data) {
	setupSpi(); // ensure on common page?
	select();
	send((reg << 5) + 1);
    send(data);
	deselect();
}

uint16_t Pt2000Base::readId() {
	/*
	select();
	send(0xBAA1);
	uint16_t ID = recv();
	deselect();
	return ID;
	*/
	return readReg(0x1B6);
}

// Read a single word in Data RAM
uint16_t Pt2000Base::readDram(MC33PT2000Mem addr) {
	uint16_t addrInt = static_cast<uint16_t>(addr);

	select();
	// Select Channel command, Common Page
	send(0x7FE1); // 0x3FF << 5
	send(0x0100);
	// read (MSB=1) at addr, and 1 word
	send((0x8000 | addrInt << 5) + 1);
	uint16_t readValue = recv();

	deselect();
	return readValue;
}

// Update a single word in Data RAM
void Pt2000Base::writeDram(MC33PT2000Mem addr, uint16_t data) {
	uint16_t addrInt = static_cast<uint16_t>(addr);

	select();
	// Select Channel command, Common Page
	send(0x7FE1); // 0x3FF << 5
	send(0x0100);
	// write (MSB=0) at addr, and 1 word
	send((addrInt << 5) + 1);
	send(data);

	deselect();

	// DRAM2 is duplicated from DRAM1 for all timings
	if(addr < 64)
		writeDram((MC33PT2000Mem)(addr + 64), data);

}

static uint16_t dacEquation(volatile float current) {
	/*
		Current, given in A
		I = (DAC_VALUE * V_DAC_LSB - V_DA_BIAS)/(G_DA_DIFF * R_SENSEx)
		DAC_VALUE = ((I*G_DA_DIFF * R_SENSEx) + V_DA_BIAS) /  V_DAC_LSB
		V_DAC_LSB is the DAC resolution = 9.77mv
		V_DA_BIAS = 250mV
		G_DA_DIFF = Gain: 5.79, [8.68], 12.53, 19.25
		R_SENSE = 10mOhm soldered on board
	*/
	return ((current * 8.68f * 10) + 250.0f) / 9.77f;
}

void Pt2000Base::setTimings() {
	setBoostVoltage(getBoostVoltage());

	// Convert mA to DAC values
	
	writeDram(MC33PT2000Mem::Iboost, dacEquation(getBoostCurrent()));
	writeDram(MC33PT2000Mem::Ipeak, dacEquation(getPeakCurrent()));
	writeDram(MC33PT2000Mem::Ihold, dacEquation(getHoldCurrent()));

	// in micro seconds to clock cycles
	writeDram(MC33PT2000Mem::Tpeak_off, (MC_CK * getTpeakOff()));
	writeDram(MC33PT2000Mem::Tpeak_tot, (MC_CK * getTpeakTot()));
	writeDram(MC33PT2000Mem::Tbypass, (MC_CK * getTbypass()));
	writeDram(MC33PT2000Mem::Thold_off, (MC_CK * getTholdOff()));
	writeDram(MC33PT2000Mem::Thold_tot, (MC_CK * getTHoldTot()));
	writeDram(MC33PT2000Mem::Tboost_min, (MC_CK * getTBoostMin()));
	writeDram(MC33PT2000Mem::Tboost_max, (MC_CK * getTBoostMax()));

	// HPFP solenoid settings
	writeDram(MC33PT2000Mem::HPFP_Ipeak, dacEquation(getPumpPeakCurrent()));
	writeDram(MC33PT2000Mem::HPFP_Ihold, dacEquation(getPumpHoldCurrent()));
	writeDram(MC33PT2000Mem::HPFP_Thold_off, MC_CK * getPumpTholdOff());
	writeDram(MC33PT2000Mem::HPFP_Thold_tot, MC_CK * getPumpTholdTot());
	
}

void Pt2000Base::setBoostVoltage(float volts) {
	// Sanity checks, Datasheet says not too high, nor too low
	if (volts > 65.0f) {
		onError("DI Boost voltage setpoint too high");
		return;
	}
	if (volts < 12.0f) {
		onError("DI Boost voltage setpoint too low");
		return;
	}

	// There's a 1/32 divider on the input, then the DAC's output is 9.77mV per LSB.  (1 / 32) / 0.00977 = 3.199 counts per volt.
	// .. more accurately 3.25 * volts + 1.584
	uint16_t data = (volts * 3.25f) + 1.584f;
	// 3.125 according to PT2000 datasheet
	//unsigned short data = volts / 0.3125f;

	writeDram(MC33PT2000Mem::Vboost_high, data+1);
	writeDram(MC33PT2000Mem::Vboost_low, data-1);
	// Remember to strobe driven!! ( this is an older statement, and reloading should occur every time the dc-dc will turn on)
}

bool Pt2000Base::checkFlash() {
	select();
	uint16_t status;
	// ch1
	// read (MSB=1) at location, and 1 word
    status = readReg(0x100);
    if (!(status & (1<<5))) {
    	deselect();
    	return false;
    }

    // ch2
	// read (MSB=1) at location, and 1 word
    status = readReg(0x120);
    if (!(status & (1<<5))) {
    	deselect();
    	return false;
    }

    // ch3
	// read (MSB=1) at location, and 1 word
    status = readReg(0x140);
    if (!(status & (1<<5))) {
    	deselect();
    	return false;
    }
    deselect();
	return true;
}

void Pt2000Base::clearDriverStatus(){
	// Note: There is a config at 0x1CE(for pt2001) & 1 that can reset this status config register on read
	// otherwise the reload/recheck occurs with this write
	// resetting it is necessary to clear default reset behavoir, as well as if an issue has been resolved
	setupSpi(); // ensure on common page?
	select();
	send((0x0000 | 0x1B2 << 5) + 1); // write, location, one word
	send(0x0000); // anything to clear
	deselect();
}

uint16_t Pt2000Base::readStatus(int reg) {
	setupSpi(); // ensure on common page?
	select();
	send((0x8000 | reg << 5) + 1);
	uint16_t driverStatus = recv();
	deselect();
	return driverStatus;
}

uint16_t Pt2000Base::readDriverStatus() {
	return readStatus(0x1B2);
}

static bool checkUndervoltVccP(uint16_t driverStatus){
	return (driverStatus  & (1<<0));
}

static bool checkUndervoltV5(uint16_t driverStatus){
	return (driverStatus  & (1<<1));
}

// static bool checkOverTemp(uint16_t driverStatus){
// 	return (driverStatus  & (1<<3));
// }

static bool checkDrivenEnabled(uint16_t driverStatus){
	return (driverStatus  & (1<<4));
}

void Pt2000Base::enableFlash() {
	select();
	writeReg(0x100, 0x0018); //ch1
	writeReg(0x120, 0x0018); //ch2
	writeReg(0x140, 0x0018); //ch3
	deselect();
}


//static void disable_flash() {
	//spiSelect(driver);
	//writeReg(0x140, 0x0000);
    //spiUnselect(driver);
//}

void Pt2000Base::downloadRam(int target) {
   uint16_t memory_area = 0;         // memory area
   uint16_t start_address = 0;      // start address
   uint16_t codeWidthRegAddr = 0;   // code width register address
   uint16_t size = 0;               // size of RAM data
   uint16_t command = 0;            // command data
   const uint16_t *RAM_ptr;               // pointer to array of data to be sent to the chip


   //Why Again? For Every time, just in case?
   setupSpi();

   switch(target)            // selects target
   {
   case CODE_RAM1:
      memory_area = 0x1;
      start_address = 0;
      codeWidthRegAddr = 0x107;
      RAM_ptr = PT2000_code_RAM1;
      size = sizeof(PT2000_code_RAM1) / 2;
      break;

   case CODE_RAM2:
      memory_area = 0x2;
      start_address = 0;
      codeWidthRegAddr = 0x127;
      RAM_ptr = PT2000_code_RAM2;
      size = sizeof(PT2000_code_RAM2) / 2;
      break;

   case CODE_RAM3:
      memory_area = 0x4;
      start_address = 0;
      codeWidthRegAddr = 0x147;
      RAM_ptr = PT2000_code_RAM3;
      size = sizeof(PT2000_code_RAM3) / 2;
      break;

   case DATA_RAM: // ch1 only?
      memory_area = 0x100;
      start_address = 0;
      RAM_ptr = PT2000_data_RAM;
      size = sizeof(PT2000_data_RAM) / 2;
      break;
// optional, both data_rams with 0x3, writes same code to both, or 0x7 to all three
   default:
      break;
   }

   // Chip-Select high
   select();

   if (target != DATA_RAM)
   {
	   command = codeWidthRegAddr << 5;   // control width register address
	   command |= 1;                      // number of words to follow
	   send(command);           // sends code_width command
   	   send(size);              // sends size (Code Width)
   }

   // Select Channel command
   send(0x7FE1);
   // CRAM1, CRAM2, CRAM3, or Common Page (Data RAM)
   send(memory_area);

   // "Command" of starting address
   // up to 0x03FE of code ram
   // up to 0x0080 of data ram
   command = start_address << 5;
   send(command);           // sends start address command

   sendLarge(RAM_ptr, size);
   deselect();
}

void Pt2000Base::downloadRegister(int r_target)
{
	uint16_t r_start_address = 0; // start address
	uint16_t r_size = 0;		  // size of configuration data
	uint16_t r_command = 0;		  // command data
	uint16_t remainder_size = 0;  // remainder size
	const uint16_t *reg_ptr;	  // pointer to array of data to be sent to the chip

	switch (r_target) // selects target
	{
	case REG_CH1: // channel 1 configurations
		r_start_address = 0x100;
		reg_ptr = PT2000_ch1_config;
		r_size = sizeof(PT2000_ch1_config) / 2; // gets number of words to be sent
		break;

	case REG_CH2: // channel 2 configurations
		r_start_address = 0x120;
		reg_ptr = PT2000_ch2_config;
		r_size = sizeof(PT2000_ch2_config) / 2; // gets number of words to be sent
		break;

	case REG_CH3: // channel 3 configurations
		r_start_address = 0x140;
		reg_ptr = PT2000_ch3_config;
		r_size = sizeof(PT2000_ch3_config) / 2; // gets number of words to be sent
		break;

	case REG_DIAG: // diagnostic configurations
		r_start_address = 0x1C0;
		reg_ptr = PT2000_diag_config;
		r_size = sizeof(PT2000_diag_config) / 2; // gets number of words to be sent
		break;

	case REG_IO: // IO configurations
		r_start_address = 0x154;
		reg_ptr = PT2000_io_config;
		r_size = sizeof(PT2000_io_config) / 2; // gets number of words to be sent
		break;

	case REG_MAIN: // main configurations
		r_start_address = 0x1A0;
		reg_ptr = PT2000_main_config;
		r_size = sizeof(PT2000_main_config) / 2; // gets number of words to be sent
		break;

	default:
		break;
	}

	// for location < size(remainder?)
	//  is location == 0? or past max xfer, send command + expected size
	//  if location = max xfer
	//
	//  retrieve data, send it, increase pointer
	//  increase

	do
	{
		if (r_size > MAX_SPI_MODE_A_TRANSFER_SIZE) // if size is too large, split into two sections ... MULTIPLE sections..
		{
			remainder_size = r_size - MAX_SPI_MODE_A_TRANSFER_SIZE; // creates remaining size
			r_size = MAX_SPI_MODE_A_TRANSFER_SIZE;					// sets first size
		}
		else
		{
			remainder_size = 0;
		}

		r_command = r_start_address << 5; // start address
		r_command += r_size;			  // number of words to follow

		select(); // Chip

		send(r_command); // sends address and number of words to be sent

		sendLarge(reg_ptr, r_size);

		// if (remainder_size > 0) // if remainder size is greater than 0, download the rest
		{
			reg_ptr += r_size;
			r_start_address += r_size; // new start address
			r_size = remainder_size;
		}
	} while (remainder_size > 0);
	deselect();
}

void Pt2000Base::shutdown() {
	setDriveEN(false); // ensure HV is off
	setResetB(false);  // turn off the chip
}

bool Pt2000Base::restart() {
	bool flag0before = false;
	bool flag0after = false;

	// Start with everything off
	shutdown();
	deselect();

	if (getVbatt() < 8) {
		onError("GDI not Restarting until we see VBatt");
		return false;
	}

	// Wait for chip to reset, then release reset and wait again
	sleepMs(1);
	setResetB(true);
	sleepMs(1);

	// Flag0 should be floating - pulldown means it should read low
	flag0before = readFlag0();

	setupSpi();

	clearDriverStatus(); // Initial clear necessary
	status = readDriverStatus();
	if (checkUndervoltV5(status)) {
		onError(McFault::UnderVoltage5);
		shutdown();
		return false;
	}

	uint16_t chipId = readId();
	if (!validateChipId(chipId)) {
		onError(McFault::NoComm);
		shutdown();
		return false;
	}

	downloadRam(CODE_RAM1);        // transfers code RAM1
	downloadRam(CODE_RAM2);        // transfers code RAM2
	downloadRam(CODE_RAM3);        // transfers code RAM3
	downloadRam(DATA_RAM);         // transfers data RAM
	downloadRegister(REG_MAIN);    // download main register configurations

	// current configuration of REG_MAIN would toggle flag0 from LOW to HIGH
	flag0after = readFlag0();
	if (flag0before || !flag0after) {
		onError(McFault::flag0);
		shutdown();
		return false;
	}

	downloadRegister(REG_CH1);     // download channel 1 register configurations
	downloadRegister(REG_CH2);     // download channel 2 register configurations
	downloadRegister(REG_CH3);     // download channel 3 register configurations
	downloadRegister(REG_IO);      // download IO register configurations
	downloadRegister(REG_DIAG);    // download diag register configuration

	//setTimings();

	// Finished downloading, let's run the code
	enableFlash();

	// give it a moment to take effect
	sleepMs(10);

	if (!checkFlash()) {
		onError(McFault::NoFlash);
		shutdown();
		return false;
	}

    clearDriverStatus();
    sleepMs(5);

	status = readDriverStatus();
	if (checkUndervoltVccP(status)) {
		onError(McFault::UnderVoltage7);
		shutdown();
		return false;
	}

	// Drive High Voltage
	setDriveEN(true); // driven = HV
	sleepMs(10); // Give it a moment
	status = readDriverStatus();
	if (!checkDrivenEnabled(status)) {
		onError(McFault::Driven);
		shutdown();
		return false;
	}
status = readStatus(0x186);

	status = readDriverStatus();
	if (checkUndervoltVccP(status)) {
		onError(McFault::UnderVoltageAfter); // Likely DC-DC LS is dead!
		shutdown();
		return false;
	}
	return true;
}
