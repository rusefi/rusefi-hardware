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

//
// PT2000_spi_map.h
//
//  PT2000 SPI Register Addresses
//
//

#ifndef PT2000_SPI_MAP_H_
#define PT2000_SPI_MAP_H_

// PT2000 Channel 1 Register Addresses
#define ch1_flash_enable                 0x100
#define ch1_ctrl_reg_uc0                 0x101
#define ch1_ctrl_reg_uc1                 0x102
#define ch1_start_config_reg_part1       0x103
#define ch1_start_config_reg_part2       0x104
#define ch1_status_reg_uc0               0x105
#define ch1_status_reg_uc1               0x106
#define ch1_code_width                   0x107
#define ch1_checksum_h                   0x108
#define ch1_checksum_l                   0x109
#define ch1_uc0_entry_point              0x10A
#define ch1_uc1_entry_point              0x10B
#define ch1_diag_routine_addr            0x10C
#define ch1_driver_disabled_routine_addr 0x10D
#define ch1_sw_interrupt_routine_addr    0x10E
#define ch1_uc0_irq_status               0x10F
#define ch1_uc1_irq_status               0x110
#define ch1_counter34_prescaler          0x111
#define ch1_dac_rxtx_cr_config           0x112
#define ch1_unlock_word                  0x113

// PT2000 Channel 2 Register Addresses
#define ch2_flash_enable                 0x120
#define ch2_ctrl_reg_uc0                 0x121
#define ch2_ctrl_reg_uc1                 0x122
#define ch2_start_config_reg_part1       0x123
#define ch2_start_config_reg_part2       0x124
#define ch2_status_reg_uc0               0x125
#define ch2_status_reg_uc1               0x126
#define ch2_code_width                   0x127
#define ch2_checksum_h                   0x128
#define ch2_checksum_l                   0x129
#define ch2_uc0_entry_point              0x12A
#define ch2_uc1_entry_point              0x12B
#define ch2_diag_routine_addr            0x12C
#define ch2_driver_disabled_routine_addr 0x12D
#define ch2_sw_interrupt_routine_addr    0x12E
#define ch2_uc0_irq_status               0x12F
#define ch2_uc1_irq_status               0x130
#define ch2_counter34_prescaler          0x131
#define ch2_dac_rxtx_cr_config           0x132
#define ch2_unlock_word                  0x133

// PT2000 Channel 3 Register Addresses
#define ch3_flash_enable                 0x140
#define ch3_ctrl_reg_uc0                 0x141
#define ch3_ctrl_reg_uc1                 0x142
#define ch3_start_config_reg_part1       0x143
#define ch3_start_config_reg_part2       0x144
#define ch3_status_reg_uc0               0x145
#define ch3_status_reg_uc1               0x146
#define ch3_code_width                   0x147
#define ch3_checksum_h                   0x148
#define ch3_checksum_l                   0x149
#define ch3_uc0_entry_point              0x14A
#define ch3_uc1_entry_point              0x14B
#define ch3_diag_routine_addr            0x14C
#define ch3_driver_disabled_routine_addr 0x14D
#define ch3_sw_interrupt_routine_addr    0x14E
#define ch3_uc0_irq_status               0x14F
#define ch3_uc1_irq_status               0x150
#define ch3_counter34_prescaler          0x151
#define ch3_dac_rxtx_cr_config           0x152
#define ch3_unlock_word                  0x153

// PT2000 IO Register Addresses
#define io_fbk_sens_uc0_ch1_part1        0x154
#define io_fbk_sens_uc0_ch1_part2        0x155
#define io_fbk_sens_uc1_ch1_part1        0x156
#define io_fbk_sens_uc1_ch1_part2        0x157
#define io_fbk_sens_uc0_ch2_part1        0x158
#define io_fbk_sens_uc0_ch2_part2        0x159
#define io_fbk_sens_uc1_ch2_part1        0x15A
#define io_fbk_sens_uc1_ch2_part2        0x15B
#define io_fbk_sens_uc0_ch3_part1        0x15C
#define io_fbk_sens_uc0_ch3_part2        0x15D
#define io_fbk_sens_uc1_ch3_part1        0x15E
#define io_fbk_sens_uc1_ch3_part2        0x15F
#define io_out_acc_uc0_ch1               0x160
#define io_out_acc_uc1_ch1               0x161
#define io_out_acc_uc0_ch2               0x162
#define io_out_acc_uc1_ch2               0x163
#define io_out_acc_uc0_ch3               0x164
#define io_out_acc_uc1_ch3               0x165
#define io_cur_block_access_part1        0x166
#define io_cur_block_access_part2        0x167
#define io_cur_block_access_part_3       0x168
#define io_fw_link                       0x169
#define io_fw_ext_req                    0x16A
#define io_vds_thresholds_hs_part1       0x16B
#define io_vds_thresholds_hs_part2       0x16C
#define io_vsrc_thresholds_hs_part1      0x16D
#define io_vsrc_thresholds_hs_part2      0x16E
#define io_vds_thresholds_ls_part1       0x16F
#define io_vds_thresholds_ls_part2       0x170
#define io_hs_slewrate                   0x171
#define io_ls_slewrate_part1             0x172
#define io_ls_slewrate_part2             0x173
#define io_offset_compensation12         0x174
#define io_offset_compensation34         0x175
#define io_offset_compensation56         0x176
#define io_adc12_result                  0x177
#define io_adc34_result                  0x178
#define io_adc56_result                  0x179
#define io_current_filter12              0x17A
#define io_current_filter34              0x17B
#define io_current_filter5l5h            0x17C
#define io_current_filter6l6h            0x17D
#define io_current_filter5neg6neg        0x17E
#define io_boost_dac                     0x17F
#define io_boost_dac_access              0x180
#define io_boost_filter                  0x181
#define io_vds7_dcdc_config              0x182
#define io_vds8_dcdc_config              0x183
#define io_batt_result                   0x184
#define io_dac12_value                   0x185
#define io_dac34_value                   0x186
#define io_dac5l5h_value                 0x187
#define io_dac5neg_value                 0x188
#define io_dac6l6h_value                 0x189
#define io_dac6neg_value                 0x18A
#define io_hs_bias_config                0x18B
#define io_ls_bias_config                0x18C
#define io_bootstrap_charged             0x18D
#define io_bootstrap_timer               0x18E
#define io_hs1_ls_act                    0x18F
#define io_hs2_ls_act                    0x190
#define io_hs3_ls_act                    0x191
#define io_hs4_ls_act                    0x192
#define io_hs5_ls_act                    0x193
#define io_hs6_ls_act                    0x194
#define io_hs7_ls_act                    0x195
#define io_dac_settling_time             0x196
#define io_oa_out1_config                0x197
#define io_oa_out2_config                0x198
#define io_oa_out3_config                0x199

