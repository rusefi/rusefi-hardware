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

#include "lpspi.h"
#include "system.h"
#include "PT2000.h"
#include "PT2000_irq.h"
#include "PT2000_spi_map.h"

extern int flag_IRQB;
extern int drv_int_counter;
extern const int sw_retry;
extern const int auto_retry;
extern uint16_t rxData;
extern uint16_t cksys_missing_error;
extern uint16_t cksys_missing;
extern uint16_t drv_irq;
extern uint16_t checksum_irq;
extern uint16_t drv_status;
extern uint16_t spi_irq;
extern uint16_t auto_irq;
extern uint16_t sw_irq;
extern uint16_t inj1_error;
extern uint16_t inj2_error;
extern uint16_t inj3_error;
extern uint16_t inj4_error;
extern uint16_t inj5_error;
extern uint16_t inj6_error;
extern uint16_t fp_error;
extern uint16_t dcdc_error;
volatile bool flag_start1_fail;
volatile bool flag_start2_fail;
volatile bool flag_start3_fail;
volatile bool flag_start4_fail;
volatile bool flag_start5_fail;
volatile bool flag_start6_fail;
volatile bool flag_start7_fail;
uint16_t AutoErrStatus;
uint16_t interrupt_reg1 = 0;
uint16_t interrupt_reg2 = 0;

