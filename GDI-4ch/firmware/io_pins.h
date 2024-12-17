/*
 * io_pins.h
 *
 * See https://github.com/rusefi/rusefi/tree/master/firmware/config/boards/GDI4/connectors for interactive pinout
 *
 *  Created on: Jan 10, 2022
 */

#pragma once

// D20
#define LED_BLUE_PORT GPIOB
#define LED_BLUE_PIN 13

//wanna develop on blue pill without actual PC2001 connected?
//#define LED_BLUE_PORT GPIOC
//#define LED_BLUE_PIN 13

// D21
#define LED_GREEN_PORT GPIOA
#define LED_GREEN_PIN 8

// Communication - CAN1
#define CAN_GPIO_PORT				GPIOA
#define CAN_TX_PIN				12
#define CAN_RX_PIN				11

// Board ID (TODO: sample analog input?)
#define BOARD_ID_PORT			GPIOA
#define BOARD_ID_PIN			1

// Communication - UART
#define UART_BAUD_RATE      115200