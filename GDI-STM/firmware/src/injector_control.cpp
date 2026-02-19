#include "injector_control.hpp"
#include "board_pins.hpp"
#include "stm32g4xx_hal.h"

extern "C" void Error_Handler(void);

TIM_HandleTypeDef htim1; // Timer for injector control

static InjectorControl* g_injectorControlInstance = nullptr;

InjectorControl::InjectorControl() : globalOnTimeUs(1000), globalOffTimeUs(9000) {
    for (auto& config : injectorConfigs) {
        config.onTimeUs = globalOnTimeUs;
        config.offTimeUs = globalOffTimeUs;
        config.active = false;
    }
    for (size_t i = 0; i < NUM_INJECTORS; ++i) {
        pulseEndTick_[i] = 0;
        cooldownUntilTick_[i] = 0;
    }
    ocRunning_ = false;
}

InjectorControl::~InjectorControl() {
    (void)HAL_TIM_OC_DeInit(&htim1);
    (void)HAL_TIM_Base_DeInit(&htim1);
    if (g_injectorControlInstance == this) {
        g_injectorControlInstance = nullptr;
    }
}

bool InjectorControl::init() {
    configureGPIO();
    configureTimer();
    g_injectorControlInstance = this;
    return true;
}

void InjectorControl::configureGPIO() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clocks for injector outputs.
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // Injector outputs (from GDI-STM.kicad_pcb):
    // INJ1..INJ8 -> PE8..PE15.
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = 0;
    for (size_t i = 0; i < NUM_INJECTORS; ++i) {
        if (board::kInjPins[i].port == GPIOE) {
            GPIO_InitStruct.Pin |= board::kInjPins[i].pin;
        }
    }
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOE, GPIO_InitStruct.Pin, GPIO_PIN_RESET);
}

void InjectorControl::configureTimer() {
    htim1.Instance = TIM1;

    // TIM1 is on APB2. With the current clock tree (APB2 prescaler = 1),
    // TIM1CLK == PCLK2. We target 1 MHz (1 us ticks) in 16-bit mode.
    const uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
    const uint32_t prescaler = (pclk2 / 1000000UL);
    if (prescaler == 0) {
        Error_Handler();
    }
    htim1.Init.Prescaler = prescaler - 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 0xFFFF;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_TIM_OC_Init(&htim1) != HAL_OK) {
        Error_Handler();
    }

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_TIMING;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_TIM_Base_Start(&htim1) != HAL_OK) {
        Error_Handler();
    }

    // Do not start CC interrupts until at least one injector pulse is scheduled.
    (void)HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);
}

void InjectorControl::setInjectorTiming(uint8_t injectorId, uint32_t onTimeUs, uint32_t offTimeUs) {
    if (injectorId < NUM_INJECTORS) {
        injectorConfigs[injectorId].onTimeUs = onTimeUs;
        injectorConfigs[injectorId].offTimeUs = offTimeUs;
    }
}

void InjectorControl::fireInjector(uint8_t injectorId) {
    if (injectorId < NUM_INJECTORS) {
        // Critical section: accessed by TIM1 CC ISR.
        const uint32_t primask = __get_PRIMASK();
        __disable_irq();

        const uint16_t now = static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(&htim1));
        const uint16_t cooldownUntil = cooldownUntilTick_[injectorId];
        if (static_cast<int16_t>(now - cooldownUntil) < 0) {
            // Still within the configured off-time: ignore request.
            if (primask == 0) {
                __enable_irq();
            }
            return;
        }

        injectorConfigs[injectorId].active = true;
        const uint32_t onUs = injectorConfigs[injectorId].onTimeUs;
        const uint32_t offUs = injectorConfigs[injectorId].offTimeUs;

        // Clamp to what we can represent reliably with a 16-bit timer and wrap-aware comparisons.
        const uint16_t onTicks = static_cast<uint16_t>((onUs > 60000U) ? 60000U : onUs);
        const uint16_t offTicks = static_cast<uint16_t>((offUs > 60000U) ? 60000U : offUs);

        const uint16_t endTick = static_cast<uint16_t>(now + onTicks);
        pulseEndTick_[injectorId] = endTick;
        cooldownUntilTick_[injectorId] = static_cast<uint16_t>(endTick + offTicks);

        HAL_GPIO_WritePin(board::kInjPins[injectorId].port, board::kInjPins[injectorId].pin, GPIO_PIN_SET);

        scheduleNextCompare(now);

        if (primask == 0) {
            __enable_irq();
        }
    }
}

