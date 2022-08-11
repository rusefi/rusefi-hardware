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

// Sent channel1 - PA6
#define HAL_SENT_CH1_LINE_PORT GPIOA
#define HAL_SENT_CH1_LINE_PIN 6

// Sent channel2 - PA7
#define HAL_SENT_CH2_LINE_PORT GPIOA
#define HAL_SENT_CH2_LINE_PIN 7

// Communication - UART
#define UART_GPIO_PORT				GPIOA
  // stm32 TX/UART1 - dongle RX often White
#define UART_TX_PIN				9
  // stm32 RX/UART1 - dongle TX often Green
#define UART_RX_PIN				10
#define UART_BAUD_RATE      115200
