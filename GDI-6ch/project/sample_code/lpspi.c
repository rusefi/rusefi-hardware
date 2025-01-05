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

/*
 * lpspi.c
 *
 *  SPI Source File
 *
 */

#include "stddef.h"
#include "lpspi.h"

// SPI transfer constants
const unsigned short TIMEOUT = 1000;
const size_t transferByteCount = 2U;

// SPI transmit and receive buffers
unsigned char masterDataSend[2];
unsigned char masterDataReceive[2];


/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : init_SPI
// Description     : Initializes the SPI interface
// Return type     : void
// Argument        : void
/////////////////////////////////////////////////////////////////////////////////////////////////
void init_SPI(void)
{
    // Add code to initialize the SPI interface
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Function name   : send_16bit_SPI
// Description     : Sends a 16 bit SPI word to the device
// Return type     : uint16_t - Data received from the device
// Argument        : unsigned short data16 - Data to send to the device
/////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short send_16bit_SPI(unsigned short data16)
{
    unsigned short rxData = 0;

    // Send data from master
    masterDataSend[0] = data16 & 0xFF;
    masterDataSend[1] = (data16 >> 8) & 0xFF;

    // Add code to send a SPI word

    rxData = ((masterDataReceive[1] << 8) & 0xFF00) | masterDataReceive[0];
    return rxData;
}
