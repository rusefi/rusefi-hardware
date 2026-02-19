#include "board_init.hpp"
#include "board_pins.hpp"

#include "stm32g4xx_hal.h"

namespace {

static void initOutputLow(const board::GpioPin& pin) {
    HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_RESET);
    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pin.port, &init);
}

static void initInputNoPull(const board::GpioPin& pin) {
    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(pin.port, &init);
}

static void initAnalog(const board::GpioPin& pin) {
    GPIO_InitTypeDef init{};
    init.Pin = pin.pin;
    init.Mode = GPIO_MODE_ANALOG;
    init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(pin.port, &init);
}

} // namespace

namespace board {

void init() {
    // GPIO clocks needed by pins in `include/board_pins.hpp`.
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // Safe defaults:
    // - Gate-driver enables OFF (low).
    // - Injector control outputs OFF (low).
    // - START signals treated as *inputs* (schematic shows they are driven through Schmitt inverters).
    // - Sense pins in analog mode to reduce leakage and digital switching noise.
    for (const auto& pin : kHsVbat) {
        initOutputLow(pin);
    }
    for (const auto& pin : kHsVboost) {
        initOutputLow(pin);
    }

    for (const auto& pin : kInjPins) {
        initOutputLow(pin);
    }

    initOutputLow(kBoostPwm);

    for (const auto& pin : kStartPins) {
        initInputNoPull(pin);
    }

    initAnalog(kSenseVboost);
    initAnalog(kSenseVoboost);
    initAnalog(kSenseVo1);
    initAnalog(kSenseVo2);
    initAnalog(kSenseVo3);
    initAnalog(kSenseVo4);

    initAnalog(kSenseVinM1);
    initAnalog(kSenseVinM2);
    initAnalog(kSenseVinM3);
    initAnalog(kSenseVinM4);
    initAnalog(kSenseVinMboost);

    initAnalog(kSenseVinP1);
    initAnalog(kSenseVinP2);
    initAnalog(kSenseVinP3);
    initAnalog(kSenseVinP4);
    initAnalog(kSenseVinPboost);

    initAnalog(kSenseVinj1);
    initAnalog(kSenseVinj2);
    initAnalog(kSenseVinj3);
    initAnalog(kSenseVinj4);
    initAnalog(kSenseVinj5);
    initAnalog(kSenseVinj6);
    initAnalog(kSenseVinj7);
    initAnalog(kSenseVinj8);
}

} // namespace board
