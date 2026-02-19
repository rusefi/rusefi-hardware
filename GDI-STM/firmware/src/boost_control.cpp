#include "boost_control.hpp"
#include "board_pins.hpp"
#include "stm32g4xx_hal.h"

extern "C" void Error_Handler(void);

namespace {

TIM_HandleTypeDef htim7;

// ~100 kHz PWM: 170 MHz / (0+1) / 1700 total ticks per period.
static constexpr uint32_t kPrescaler = 0;
static constexpr uint32_t kTotalTicks = 1700;
static constexpr uint32_t kMinTicks = 2; // avoid zero-length phases

volatile uint32_t g_onTicks = 0;   // ticks HIGH per PWM period
volatile uint32_t g_offTicks = kTotalTicks;
volatile bool g_enabled = false;
volatile bool g_pinHigh = false;

} // namespace

extern "C" void TIM7_DAC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim7);
}

// Called from HAL_TIM_PeriodElapsedCallback (defined in injector_bridge.cpp).
void boost_onTim7PeriodElapsed() {
    if (!g_enabled) {
        return;
    }

    if (g_pinHigh) {
        board::kBoostPwm.port->BSRR = static_cast<uint32_t>(board::kBoostPwm.pin) << 16;
        g_pinHigh = false;
        __HAL_TIM_SET_AUTORELOAD(&htim7, g_offTicks - 1);
    } else {
        const uint32_t onT = g_onTicks;
        if (onT >= kMinTicks) {
            board::kBoostPwm.port->BSRR = board::kBoostPwm.pin;
            g_pinHigh = true;
        }
        __HAL_TIM_SET_AUTORELOAD(&htim7, (onT >= kMinTicks ? onT : kTotalTicks) - 1);
    }
}

namespace boost {

void init() {
    // Configure PD0 as push-pull output.
    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = board::kBoostPwm.pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(board::kBoostPwm.port, &gpio);
    HAL_GPIO_WritePin(board::kBoostPwm.port, board::kBoostPwm.pin, GPIO_PIN_RESET);

    // TIM7 basic timer for software PWM.
    htim7.Instance = TIM7;
    htim7.Init.Prescaler = kPrescaler;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = kTotalTicks - 1;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
        Error_Handler();
    }
}

void setDutyPercent(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    const uint32_t on = (kTotalTicks * percent) / 100U;
    const uint32_t off = kTotalTicks - on;
    g_onTicks = (on < kMinTicks && percent > 0) ? kMinTicks : on;
    g_offTicks = (off < kMinTicks) ? kMinTicks : off;
}

void enable() {
    g_pinHigh = false;
    g_enabled = true;
    (void)HAL_TIM_Base_Start_IT(&htim7);
}

void disable() {
    g_enabled = false;
    (void)HAL_TIM_Base_Stop_IT(&htim7);
    // Force PD0 LOW.
    board::kBoostPwm.port->BSRR = static_cast<uint32_t>(board::kBoostPwm.pin) << 16;
    g_pinHigh = false;
}

void update(bool systemHealthy) {
    if (!systemHealthy) {
        disable();
    }
}

} // namespace boost
