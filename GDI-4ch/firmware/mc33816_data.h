/*
 * mc33816_data.h
 *
 * @date May 3, 2019
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

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

extern unsigned short PT2001_code_RAM1[108];    // CODE RAM CH 1
extern unsigned short PT2001_code_RAM2[43];    // CODE RAM CH 2
extern unsigned short PT2001_data_RAM[128];    // DATA RAM
extern unsigned short PT2001_main_config[29];  // main configurations
extern unsigned short PT2001_ch1_config[19];   // CH 1 configurations
extern unsigned short PT2001_ch2_config[19];   // CH 2 configurations
extern unsigned short PT2001_io_config[53];    // IO configurations
extern unsigned short PT2001_diag_config[44];  // diag configurations

