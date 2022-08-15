/*
 * sent.h
 *
 *  Created on: 16 May 2022
 *      Author: alexv
 */

#pragma once

#define SENT_ICU_FREQ       72000000 // == CPU freq

// Sent input1 - TIM3 CH1 - PA7
#define SENT_ICUD_CH1_D ICUD3
#define SENT_ICUD_CH1_CH ICU_CHANNEL_2

// Sent input2 - TIM4 CH1 - PB6
#define SENT_ICUD_CH2_D ICUD4
#define SENT_ICUD_CH2_CH ICU_CHANNEL_1

// Sent input3 - TIM1 CH1 - PA8
#define SENT_ICUD_CH3_D ICUD1
#define SENT_ICUD_CH3_CH ICU_CHANNEL_1

// Sent input4 - TIM2 CH2 - PA1
#define SENT_ICUD_CH4_D ICUD2
#define SENT_ICUD_CH4_CH ICU_CHANNEL_2
