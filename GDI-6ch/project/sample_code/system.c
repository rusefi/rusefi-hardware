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

#include "system.h"

const unsigned long  DELAY_FACTOR = 1599;   // This needs to be set for the specific MCU being used
const unsigned long  DELAY_FACTOR2 = 159;
extern volatile int flag_IRQB;

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : init_ADC
// Description     : Initializes the ADC subsystem.
// Return type     : void
// Argument        : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void init_ADC(void)
{
    // Add MCU specific ADC initialization code here
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : read_ADC
// Description     : Read from ADC module 0
// Return type     : uint16_t
// Argument        : unsigned char channel
/////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short read_ADC(unsigned char channel)
{
    unsigned short result = 0;

    // Add MCU specific ADC read code here

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : read_ADC1
// Description     : Read from ADC module 1
// Return type     : uint16_t
// Argument        : unsigned char channel
/////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short read_ADC1(unsigned char channel)
{
    unsigned short result = 0;

    // Add MCU specific ADC read code here

    return result;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : init_CLOCK
// Description     : Initializes the system clock
// Return type     : void
// Argument        : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void init_CLOCK(void)
{
    // Add code here to initialize the system clock
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : delay
// Description     : Delays for the specified number of milliseconds.
// Return type     : void
// Argument        : unsigned long msDelay
/////////////////////////////////////////////////////////////////////////////////////////////////
void delay(unsigned long msDelay)    // RICARDO HELP TO GET SOMETHING ACCURATE
{
    unsigned int i = 0;

    for (i = 0; i < (msDelay * DELAY_FACTOR); i++);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : delay100us
// Description     : Delays for the specified number of 100us increments.
// Return type     : void
// Argument        : unsigned long delay
/////////////////////////////////////////////////////////////////////////////////////////////////
void delay100us(unsigned long delay)  // RICARDO HELP TO GET SOMETHING ACCURATE IN 1us or 10us not 100us
{
    unsigned int i = 0;

    for (i = 0; i < (delay * DELAY_FACTOR2); i++);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : init_GPIO
// Description     : Initializes the GPIO pins
// Return type     : void
// Argument        : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void init_GPIO(void)
{
    // Add GPIO initialization code here
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : set_reset_pin
// Description     : Sets the state of the PT2000 reset pin (HIGH or LOW)
// Return type     : void
// Argument        : int state
/////////////////////////////////////////////////////////////////////////////////////////////////
void set_reset_pin(int state)
{
    if (state == HIGH)
    {
        // Set the reset GPIO pin high
        SET_RESETB_HIGH;
    }
    else if (state == LOW)
    {
        // Set the reset GPIO pin low
        SET_RESETB_LOW;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : set_drven_pin
// Description     : Sets the state of the Drive Enable pin  (HIGH or LOW)
// Return type     : void
// Argument        : int state
/////////////////////////////////////////////////////////////////////////////////////////////////
void set_drven_pin(int state)
{
    if (state == HIGH)
    {
        // Set the drive enable GPIO pin high
        SET_DRVEN_HIGH;
    }
    else if (state == LOW)
    {
        // Set the drive enable GPIO pin low
        SET_DRVEN_LOW;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : PWM_1MHz
// Description     : Initialize the 1MHz PWM signal
// Return type     : void
// Argument        : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void PWM_1MHz(void)
{
    // Add code to initialize the 1MHz PWM signal
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : PORTA_IRQHandler
// Description     : This interrupt is enabled when PT2000 IRQB pin is low
// Return type     : void
// Argument        : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void PORTA_IRQHandler(void) //IRQB
{
    char ISF_status = 0; // Add code to get interrupt status

    // Off_sequence(); Optional since PT2000 will shutdown output by itself

    if (ISF_status == PTA3)
    {
        flag_IRQB = 1;

        // Add code to clear interrupt
    }
}
