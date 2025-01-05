/*******************************************************************************
* Example Code
*
* Copyright(C) 2025 NXP Semiconductors
* NXP Semiconductors Confidential and Proprietary
*
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* NXP products.  This software is supplied "AS IS" without any warranties
* of any kind, and NXP Semiconductors and its licensor disclaim any and
* all warranties, express or implied, including all implied warranties of
* merchantability, fitness for a particular purpose and non-infringement of
* intellectual property rights.  NXP Semiconductors assumes no responsibility
* or liability for the use of the software, conveys no license or rights
* under any patent, copyright, mask work right, or any other intellectual
* property rights in or to any products. NXP Semiconductors reserves the
* right to make changes in the software without notification. NXP
* Semiconductors also makes no representation or warranty that such
* application will be suitable for the specified use without further testing
* or modification.
*
* IN NO EVENT WILL NXP SEMICONDUCTORS BE LIABLE, WHETHER IN CONTRACT, 
* TORT, OR OTHERWISE, FOR ANY INCIDENTAL, SPECIAL, INDIRECT, CONSEQUENTIAL 
* OR PUNITIVE DAMAGES, INCLUDING, BUT NOT LIMITED TO, DAMAGES FOR ANY 
* LOSS OF USE, LOSS OF TIME, INCONVENIENCE, COMMERCIAL LOSS, OR LOST 
* PROFITS, SAVINGS, OR REVENUES, TO THE FULL EXTENT SUCH MAY BE DISCLAIMED  
* BY LAW. NXP SEMICONDUCTOR’S TOTAL LIABILITY FOR ALL COSTS, DAMAGES, 
* CLAIMS, OR LOSSES WHATSOEVER ARISING OUT OF OR IN CONNECTION WITH THE 
* SOFTWARE IS LIMITED TO THE AGGREGATE AMOUNT PAID BY YOU TO NXP SEMICONDUCTORS
* IN CONNECTION WITH THE SOFTWARE TO WHICH LOSSES OR DAMAGES ARE CLAIMED.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors' and its
* licensor's relevant copyrights in the software, without fee, provided
* that it is used in conjunction with NXP Semiconductors devices.  This
* copyright, permission, and disclaimer notice must appear in all copies
* of this code.
*******************************************************************************/

#include "stddef.h"
#include "system.h"
#include "PT2000.h"
#include "PT2000_spi_map.h"
#include "PT2000_irq.h"
#include "lpspi.h"