// Interrupt Handlers

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : ProcessPT2000Interrupts
// Description     : 
// Return type     : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessPT2000Interrupts()
{
    flag_IRQB = 0;

    // Read interrupt register 2 Address 1B5h
    interrupt_reg2 = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part2, 0x00);
    if (interrupt_reg2 != 0)
    {
        ProcessDriverInterrupts();

    } // END of interrupt on 1B5h  UV, OT

    // Read interrupt register 1 Address 1B4h (Auto and SW interrupt from microcore)
    interrupt_reg1 = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00);
    if (interrupt_reg1 != 0)
    {
        // Automatic IRQ (halt bits)
        auto_irq = interrupt_reg1 & 0x003F; // only select b0-5
        if (auto_irq != 0) // One of the halt bits is at 1
        {
            ProcessAutomaticInterrupts();
            ProcessSoftwareInterrupts(); // Needed in case a SW interrupt also happened
        }

        // Software IRQ (irq bits)
        sw_irq = interrupt_reg1 & 0x3F00; // only select b8-13
        if (sw_irq != 0) // One of the irq bits is at 1
        {
            ProcessSoftwareInterrupts();
            ProcessAutomaticInterrupts();
        }
    } // End of Interrupt 1B4 auto SW interrupt
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : ProcessDriverInterrupts
// Description     :
// Return type     : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessDriverInterrupts()
{
    // DRV IRQ
    drv_irq = interrupt_reg2 & 0x0100; // only select b8
    if (drv_irq != 0)
    {
        // drv interrupt occurred need to check which one
        drv_status = send_single_PT2000_SPI_Cmd(READ, main_Driver_Status, 0x00);   // Read driver_status 1B2h
        if ((drv_status & 0xF) != 0) // only select OT, and 3 UV
        {
            send_single_PT2000_SPI_Cmd(WRITE, main_Driver_Status, 0xF); // Try to clear the fault by writing a 1
            // Check if fault is still there
            drv_status = send_single_PT2000_SPI_Cmd(READ, main_Driver_Status, 0x00);   // Read driver_status 1B2h

            if ((drv_status & 0xF) != 0)
            {
                // Error is still there a full reset is tried
            }
            else
            {
                drv_int_counter++;
            }
        } // End of OT UV loop
    } // End driver status interrupt

    // SPI error IRQ
    spi_irq = interrupt_reg2 & 0x0200; // only select b9 SPI IRQ
    if (spi_irq != 0)
    {
        send_single_PT2000_SPI_Cmd(READ, main_SPI_error_code, 0x00);          // Read driver_status 1B3h, ONLY word accepted if SPI error occurred
        send_single_PT2000_SPI_Cmd(WRITE, selection_register, 0x100);         // Select common page
        rxData = send_single_PT2000_SPI_Cmd(READ, main_SPI_error_code, 0x00); // Check and clear error

        // Add code to do a full restart of the MCU using SW reset

    } // End of IRQB SPI

    // cksys missing case IRQB
    cksys_missing = interrupt_reg2 & 0x0400; // only select b10 CLK missing
    if (cksys_missing != 0)
    {
        cksys_missing = send_single_PT2000_SPI_Cmd(READ, main_Backup_Clock_Status, 0x00);  // Read backup_clock_status 1A8h, IRQB goes High again
        if ((cksys_missing & 0x1) != 0) // Keep only bit 0
        {
            PWM_1MHz();   // Reinitialize the 1MHz PWM
            cksys_missing_error++;
            if (cksys_missing_error > 100)
            {
                // Error is still there a full reset is tried
                SET_DRVEN_LOW;
                SET_RESETB_LOW;
                delay(1);
                SET_RESETB_HIGH;
                ProgramDevice();
                delay(1);
                SET_DRVEN_HIGH;
                cksys_missing_error = 0;
            }
            send_single_PT2000_SPI_Cmd(WRITE, main_Backup_Clock_Status, 0x7);  // Switch to CLK pin, in this case IRQB will be low again another interrupt will occurred
            delay(1);  // Need this delay to let PLL relock properly
        }
        if ((cksys_missing & 0x1) == 0)   // Run on main CLK again
        {
            // Set LED GREEN ON
        }
    } // END OF IRQ CLK

    // Checksum case
    checksum_irq = interrupt_reg2 & 0x3800; // only select b11 to b13 checksum error
    if (checksum_irq != 0)
    {
        if ((checksum_irq & 0x0800) == 0x0800)   // Error on Channel 1
        {
            download_RAM(CODE_RAM1);        // transfers code RAM1
            download_register(CH1_REG);        // send channel registers
            send_single_PT2000_SPI_Cmd(WRITE, ch1_flash_enable, 0x001A);  // enable flash and dual sequence ch1 (address = 0x100)
        }
        if ((checksum_irq & 0x1000) == 0x1000)   // Error on Channel 1
        {
            download_RAM(CODE_RAM2);        // transfers code RAM1
            download_register(CH2_REG);        // send channel registers
            send_single_PT2000_SPI_Cmd(WRITE, ch2_flash_enable, 0x001A);  // enable flash and dual sequence ch1 (address = 0x100)
        }
        if ((checksum_irq & 0x2000) == 0x2000)   // Error on Channel 1
        {
            download_RAM(CODE_RAM3);        // transfers code RAM1
            download_register(CH3_REG);        // send channel registers
            send_single_PT2000_SPI_Cmd(WRITE, ch3_flash_enable, 0x001A);  // enable flash and dual sequence ch1 (address = 0x100)
        }
        if (Checksum_check() == 0)
        {
            // Add code to reset MCU if checksum fails again

        }
        else
        {
            // Set LED GREEN ON
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : ProcessAutomaticInterrupts
// Description     :
// Return type     : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessAutomaticInterrupts()
{
    uint16_t status = 0;
    auto_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x003F;        // since this register is latched on first IRQ need to check if other occured
    // ERROR uc0ch1
    if ((auto_irq & 0x1) == 1)  // Error occurred on uc0ch1
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch1_status_reg_uc0, 0x00); // Read Status register 105h uc0ch1 to check which injector failed
        if ((status & 0x1) == 1)   // INJ1 fails b0 at 1
        {
            inj1_error++;
            AutoErrStatus = AutoErr(INJ1);                // Check which fault happened
            if (inj1_error > auto_retry)  // Shutdown INJ1
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register b6
                flag_start1_fail = 1;  // Put start1 flag high in order to stop Start1 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register b6
            }
        }
        if ((status & 0x2) == 2)   // INJ2 fails b0 at 1
        {
            inj2_error++;
            AutoErrStatus = AutoErr(INJ2);                // Check which fault happened
            if (inj2_error > auto_retry)  // Shutdown INJ1
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register b6
                flag_start2_fail = 1;  // Put start1 flag high in order to stop Start2 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register b6
            }
        }
    }

    // ERROR uc1ch1
    auto_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x003F;        // since this register is latched on first IRQ need to check if other occured
    if ((auto_irq & 0x2) == 1)  // Error occurred on uc1ch1
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch1_status_reg_uc1, 0x00); // Read Status register 1 to check which injector failed
        if ((status & 0x1) == 1)   // INJ3 fails b0 at 1
        {
            inj3_error++;
            AutoErrStatus = AutoErr(INJ3);  // Check which fault happened
            if (inj3_error > auto_retry)    // Shutdown INJ1
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x40); // Clear the fault by writing the control register
                flag_start3_fail = 1;                                      // Put start1 flag high in order to stop Start3 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x40); // Clear the fault by writing the control register b6
            }
        }
        if ((status & 0x2) == 2)   // INJ4 fails b0 at 1
        {
            inj4_error++;
            AutoErrStatus = AutoErr(INJ4);  // Check which fault happened
            if (inj4_error > auto_retry)    // Shutdown INJ1
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x40); // Clear the fault by writing the control register
                flag_start4_fail = 1;                                      // Put start1 flag high in order to stop Start4 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x40); // Clear the fault by writing the control register b6
            }
        }
    }

    // ERROR uc1ch2
    auto_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x003F;        // since this register is latched on first IRQ need to check if other occured
    if ((auto_irq & 0x4) == 1)  // Error occurred on uc0ch2
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch2_status_reg_uc0, 0x00); // Read Status register  to check which injector failed
        if ((status & 0x1) == 1)   // INJ5 fails b0 at 1
        {
            inj5_error++;
            AutoErrStatus = AutoErr(INJ5);  // Check which fault happened
            if (inj5_error > auto_retry)    // Shutdown INJ5
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register
                flag_start5_fail = 1;                                      // Put start1 flag high in order to stop Start5 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register b6
            }
        }
        if ((status & 0x2) == 2)   // INJ6 fails b0 at 1
        {
            inj6_error++;
            AutoErrStatus = AutoErr(INJ6);  // Check which fault happened
            if (inj6_error > auto_retry)    // Shutdown INJ6
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register
                flag_start6_fail = 1;                                      // Put start1 flag high in order to stop Start6 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register b6
            }
        }
    }

    // ERROR uc0ch3
    auto_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x003F;  // since this register is latched on first IRQ need to check if other occured
    if ((auto_irq & 0x10) == 1)  // Error occurred on uc0ch3
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch3_status_reg_uc0, 0x00); // Read Status register  to check which injector failed
        if ((status & 0x1) == 1)                                             // FP fails b0 at 1
        {
            fp_error++;
            AutoErrStatus = AutoErr(FP);  // Check which fault happened
            if (fp_error > auto_retry)    // Shutdown FP
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch3_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register
                flag_start7_fail = 1;                                      // Put start1 flag high in order to stop Start7 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch3_ctrl_reg_uc0, 0x40); // Clear the fault by writing the control register b6
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : ProcessSoftwareInterrupts
// Description     : REad interrupt reg to detect whihc ucore fails. This reg is latch so need read it after clearing each
//                     to make sure that none of them are in the queue. This behavior also depends on how microcode is written
// Return type     : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessSoftwareInterrupts()
{
    uint16_t status = 0;
    sw_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x3F00; // since this register is latched on first IRQ need to check if other occured
    if ((sw_irq & 0x100) == 0x100)  // Error occurred on uc0ch1
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch1_status_reg_uc0, 0x00); // Read Status register 105h uc0ch1 to check which injector failed
        if ((status & 0x1) == 1)    // INJ1 fails b0 at 1
        {
            inj1_error++;
            if (inj1_error > sw_retry)  // Shutdown INJ1
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register
                flag_start1_fail = 1;                                       // Put start1 flag high in order to stop Start1 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register b578
            }
        }
        if ((status & 0x2) == 0x2)      // INJ2 fails b1 at 1
        {
            inj2_error++;
            if (inj2_error > sw_retry)  // Shutdown INJ2
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register
                flag_start2_fail = 1;                                       // Put start1 flag high in order to stop Start2 pulse
            }
            else
            {
                // Set LED GREEN ON
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register b578
            }
        }
    } // End uc0ch1

    sw_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x3F00; // since this register is latched on first IRQ need to check if other occured
    if ((sw_irq & 0x200) == 0x200)  // Error occurred on uc1ch1
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch1_status_reg_uc1, 0x00); // Read Status register to check which injector failed
        if ((status & 0x1) == 1)                                             // INJ3 fails b0 at 1
        {
            inj3_error++;
            if (inj3_error > sw_retry)  // Shutdown INJ3
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x1A0);  // Clear the fault by writing the control register
                flag_start3_fail = 1;                                        // Put start1 flag high in order to stop Start3 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x1A0);  // Clear the fault by writing the control register
            }
        }
        if ((status & 0x2) == 0x2)  // INJ4 fails b1 at 1
        {
            inj4_error++;
            if (inj4_error > sw_retry)  // Shutdown INJ4
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x1A0);  // Clear the fault by writing the control register
                flag_start4_fail = 1;                                        // Put start1 flag high in order to stop Start4 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x1A0);  // Clear the fault by writing the control register b578
            }
        }
    } // End uc1ch1

    sw_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x3F00; // since this register is latched on first IRQ need to check if other occured
    if ((sw_irq & 0x400) == 0x400)  // Error occurred on uc0ch2
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch2_status_reg_uc0, 0x00); // Read Status register to check which injector failed

        if ((status & 0x1) == 1) // INJ5 fails b0 at 1
        {
            inj5_error++;
            if (inj5_error > sw_retry)  // Shutdown INJ5
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register
                flag_start5_fail = 1;                                       // Put start1 flag high in order to stop Start5 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register b578
            }
        }
        if ((status & 0x2) == 0x2)   // INJ6 fails b1 at 1
        {
            inj6_error++;
            if (inj6_error > sw_retry)  // Shutdown INJ6
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register
                flag_start6_fail = 1;                                       // Put start1 flag high in order to stop Start6 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch2_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register b578
            }
        }
    } // End uc0ch2

    sw_irq = send_single_PT2000_SPI_Cmd(READ, main_Interrupt_register_part1, 0x00) & 0x3F00; // since this register is latched on first IRQ need to check if other occured

    if ((sw_irq & 0x1000) == 0x1000)  // Error occurred on uc0ch3
    {
        status = send_single_PT2000_SPI_Cmd(READ, ch3_status_reg_uc0, 0x00); // Read Status register to check which injector failed

        if ((status & 0x1) == 1)  // FP fails b0 at 1
        {
            fp_error++;
            if (fp_error > sw_retry)  // Shutdown INJ3
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch3_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register
                flag_start7_fail = 1;                                       // Put start1 flag high in order to stop Start7 pulse
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch3_ctrl_reg_uc0, 0x1A0); // Clear the fault by writing the control register b578
            }
        }
        if ((status & 0x2) == 2)   // DCDC fails b1 at 1
        {
            dcdc_error++;
            if (dcdc_error > sw_retry)  // Shutdown DCDC and all injections
            {
                // ConfigureGPIO(PORTB,  8, PTB, GPIO_INPUT_DIRECTION);  // START1 force as INPUT so low...
            }
            else
            {
                send_single_PT2000_SPI_Cmd(WRITE, ch1_ctrl_reg_uc1, 0x1A0); // Clear the fault by writing the control register b578
            }
        }
    } // End ch3
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : AutoErr
// Description     : Will determine what kind of error happened and which HS LS failed
// Return type     : HS number failed (0 means no fail), LS number failed (0 means no fail), type of error
///////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t AutoErr(unsigned char INJ)
{
    unsigned char HS_Boost_error=0;
    unsigned char HS_Bat_error=0;
    unsigned char LS_error=0;
    unsigned int Fault_signature=0;            //0 no fault, 1 Short to GND, 2 Short to Bat, 4 INJ Open, 8 MOSFET Open
    unsigned int HSxData;
    unsigned int LSxData;
    _Bool FuelPump =0;

    switch (INJ)
    {
        case INJ1:   // HS1 HS2 LS1 uc0ch1
        {
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch1_part1, 0x00);
            HSxData = rxData;
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch1_part_3, 0x00);
            LSxData = rxData;
            break;
        }

        case INJ2:   // HS1 HS2 LS2 uc0ch1
        {
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch1_part1, 0x00);
            HSxData = rxData;
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch1_part_3, 0x00);
            LSxData = rxData >> 2;
            break;
        }

        case INJ3:   // HS3 HS4 LS3 uc1ch1
        {
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc1ch1_part1, 0x00);
            HSxData = rxData >> 6;
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc1ch1_part_3, 0x00);
            LSxData = rxData >> 4;
            break;
        }
        case INJ4:   // HS3 HS4 LS4 uc1ch1
        {
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc1ch1_part1, 0x00);
            HSxData = rxData >> 6;

            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc1ch1_part_3, 0x00);
            LSxData = rxData >> 6;
            break;
        }
        case INJ5:   // HS5 HS6 LS5 uc0ch2
        {
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch2_part1, 0x00); //Only HS5 accessible on this reg
            HSxData = rxData >> 12;
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch2_part2, 0x00); // read part2 to get HS6
            HSxData = HSxData + (rxData >> 3);                                        // shift HS6 to have 6 bits with HS5 and HS6 same as other

            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch2_part_3, 0x00);
            LSxData = rxData >> 8;
            break;
        }
        case FP:   // HS7 LS7 uc0ch3
        {
            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch3_part2, 0x00);
            HSxData = rxData >> 3;

            rxData = send_single_PT2000_SPI_Cmd(READ, diag_err_uc0ch3_part_3, 0x00);
            LSxData = rxData >> 12;

            FuelPump=1;
            break;
        }
    }

    // The following example is a simplified way to check which error occurred

    // HS Bat error
    if ((HSxData & 0x4) == 0x4)     // HSx was ON when fault occurred
    {
        if ((HSxData & 0x1) != 0)   // VDS should be 0 if no error
            {
                HS_Bat_error = 1;
                if ((HSxData & 0x2) != 0) Fault_signature = MOS_OPEN; // HS gate open or HS disconnected
                else Fault_signature = GND_SHORT;                     // HS shorted to GND
            }
    }
    else        // HSx was OFF when fault occurred
    {
        if ((HSxData & 0x1) == 0)  // VDS should be 1 if no error
        {
            HS_Bat_error = 1;
            Fault_signature = INJ_OPEN;
        }
    }

    // HS Boost error
    if (FuelPump==0)        // no Boost FET on the fuel pump
    {
        if ((HSxData & 0x20) == 0x20)         // HSx was ON when fault occurred
            {
                if ((HSxData & 0x8) != 0)    // VDS should be 0 if no error
                {
                    HS_Boost_error = 1;
                    if ((HSxData & 0x10) != 0) Fault_signature = MOS_OPEN; // MOSOPEN or shorted to VBAT(but this should be detected in idle)
                    else Fault_signature = GND_SHORT;                        // HS shorted to GND
                }
            }
        else        // HSx was OFF when fault occurred
            {
                if ((HSxData & 0x8) == 0)        // VDS should be 1 if no error
                {
                    HS_Boost_error = 1;
                    Fault_signature = INJ_OPEN;  // This can also be gate HS Boost open
                }
            }
    }

    //LS error
    if ((LSxData & 0x2) == 0x2)  // LSx was ON when fault occurred
    {
        if ((LSxData & 0x1) != 0)
        {
            LS_error = 1; // LSx failed  short to Bat or Low side open
            Fault_signature = MOS_OPEN;
        }
    }
    else // LSx was OFF when fault occurred
    {
        if ((LSxData & 0x1) == 0)
            {
                LS_error = 1; // LSx failed Low side shorted to GND
                Fault_signature = GND_SHORT;
            }
    }

    FuelPump=0;
    AutoErrStatus = (HS_Boost_error) + (HS_Bat_error << 1) + (LS_error << 2) + (Fault_signature <<3) ;
    return (AutoErrStatus);
}
