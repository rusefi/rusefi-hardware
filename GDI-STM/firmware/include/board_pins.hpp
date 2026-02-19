#ifndef BOARD_PINS_HPP
#define BOARD_PINS_HPP

#include "stm32g4xx_hal.h"

#include <cstddef>
#include <cstdint>

namespace board {

struct GpioPin {
    GPIO_TypeDef* port;
    uint16_t pin;
};

// From `GDI-STM.kicad_pcb` (U7: STM32G474M, LQFP-80) - produced PCB is source of truth.
inline constexpr size_t kNumInjectors = 8;

// INJ1..INJ8 -> PE8..PE15 (LQFP80 pins 31..38).
inline constexpr GpioPin kInjPins[kNumInjectors] = {
    {GPIOE, GPIO_PIN_8},  // INJ1
    {GPIOE, GPIO_PIN_9},  // INJ2
    {GPIOE, GPIO_PIN_10}, // INJ3
    {GPIOE, GPIO_PIN_11}, // INJ4
    {GPIOE, GPIO_PIN_12}, // INJ5
    {GPIOE, GPIO_PIN_13}, // INJ6
    {GPIOE, GPIO_PIN_14}, // INJ7
    {GPIOE, GPIO_PIN_15}, // INJ8
};

// START1..START8 -> PA9,PA10,PB3,PB4,PA15,PB5,PB6,PB9 (pins 57,58,72,73,65,74,75,78).
inline constexpr GpioPin kStartPins[kNumInjectors] = {
    {GPIOA, GPIO_PIN_9},  // START1
    {GPIOA, GPIO_PIN_10}, // START2
    {GPIOB, GPIO_PIN_3},  // START3
    {GPIOB, GPIO_PIN_4},  // START4
    {GPIOA, GPIO_PIN_15}, // START5
    {GPIOB, GPIO_PIN_5},  // START6
    {GPIOB, GPIO_PIN_6},  // START7
    {GPIOB, GPIO_PIN_9},  // START8
};

// FDCAN1: CAN_TX -> PA12 (pin 60), CAN_RX -> PA11 (pin 59).
inline constexpr GpioPin kCanTx = {GPIOA, GPIO_PIN_12}; // CAN_TX
inline constexpr GpioPin kCanRx = {GPIOA, GPIO_PIN_11}; // CAN_RX

// High-side sense/control nets
// From `bank.kicad_sch` these nets feed IRS21867S gate-driver inputs (HIN). The firmware treats them as
// digital control outputs and holds them LOW at boot for safety (drivers disabled).
inline constexpr GpioPin kHsVbat[4] = {
    {GPIOC, GPIO_PIN_6}, // HS_VBAT1 (pin 52)
    {GPIOC, GPIO_PIN_7}, // HS_VBAT2 (pin 53)
    {GPIOC, GPIO_PIN_8}, // HS_VBAT3 (pin 54)
    {GPIOC, GPIO_PIN_9}, // HS_VBAT4 (pin 55)
};

inline constexpr GpioPin kHsVboost[4] = {
    {GPIOC, GPIO_PIN_10}, // HS_VBOOST1 (pin 66)
    {GPIOC, GPIO_PIN_11}, // HS_VBOOST2 (pin 67)
    {GPIOC, GPIO_PIN_12}, // HS_VBOOST3 (pin 68)
    {GPIOC, GPIO_PIN_13}, // HS_VBOOST4 (pin 2)
};

// Boost driver PWM/control.
inline constexpr GpioPin kBoostPwm = {GPIOD, GPIO_PIN_0}; // BOOST_PWM (pin 69)

// Analog sense pins (configured as analog inputs by board init).
inline constexpr GpioPin kSenseVboost = {GPIOE, GPIO_PIN_7};   // Vboost (pin 30)
inline constexpr GpioPin kSenseVoboost = {GPIOA, GPIO_PIN_8};  // Voboost (pin 56)
inline constexpr GpioPin kSenseVo1 = {GPIOA, GPIO_PIN_2};      // Vo1 (pin 14)
inline constexpr GpioPin kSenseVo2 = {GPIOA, GPIO_PIN_6};      // Vo2 (pin 20)
inline constexpr GpioPin kSenseVo3 = {GPIOB, GPIO_PIN_1};      // Vo3 (pin 25)
inline constexpr GpioPin kSenseVo4 = {GPIOB, GPIO_PIN_12};     // Vo4 (pin 43)

inline constexpr GpioPin kSenseVinM1 = {GPIOC, GPIO_PIN_5};     // VinM1 (pin 23)
inline constexpr GpioPin kSenseVinM2 = {GPIOA, GPIO_PIN_5};     // VinM2 (pin 19)
inline constexpr GpioPin kSenseVinM3 = {GPIOB, GPIO_PIN_2};     // VinM3 (pin 26)
inline constexpr GpioPin kSenseVinM4 = {GPIOB, GPIO_PIN_10};    // VinM4 (pin 39)
inline constexpr GpioPin kSenseVinMboost = {GPIOB, GPIO_PIN_15}; // VinMboost (pin 46)

inline constexpr GpioPin kSenseVinP1 = {GPIOA, GPIO_PIN_7};      // VinP1 (pin 21)
inline constexpr GpioPin kSenseVinP2 = {GPIOB, GPIO_PIN_0};      // VinP2 (pin 24)
inline constexpr GpioPin kSenseVinP3 = {GPIOB, GPIO_PIN_13};     // VinP3 (pin 44)
inline constexpr GpioPin kSenseVinP4 = {GPIOB, GPIO_PIN_11};     // VinP4 (pin 42)
inline constexpr GpioPin kSenseVinPboost = {GPIOC, GPIO_PIN_3};  // VinPboost (pin 11)

inline constexpr GpioPin kSenseVinj1 = {GPIOC, GPIO_PIN_0}; // Vinj1 (pin 8)
inline constexpr GpioPin kSenseVinj2 = {GPIOC, GPIO_PIN_1}; // Vinj2 (pin 9)
inline constexpr GpioPin kSenseVinj3 = {GPIOC, GPIO_PIN_2}; // Vinj3 (pin 10)
inline constexpr GpioPin kSenseVinj4 = {GPIOA, GPIO_PIN_0}; // Vinj4 (pin 12)
inline constexpr GpioPin kSenseVinj5 = {GPIOA, GPIO_PIN_1}; // Vinj5 (pin 13)
inline constexpr GpioPin kSenseVinj6 = {GPIOA, GPIO_PIN_3}; // Vinj6 (pin 17)
inline constexpr GpioPin kSenseVinj7 = {GPIOA, GPIO_PIN_4}; // Vinj7 (pin 18)
inline constexpr GpioPin kSenseVinj8 = {GPIOC, GPIO_PIN_4}; // Vinj8 (pin 22)

} // namespace board

#endif // BOARD_PINS_HPP
