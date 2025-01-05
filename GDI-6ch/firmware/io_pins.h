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

// D21
#define LED_GREEN_PORT GPIOA
#define LED_GREEN_PIN 8

// UART
#define UART_TX_PORT GPIOA
#define UART_TX_PIN 9
#define UART_RX_PORT GPIOA
#define UART_RX_PIN 10

// CAN
#define CAN_RX_PORT GPIOA
#define CAN_RX_PIN 11 
#define CAN_TX_PORT GPIOA
#define CAN_TX_PIN 12 

// SPI
#define SPI_SCK_PORT GPIOA
#define SPI_SCK_PIN 5
#define SPI_MISO_PORT GPIOA
#define SPI_MISO_PIN 6
#define SPI_MOSI_PORT GPIOA
#define SPI_MOSI_PIN 7

// PT2001 specifics
#define SPI_CS_PORT GPIOB
#define SPI_CS_PIN 2

#define PT_DRVEN_PORT GPIOB
#define PT_DRVEN_PIN 4
#define PT_RESET_PORT GPIOB
#define PT_RESET_PIN 5

#define PT_FLAG_0_PORT GPIOB
#define PT_FLAG_0_PIN 7
#define PT_FLAG_1_PORT GPIOB
#define PT_FLAG_1_PIN 10
#define PT_FLAG_2_PORT GPIOB
#define PT_FLAG_2_PIN 11 
#define PT_FLAG_3_PORT GPIOA
#define PT_FLAG_3_PIN 15 
#define PT_FLAG_4_PORT GPIOD
#define PT_FLAG_4_PIN 0 

#define PT_OA_1_PORT GPIOB
#define PT_OA_1_PIN 0
#define PT_OA_2_PORT GPIOB
#define PT_OA_2_PIN 1
#define PT_OA_3_PORT GPIOA
#define PT_OA_3_PIN 1

#define PT_IRQ_PORT GPIOB
#define PT_IRQ_PIN 9
#define PT_DBG_PORT GPIOB
#define PT_DBG_PIN 12