// PT2000 Main Register Addresses
#define main_Clock_Prescaler             0x1A0
#define main_Flags_Direction             0x1A1
#define main_Flags_Polarity              0x1A2
#define main_Flags_source                0x1A3
#define main_compensation_prescaler      0x1A4
#define main_Driver_Config_part1         0x1A5
#define main_Driver_Config_part2         0x1A6
#define main_PLL_config                  0x1A7
#define main_Backup_Clock_Status         0x1A8
#define main_SPI_config                  0x1A9
#define main_Trace_start                 0x1AA
#define main_Trace_stop                  0x1AB
#define main_Trace_config                0x1AC
#define main_Device_lock                 0x1AD
#define main_Reset_behavior              0x1AE
#define main_Device_unlock               0x1AF
#define main_Global_reset_code_part1     0x1B0
#define main_Global_reset_code_part2     0x1B1
#define main_Driver_Status               0x1B2
#define main_SPI_error_code              0x1B3
#define main_Interrupt_register_part1    0x1B4
#define main_Interrupt_register_part2    0x1B5
#define main_Device_Identifier           0x1B6
#define main_Reset_source                0x1B7
#define main_BIST_interface              0x1BD

// PT2000 Diag Register Addresses
#define diag_ls1_diag_config1            0x1C0
#define diag_ls1_diag_config2            0x1C1
#define diag_ls1_output_config           0x1C2
#define diag_ls2_diag_config1            0x1C3
#define diag_ls2_diag_config2            0x1C4
#define diag_ls2_output_config           0x1C5
#define diag_ls3_diag_config1            0x1C6
#define diag_ls3_diag_config2            0x1C7
#define diag_ls3_output_config           0x1C8
#define diag_ls4_diag_config1            0x1C9
#define diag_ls4_diag_config2            0x1CA
#define diag_ls4_output_config           0x1CB
#define diag_ls5_diag_config1            0x1CC
#define diag_ls5_diag_config2            0x1CD
#define diag_ls5_output_config           0x1CE
#define diag_ls6_diag_config1            0x1CF
#define diag_ls6_diag_config2            0x1D0
#define diag_ls6_output_config           0x1D1
#define diag_ls7_diag_config1            0x1D2
#define diag_ls7_diag_config2            0x1D3
#define diag_ls7_output_config           0x1D4
#define diag_ls8_diag_config1            0x1D5
#define diag_ls8_diag_config2            0x1D6
#define diag_ls8_output_config           0x1D7
#define diag_hs1_diag_config1            0x1D8
#define diag_hs1_diag_config2            0x1D9
#define diag_hs1_output_config           0x1DA
#define diag_hs2_diag_config1            0x1DB
#define diag_hs2_diag_config2            0x1DC
#define diag_hs2_output_config           0x1DD
#define diag_hs3_diag_config1            0x1DE
#define diag_hs3_diag_config2            0x1DF
#define diag_hs3_output_config           0x1E0
#define diag_hs4_diag_config1            0x1E1
#define diag_hs4_diag_config2            0x1E2
#define diag_hs4_output_config           0x1E3
#define diag_hs5_diag_config1            0x1E4
#define diag_hs5_diag_config2            0x1E5
#define diag_hs5_output_config           0x1E6
#define diag_hs6_diag_config1            0x1E7
#define diag_hs6_diag_config2            0x1E8
#define diag_hs6_output_config           0x1E9
#define diag_hs7_diag_config1            0x1EA
#define diag_hs7_diag_config2            0x1EB
#define diag_hs7_output_config           0x1EC
#define diag_err_uc0ch1_part1            0x1ED
#define diag_err_uc0ch1_part2            0x1EE
#define diag_err_uc0ch1_part_3           0x1EF
#define diag_err_uc1ch1_part1            0x1F0
#define diag_err_uc1ch1_part2            0x1F1
#define diag_err_uc1ch1_part_3           0x1F2
#define diag_err_uc0ch2_part1            0x1F3
#define diag_err_uc0ch2_part2            0x1F4
#define diag_err_uc0ch2_part_3           0x1F5
#define diag_err_uc1ch2_part1            0x1F6
#define diag_err_uc1ch2_part2            0x1F7
#define diag_err_uc1ch2_part_3           0x1F8
#define diag_err_uc0ch3_part1            0x1F9
#define diag_err_uc0ch3_part2            0x1FA
#define diag_err_uc0ch3_part_3           0x1FB
#define diag_err_uc1ch3_part1            0x1FC
#define diag_err_uc1ch3_part2            0x1FD
#define diag_err_uc1ch3_part_3           0x1FE
#define diag_diagnostics_option          0x1FF

#define selection_register               0x3FF

#endif // PT2000_SPI_MAP_H_