void InjectorControl::stopInjector(uint8_t injectorId) {
    if (injectorId < NUM_INJECTORS) {
        const uint32_t primask = __get_PRIMASK();
        __disable_irq();

        const uint16_t now = static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(&htim1));

        injectorConfigs[injectorId].active = false;
        HAL_GPIO_WritePin(board::kInjPins[injectorId].port, board::kInjPins[injectorId].pin, GPIO_PIN_RESET);

        // If the injector is stopped early (e.g. by an external trigger falling edge),
        // clear any pending "must stay off until endTick" state so a new pulse can start
        // after the configured off-time from *now*.
        pulseEndTick_[injectorId] = now;
        const uint32_t offUs = injectorConfigs[injectorId].offTimeUs;
        const uint16_t offTicks = static_cast<uint16_t>((offUs > 60000U) ? 60000U : offUs);
        cooldownUntilTick_[injectorId] = static_cast<uint16_t>(now + offTicks);

        scheduleNextCompare(now);

        if (primask == 0) {
            __enable_irq();
        }
    }
}

void InjectorControl::fireAllInjectors() {
    for (uint8_t i = 0; i < NUM_INJECTORS; ++i) {
        fireInjector(i);
    }
}

void InjectorControl::stopAllInjectors() {
    for (uint8_t i = 0; i < NUM_INJECTORS; ++i) {
        stopInjector(i);
    }
}

void InjectorControl::setGlobalTiming(uint32_t onTimeUs, uint32_t offTimeUs) {
    globalOnTimeUs = onTimeUs;
    globalOffTimeUs = offTimeUs;

    for (auto& config : injectorConfigs) {
        if (!config.active) {
            config.onTimeUs = globalOnTimeUs;
            config.offTimeUs = globalOffTimeUs;
        }
    }
}

void InjectorControl::scheduleNextCompare(uint16_t nowTick) {
    // Find the soonest pulse end among active injectors.
    bool any = false;
    uint16_t bestDelta = 0xFFFF;
    uint16_t bestEnd = 0;

    for (size_t i = 0; i < NUM_INJECTORS; ++i) {
        if (!injectorConfigs[i].active) {
            continue;
        }
        const uint16_t end = pulseEndTick_[i];
        const uint16_t delta = static_cast<uint16_t>(end - nowTick);
        if (!any || delta < bestDelta) {
            any = true;
            bestDelta = delta;
            bestEnd = end;
        }
    }

    if (!any) {
        if (ocRunning_) {
            (void)HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);
            ocRunning_ = false;
        }
        return;
    }

    // Program next compare. Avoid programming CCR == CNT which can miss an event.
    if (bestEnd == nowTick) {
        bestEnd = static_cast<uint16_t>(nowTick + 1);
    }
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, bestEnd);

    if (!ocRunning_) {
        if (HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1) != HAL_OK) {
            Error_Handler();
        }
        ocRunning_ = true;
    }
}

extern "C" void TIM1_CC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

void InjectorControl::onTim1CompareIsr() {
    const uint16_t now = static_cast<uint16_t>(__HAL_TIM_GET_COUNTER(&htim1));

    // Turn off any injectors whose pulse end is in the past (wrap-aware).
    for (size_t i = 0; i < NUM_INJECTORS; ++i) {
        if (!injectorConfigs[i].active) {
            continue;
        }
        const uint16_t end = pulseEndTick_[i];
        if (static_cast<int16_t>(now - end) >= 0) {
            injectorConfigs[i].active = false;
            HAL_GPIO_WritePin(board::kInjPins[i].port, board::kInjPins[i].pin, GPIO_PIN_RESET);
        }
    }

    scheduleNextCompare(now);
}

extern "C" void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim) {
    if (!htim || htim->Instance != TIM1) {
        return;
    }

    if (g_injectorControlInstance) {
        g_injectorControlInstance->onTim1CompareIsr();
    }
}
