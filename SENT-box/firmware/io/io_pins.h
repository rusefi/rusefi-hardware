#pragma once

#define LED_BLUE_PORT GPIOC
#define LED_BLUE_PIN 13
#define LL_LED_BLUE_PIN LL_GPIO_PIN_13

//#define LED_GREEN_PORT GPIOA
//#define LED_GREEN_PIN 8

// Communication - UART
#define UART_GPIO_PORT				GPIOA
#define LL_UART_TX_PIN				LL_GPIO_PIN_9
#define LL_UART_RX_PIN				LL_GPIO_PIN_10

// Communication - CAN1
#define CAN_GPIO_PORT				GPIOA
#define LL_CAN_TX_PIN				LL_GPIO_PIN_12
#define LL_CAN_RX_PIN				LL_GPIO_PIN_11
