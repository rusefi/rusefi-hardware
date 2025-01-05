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
 * PT2000.h
 *
 *  PT2000 Header File
 *
 */

#ifndef PT2000_H_
#define PT2000_H_

#include "stdint.h"
#include "stdbool.h"
#include "PT2000_LoadData.h"

#define CODE_RAM1 0
#define CODE_RAM2 1
#define CODE_RAM3 2
#define DATA_RAM  3

#define CH1_REG   0
#define CH2_REG   1
#define CH3_REG   2
#define DIAG_REG  3
#define IO_REG    4
#define MAIN_REG  5

uint16_t send_single_PT2000_SPI_Cmd(bool bRead, uint16_t offset, uint16_t txData);
bool send_PT2000_SPI_Cmd(bool bRead, uint16_t start_addr, uint16_t length, uint16_t* pTxData, uint16_t* pRxData);


void ProgramDevice();
void download_RAM(int target);
void download_register(int r_target);

bool ID_Check ();
bool CLK_check();
bool Driver_Status_Init ();
bool DRVEN_check();
bool BIST_check(_Bool MBIST_run, _Bool LBIST_run);
bool OA_path_check(_Bool OA1_check, _Bool OA2_check, _Bool OA3_check);
bool Checksum_check();
uint16_t Read_VbatADC ();
unsigned long Bootstrap_check ();
void Device_Lock_Unlock (unsigned char Lock_Unlock);
void Tracer (unsigned int trace_start, unsigned int trace_stop, _Bool ucore, unsigned char channel , unsigned char post_trigger_length, _Bool trace_enable);

#endif /* PT2000_H_ */