const int MAX_SPI_MODE_A_TRANSFER_SIZE = 31;  // max size for register config transfer

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : send_PT2000_SPI_Cmd
// Description     : Sends a SPI command using the PT2000 SPI protocol
// Return type     : bool - true = send was successful, false = send failed
// Argument        : bool bRead - true for a read command, false for a write command
// Argument        : unsigned short offset - Start address for sending data
// Argument        : unsigned short length - Number of 16 bit words to be sent
// Argument        : unsigned short* pTxData - Buffer for transmit data
// Argument        : unsigned short* pRxData - Buffer for receive data
/////////////////////////////////////////////////////////////////////////////////////////////////
bool send_PT2000_SPI_Cmd(bool bRead, unsigned short offset, unsigned short length, unsigned short* pTxData, unsigned short* pRxData)
{
    int i = 0;
    unsigned short command = 0;

    // Return false if the length is out of range
    if ((length < 1) || (length > 31)) { return false; }

    // Return false if either of the data arrays are not allocated
    if ((pTxData == NULL) || (pRxData == NULL)) { return false; }

    // If the start address is out of range, return false
    if (offset > 1031) { return false; }

    // Set the read bit if specified
    if (bRead == true)
    {
        command |= 0x8000;
    }

    // Set the command address and length
    command |= offset << 5;
    command |= length;

    // Send the command word
    send_16bit_SPI(command);

    // Send the data word(s)
    for (i = 0; i < length; i++)
    {
        *(pRxData++) = send_16bit_SPI(*(pTxData++));
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : send_single_PT2000_SPI_Cmd
// Description     : Sends a SPI command using the PT2000 SPI protocol
// Return type     : unsigned short - Data read from the SPI interface
// Argument        : bool bRead - READ for a read command, WRITE for a write command
// Argument        : unsigned short offset - Start address for sending data
// Argument        : unsigned short txData - Transmit data
/////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short send_single_PT2000_SPI_Cmd(bool bRead, unsigned short offset, unsigned short txData)
{
    const unsigned short length = 1;
    unsigned short rxData = 0;
    unsigned short command = 0;

    // If the start address is out of range, return false
    if (offset > 1031) { return 0; }

    // Set the read bit if specified
    if (bRead == true)
    {
        command |= 0x8000;
    }

    // Set the command address and length
    command |= offset << 5;
    command |= length;

    // Send the command word
    send_16bit_SPI(command);

    // Send the data word
    rxData = send_16bit_SPI(txData);

    return rxData;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : ProgramDevice
// Description     : Programs the PT2000 code RAM, data RAM and registers.
// Return type     : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void ProgramDevice()
{
    download_register(MAIN_REG);    // download main register configurations
    download_RAM(CODE_RAM1);        // transfers code RAM1
    download_RAM(CODE_RAM2);        // transfers code RAM2
    download_RAM(CODE_RAM3);        // transfers code RAM3
    download_RAM(DATA_RAM);         // transfers data RAM

    download_register(IO_REG);      // download IO register configurations
    download_register(DIAG_REG);    // download diag register configurations
    download_register(CH1_REG);     // download channel 1 register configurations
    download_register(CH2_REG);     // download channel 2 register configurations
    download_register(CH3_REG);     // download channel 3 register configurations
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : download_RAM
// Description     : Downloads code RAM or data RAM via SPI.
// Return type     : void
// Argument        : int target
/////////////////////////////////////////////////////////////////////////////////////////////////
void download_RAM(int target)
{
    unsigned short memory_area = 0;        // memory area
    unsigned short start_address = 0;      // start address
    unsigned short codeWidthRegAddr = 0;   // code width register address
    unsigned short size = 0;               // size of RAM data
    unsigned short command = 0;            // command data
    unsigned short data = 0;               // RAM data
    unsigned int k = 0;                    // used in loop for writing RAM data to the chip
    unsigned short * RAM_ptr = NULL;       // pointer to array of data to be sent to the chip

    // Select common page memory area
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x0100);

    // Set the maximum watchdog timeout
    send_single_PT2000_SPI_Cmd(WRITE, main_SPI_config, 0x1F);

    // Select target
    switch (target)
    {
    case CODE_RAM1:
        memory_area = 0x1;
        start_address = 0;
        codeWidthRegAddr = ch1_code_width;
        RAM_ptr = PT2000_code_RAM1;
        size = sizeof(PT2000_code_RAM1) / 2;   // gets number of words to be sent
        break;

    case CODE_RAM2:
        memory_area = 0x2;
        start_address = 0;
        codeWidthRegAddr = ch2_code_width;
        RAM_ptr = PT2000_code_RAM2;
        size = sizeof(PT2000_code_RAM2) / 2;   // gets number of words to be sent
        break;

    case CODE_RAM3:
        memory_area = 0x4;
        start_address = 0;
        codeWidthRegAddr = ch3_code_width;
        RAM_ptr = PT2000_code_RAM3;
        size = sizeof(PT2000_code_RAM3) / 2;   // gets number of words to be sent
        break;

    case DATA_RAM:
        memory_area = 0x0100;
        start_address = 0;
        RAM_ptr = PT2000_data_RAM;
        size = sizeof(PT2000_data_RAM) / 2;    // gets number of words to be sent
        break;

    default:
        break;
    }

    // Set the code width register only if code RAM is being downloaded
    if (target != DATA_RAM)
    {
        // Set the code width register with the size of the download
        send_single_PT2000_SPI_Cmd(WRITE, codeWidthRegAddr, size);
    }

    // Send command for memory area selection
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, memory_area);

    command = start_address << 5;
    send_16bit_SPI(command);              // sends start address command

    for (k = 0; k < size; k++)            // downloads RAM
    {
        data = *RAM_ptr;                  // retrieves data to be sent
        send_16bit_SPI(data);             // sends data
        RAM_ptr++;
    }
} //end download_RAM

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : download_register
// Description     : Downloads register configurations via SPI.
// Return type     : void
// Argument        : int r_target
/////////////////////////////////////////////////////////////////////////////////////////////////
void download_register(int r_target)
{
    int n = 0;                           // used for loop for writing data to the chip
    unsigned short r_start_address = 0;  // start address
    unsigned short r_size = 0;           // size of configuration data
    unsigned short r_command = 0;        // command data
    unsigned short r_data = 0;           // configuration data
    unsigned short remainder_size = 0;   // remainder size
    unsigned short *reg_ptr = NULL;      // pointer to array of data to be sent to the chip

    switch (r_target)                    // selects target
    {
    case CH1_REG:                                // channel 1 configurations
        r_start_address = ch1_flash_enable;
        reg_ptr = PT2000_ch1_config;
        r_size = sizeof(PT2000_ch1_config) / 2;  // number of words to be sent
        break;

    case CH2_REG:                                // channel 2 configurations
        r_start_address = ch2_flash_enable;
        reg_ptr = PT2000_ch2_config;
        r_size = sizeof(PT2000_ch2_config) / 2;  // number of words to be sent
        break;

    case CH3_REG:                                // channel 3 configurations
        r_start_address = ch3_flash_enable;
        reg_ptr = PT2000_ch3_config;
        r_size = sizeof(PT2000_ch3_config) / 2;  // number of words to be sent
        break;

    case DIAG_REG:                               // diagnostic configurations
        r_start_address = diag_ls1_diag_config1;
        reg_ptr = PT2000_diag_config;
        r_size = sizeof(PT2000_diag_config) / 2; // number of words to be sent
        break;

    case IO_REG:                                 // IO configurations
        r_start_address = io_fbk_sens_uc0_ch1_part1;
        reg_ptr = PT2000_io_config;
        r_size = sizeof(PT2000_io_config) / 2;   // number of words to be sent
        break;

    case MAIN_REG:                               // main configurations
        r_start_address = main_Clock_Prescaler;
        reg_ptr = PT2000_main_config;
        r_size = sizeof(PT2000_main_config) / 2; // number of words to be sent
        break;

    default:
        break;
    }

    // Select common page memory area
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x0100);

    // Set the maximum watchdog timeout
    send_single_PT2000_SPI_Cmd(WRITE, main_SPI_config, 0x1F);

    do
    {
        // If the size of the transfer is greater than the maximum SPI burst
        // transfer size, set the size to the maximum SPI burst size and store
        // the remainder size for use in the next SPI transfer
        if (r_size > MAX_SPI_MODE_A_TRANSFER_SIZE)
        {
            remainder_size = r_size - MAX_SPI_MODE_A_TRANSFER_SIZE;
            r_size = MAX_SPI_MODE_A_TRANSFER_SIZE;
        }
        else
        {
            // The size is <= MAX_SPI_MODE_A_TRANSFER_SIZE
            // so there is no remaining data to be sent
            remainder_size = 0;
        }

        // Build the control word to be sent
        r_command = r_start_address << 5;
        r_command += r_size;

        // Write a control word indicating the address and the number of words to be sent
        send_16bit_SPI(r_command);

        for (n = 0; n < r_size; n++)
        {
            // If this is the first register for ch1, ch2, or ch3 send 0 so the flash and dual
            // microcores are not enabled
            if( (n == 0) && ((r_target == CH1_REG) || (r_target == CH2_REG) || (r_target == CH3_REG)) )
            {
                r_data = 0;
            }
            else
            {
                r_data = *reg_ptr;          // set data to be sent
            }
            send_16bit_SPI(r_data);         // send data
            reg_ptr++;                      // update data pointer
        }

        r_start_address += r_size;         // update the start address
        r_size = remainder_size;           // set size to the remaining amount of data
    } while (remainder_size > 0);


    // For channels 1, 2, & 3 program the flash enable register using the the
    // first entry in the channel's configuration table
    switch (r_target)
    {
    case CH1_REG:
        // Program the ch1 flash enable register
        send_single_PT2000_SPI_Cmd(WRITE, ch1_flash_enable, PT2000_ch1_config[0]);
        break;

    case CH2_REG:
        // Program the ch2 flash enable register
        send_single_PT2000_SPI_Cmd(WRITE, ch2_flash_enable, PT2000_ch2_config[0]);
        break;

    case CH3_REG:
        // Program the ch3 flash enable register
        send_single_PT2000_SPI_Cmd(WRITE, ch3_flash_enable, PT2000_ch3_config[0]);
        break;

    default:
        break;
    }

} // end download_register

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : CLK_check
// Description     : Check if the 1MHz CLK is connected to PT200x
// Return type     : bool: 1 if CLK is present and 0 if no CLK detected (run on backup CLK)
/////////////////////////////////////////////////////////////////////////////////////////////////
bool CLK_check()
{
    unsigned short cksys_missing;
    bool CLK_results = true;

    // Check CLK 1MHz
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page
    cksys_missing = send_single_PT2000_SPI_Cmd(READ, main_Backup_Clock_Status, 0x00);   // Read backup_clock_status 1A8h

    if ((cksys_missing & 0x1) != 0) { CLK_results = false; } // If 1MHz CLK not there set results to 0

    return CLK_results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : Driver_Status_Init
// Description     : Read Driver status register to make sure no UV or OT present
// Return type     : bool: 1 if no issue, 0 if issue
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Driver_Status_Init ()
{
    unsigned short rxData = 0;
    bool Driver_Status_results;

    Driver_Status_results = 1;
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page
    send_single_PT2000_SPI_Cmd(READ, 0x1B2, 0); // Read driver status register
    rxData = send_single_PT2000_SPI_Cmd(READ, main_Driver_Status, 0); // Read driver status second time to past faults

    if  ( (rxData & 0x4F) != 0)   { Driver_Status_results = 0; } // Error if different than 0

    return  Driver_Status_results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : DRVEN_check
// Description     : Check if the DRVEN SPI monitoring works and if MCU can drive it
// Return type     : bool: 1 is DRVEN test pass and 0 if it fails
/////////////////////////////////////////////////////////////////////////////////////////////////
bool DRVEN_check()
{
    //DRVEN safe state test
    bool DRVEN_results;
    DRVEN_results = 1;
    unsigned short rxData = 0;

    SET_DRVEN_LOW;
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page

    // Check if DRVEN monitoring works using SPI register 1B2h
    rxData = send_single_PT2000_SPI_Cmd(READ, main_Driver_Status, 0x00);

    // If bit DrvEn value is not 0, exit loop and do nothing since it will not be safe
    if ((rxData & 0x10) != 0) { DRVEN_results=0; }

    // Sets drive enable signal high (enables outputs and vboost)
    SET_DRVEN_HIGH;

    // Check if DRVEN monitoring works using SPI register 1B2h
    rxData = send_single_PT2000_SPI_Cmd(READ, main_Driver_Status, 0x00);

    // If bit DrvEn value is not 1, exit loop and do nothing since it will not be safe
    if ((rxData & 0x10) != 0x10) { DRVEN_results=0; }

    SET_DRVEN_LOW;

    return DRVEN_results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : BIST_check
// Description     : make sure memory and logic works properly
// Return type     : bool: 1 if BIST OK and 0 if BIST failed
/////////////////////////////////////////////////////////////////////////////////////////////////
bool BIST_check(_Bool MBIST_run, _Bool LBIST_run)
{
    unsigned short rxData = 0;
    bool MBIST_results=1;
    bool LBIST_results=1;
    bool BIST_results=0;

    if(MBIST_run ==1)
    {
        // Run MBIST
        rxData = 1;
        send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page
        send_single_PT2000_SPI_Cmd(WRITE, main_BIST_interface, 0xB157);
        delay100us(40); // wait 4 ms until MBIST complete

        while (rxData == 0x1)
        {
            rxData = send_single_PT2000_SPI_Cmd(READ, main_BIST_interface, 0);
            delay100us(1);
        }

        if (rxData == 0x2)
        {
            MBIST_results=1;
        }
        else
        {
            MBIST_results=0;
        }
    }

    if(LBIST_run ==1)
    {
        // Run LBIST
        rxData = 0;
        send_single_PT2000_SPI_Cmd(WRITE, main_BIST_interface, 0x666);
        delay100us(350); // wait 35ms (24MHz PLL used) until LBIST complete
        send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page since LBIST removed it
        rxData = send_single_PT2000_SPI_Cmd(READ, main_BIST_interface, 0); // LBIST results

        if (rxData == 0x8)
        {
            LBIST_results=1;
            // Clear LBIST to reenable logic
            send_single_PT2000_SPI_Cmd(WRITE, main_BIST_interface, 0xC1A0);
            rxData = send_single_PT2000_SPI_Cmd(READ, main_BIST_interface, 0);
            if (rxData == 0x0) LBIST_results=1;  // make sure you are exiting the BIST mode
            else LBIST_results=0;
        }
        else LBIST_results=0;
    }

    // BIST result
    BIST_results = MBIST_results & LBIST_results;

    return BIST_results;   //if 1 OK else failure occurred on BIST
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : OA_path_check
// Description     : make sure that OA are connected to MCU
// Return type     : bool: 1 if OA 1 2 3 are connected to MCU, 0 if not
/////////////////////////////////////////////////////////////////////////////////////////////////
bool OA_path_check(_Bool OA1_check, _Bool OA2_check, _Bool OA3_check)
{
    bool OA_results=0;
    int OA_value=0;
    bool OA1_test=1;
    bool OA2_test=1;
    bool OA3_test=1;
    unsigned char ch1 = 1;
    unsigned char ch2 = 2;
    unsigned char ch3 = 3;

    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page
    
    // Check OA connection to MCU using internal 2.5V

    if(OA1_check == 1)  // OA1
    {
        send_single_PT2000_SPI_Cmd(WRITE, io_oa_out1_config, 0x2B);   //This will set the OA gain to 2.0, voltage on OA will be 5V
        OA_value = read_ADC(ch1);
        if ( OA_value > 0xF5) OA1_test =1;        // No fault F5x19.6mV = 4.8V on OA meaning VCC2P5 > 2.4V (gain of 2)
        else OA1_test =0;
    }
    if(OA2_check == 1)  // OA2
    {
        send_single_PT2000_SPI_Cmd(WRITE, io_oa_out2_config, 0x2B);   //This will set the OA gain to 2.0, voltage on OA will be 5V
        OA_value = read_ADC1(ch2);
        if ( OA_value > 0xF5) OA2_test =1;
        else OA2_test =0;
    }
    if(OA3_check == 1)  // OA3
    {
        send_single_PT2000_SPI_Cmd(WRITE, io_oa_out3_config, 0x2B);   //This will set the OA gain to 2.0, voltage on OA will be 5V
        OA_value = read_ADC1(ch3);
        if ( OA_value > 0xF5) OA3_test =1;
        else OA3_test =0;
    }
    
    OA_results = OA1_test  & OA2_test & OA3_test;

    return OA_results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : Checksum_check
// Description     : Check if the flash enable works properly. To be run after programming device
// Return type     : bool: 1 if flash enable is OK and 0 if checksum error occurred (check key used)
//////////////////////////////////////////////////////////////////////////////////////////////////
bool Checksum_check()
{
    unsigned short rxData = 0;
    bool Checksum_results = true;

    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page

    // Checksum test
    // Channel 1
    rxData = send_single_PT2000_SPI_Cmd(READ, ch1_flash_enable, 0x00);
    // If bit checksum failure value is not 0, exit loop and do nothing since it will not be safe
    if (((rxData & 0x01) == 1) | ((rxData & 0x30) != 0x30)) { Checksum_results = 0; }

    // Channel 2
    rxData = send_single_PT2000_SPI_Cmd(READ, ch2_flash_enable, 0x00);
    // If bit checksum failure value is not 0, exit loop and do nothing since it will not be safe
    if (((rxData & 0x01) == 1) | ((rxData & 0x30) != 0x30)) { Checksum_results = 0; }

    // Channel 3
    rxData = send_single_PT2000_SPI_Cmd(READ, ch3_flash_enable, 0x00);
    // If bit checksum failure value is not 0, exit loop and do nothing since it will not be safe
    if (((rxData & 0x01) == 1) | ((rxData & 0x30) != 0x30)) { Checksum_results = 0; }

    return Checksum_results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : Device_Lock_Unlock
// Description     : Lock some SPI register or unlock them. Used only for safety purpose
//                   If Lock_Unlock is set to 1 then spi will be locked if set to 0 it will unlock
// Return type     : void
//////////////////////////////////////////////////////////////////////////////////////////////////
void Device_Lock_Unlock (unsigned char Lock_Unlock)
{
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page

    if (Lock_Unlock ==1) // Lock device
    {
        send_single_PT2000_SPI_Cmd(WRITE, main_Device_lock, 0x01); // Just SPI is locked not the DRAM private
    }
    else   // unlock device
    {
        send_single_PT2000_SPI_Cmd(WRITE, main_Device_unlock, 0x1337); // Unlock device
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : Read_VbatADC
// Description     : Read Vbat voltage using PT2000x ADC. Make sure off comp prescaler is set
//                   properly to 500kHz max
// Return type     : Vbat value
//////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short Read_VbatADC ()
{
    unsigned short VBAT_value = 0;

    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Select common page
    VBAT_value = send_single_PT2000_SPI_Cmd(READ, io_batt_result, 0); // Read adc vbat register
    VBAT_value = VBAT_value * 0.03906 * 16;  // LSB 39.06mV, ratio 16

    return  VBAT_value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : ID_Check
// Description     : Used to check if SPI works properly and if device connected match with MCU code
// Return type     : bool: 1 if OK and 0 if fails
//////////////////////////////////////////////////////////////////////////////////////////////////
bool ID_Check ()
{
    unsigned short rxData = 0;
    bool ID_results = true;

    // Check if SPI works and that device is the good one
    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Set common page
    rxData = send_single_PT2000_SPI_Cmd(READ, main_Device_Identifier, 0x00); // Read back device ID (value will be in rxData)

    // If device ID is wrong, return 0
    if (rxData != 0xAE32) { ID_results = false; }

    return ID_results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : Bootstrap_check
// Description     : Used to check if bootstrap capacitor are charged before starting actuation
//                     make sure hsx_ls_act are set properly
// Return type     : unsigned long: timer value if charged meaning >0 and 0 if not charged
//////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long Bootstrap_check ()
{
    unsigned short rxData = 0;
    unsigned long Bootstrap_results = 1;

    send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100); // Set common page
    rxData = send_single_PT2000_SPI_Cmd(READ, io_bootstrap_charged, 0x00);

    if((rxData & 0x7F) != 0)
    {
        delay(300); // wait 300ms to see if it is finally charged this timing depend on bootstrap charge strategy
        rxData = send_single_PT2000_SPI_Cmd(READ, io_bootstrap_charged, 0x00);

        if((rxData & 0x7F) != 0) {Bootstrap_results =0;} // error if still not charged

    }
    else
    {
        rxData = send_single_PT2000_SPI_Cmd(READ, io_bootstrap_timer, 0x00);
        Bootstrap_results = (rxData << 14)  * 4.16666666666667E-08;   // data * 2^14 * (1/24MHz)
    }

    return Bootstrap_results;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : Trace_config
// Description     : Used to configure and enable the trace.
//                   External tracer KITPSCDEBUGEVM to be turned ON before executing this
//                   function (see IDE)
// Return type     : void
//////////////////////////////////////////////////////////////////////////////////////////////////
void Tracer (unsigned int trace_start, unsigned int trace_stop, _Bool ucore, unsigned char channel , unsigned char post_trigger_length, _Bool trace_enable)
{
    unsigned short uc_select;
    unsigned short trace_config;

    send_single_PT2000_SPI_Cmd(WRITE, main_Trace_start, trace_start); // Set trace start
    send_single_PT2000_SPI_Cmd(WRITE, main_Trace_stop, trace_stop); // Set trace stop
    uc_select = ((channel-1) << 1) + ucore;  // 000 uc0 channel1, 001 uc1 ch1, 100 uc0 ch3...
    trace_config = post_trigger_length + (uc_select << 8) + (trace_enable << 15);
      send_single_PT2000_SPI_Cmd(WRITE, main_Trace_config, trace_config); // Set trace config
}
