#include "injector_bridge.hpp"

#include "board_pins.hpp"
#include "injector_control.hpp"
#include "safety_monitor.hpp"

#include "stm32g4xx_hal.h"

namespace {

constexpr uint32_t kMaxPulseUs = 10000;     // safety cutoff if a falling edge is missed
constexpr uint32_t kOffTimeUs = 0;          // allow back-to-back pulses
constexpr uint32_t kStart8PollPeriodUs = 20; // 50 kHz sampling for START8 (PB9)

InjectorBridge* g_bridge = nullptr;
TIM_HandleTypeDef htim6;

static void initStartPinExti(GPIO_TypeDef* port, uint16_t pin) {
    GPIO_InitTypeDef init{};
    init.Pin = pin;
    init.Mode = GPIO_MODE_IT_RISING_FALLING;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &init);
}

static void initTim6Poller() {
    __HAL_RCC_TIM6_CLK_ENABLE();

    htim6.Instance = TIM6;
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;

    // Use a 1 MHz timebase for easy period calculation.
    const uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    const uint32_t prescaler = (pclk1 / 1000000UL);
    if (prescaler == 0) {
        return;
    }
    htim6.Init.Prescaler = prescaler - 1;
    htim6.Init.Period = (kStart8PollPeriodUs > 0) ? (kStart8PollPeriodUs - 1) : 0;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    (void)HAL_TIM_Base_Init(&htim6);

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

    (void)HAL_TIM_Base_Start_IT(&htim6);
}

} // namespace

bool InjectorBridge::init(InjectorControl& injectorControl, SafetyMonitor& safetyMonitor) {
    injectorControl_ = &injectorControl;
    safetyMonitor_ = &safetyMonitor;
    g_bridge = this;

    // Program conservative defaults. Outputs remain OFF until a START edge is received.
    injectorControl_->stopAllInjectors();
    for (uint8_t i = 0; i < NUM_INJECTORS; ++i) {
        injectorControl_->setInjectorTiming(i, kMaxPulseUs, kOffTimeUs);
    }

    // Configure START1..START7 as EXTI.
    // START8 (PB9) shares EXTI line 9 with START1 (PA9), so it is polled by TIM6.
    initStartPinExti(GPIOA, GPIO_PIN_9);  // START1
    initStartPinExti(GPIOA, GPIO_PIN_10); // START2
    initStartPinExti(GPIOB, GPIO_PIN_3);  // START3
    initStartPinExti(GPIOB, GPIO_PIN_4);  // START4
    initStartPinExti(GPIOA, GPIO_PIN_15); // START5
    initStartPinExti(GPIOB, GPIO_PIN_5);  // START6
    initStartPinExti(GPIOB, GPIO_PIN_6);  // START7

    HAL_NVIC_SetPriority(EXTI3_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
    HAL_NVIC_SetPriority(EXTI4_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    start8LastHigh_ = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_SET);
    initTim6Poller();

    return true;
}

void InjectorBridge::applyStartLevel(uint8_t channel, bool high) {
    if (!injectorControl_ || channel >= NUM_INJECTORS) {
        return;
    }

    if (!safetyMonitor_ || !safetyMonitor_->isSystemHealthy()) {
        injectorControl_->stopInjector(channel);
        return;
    }

    if (high) {
        injectorControl_->fireInjector(channel);
    } else {
        injectorControl_->stopInjector(channel);
    }
}

void InjectorBridge::onGpioExtiIsr(uint16_t gpioPin) {
    switch (gpioPin) {
    case GPIO_PIN_9: // START1
        applyStartLevel(0, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_SET);
        break;
    case GPIO_PIN_10: // START2
        applyStartLevel(1, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_SET);
        break;
    case GPIO_PIN_3: // START3
        applyStartLevel(2, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET);
        break;
    case GPIO_PIN_4: // START4
        applyStartLevel(3, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET);
        break;
    case GPIO_PIN_15: // START5
        applyStartLevel(4, HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET);
        break;
    case GPIO_PIN_5: // START6
        applyStartLevel(5, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET);
        break;
    case GPIO_PIN_6: // START7
        applyStartLevel(6, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET);
        break;
    default:
        break;
    }
}

void InjectorBridge::onTim6TickIsr() {
    const bool nowHigh = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == GPIO_PIN_SET);
    if (nowHigh == start8LastHigh_) {
        return;
    }
    start8LastHigh_ = nowHigh;
    applyStartLevel(7, nowHigh);
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (g_bridge) {
        g_bridge->onGpioExtiIsr(GPIO_Pin);
    }
}

// Defined in boost_control.cpp.
extern void boost_onTim7PeriodElapsed();

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    if (!htim) {
        return;
    }
    if (htim->Instance == TIM6 && g_bridge) {
        g_bridge->onTim6TickIsr();
    } else if (htim->Instance == TIM7) {
        boost_onTim7PeriodElapsed();
    }
}

extern "C" void EXTI3_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

extern "C" void EXTI4_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

extern "C" void EXTI9_5_IRQHandler(void) {
    // START1 (PA9), START6 (PB5), START7 (PB6) are in this range.
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

extern "C" void EXTI15_10_IRQHandler(void) {
    // START2 (PA10) and START5 (PA15) are in this range.
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

extern "C" void TIM6_DAC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim6);
}
