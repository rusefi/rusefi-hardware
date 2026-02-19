// Application entry point (STM32G474, Cortex-M4F)

#include "adc_sampler.hpp"
#include "boost_control.hpp"
#include "can_driver.hpp"
#include "gdi_can_protocol.hpp"
#include "board_init.hpp"
#include "injector_bridge.hpp"
#include "injector_control.hpp"
#include "high_side_control.hpp"
#include "safety_monitor.hpp"

#include "stm32g4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"

InjectorControl injectorControl;
InjectorBridge injectorBridge;
CanDriver canDriver;
GdiCanProtocol gdiProtocol{canDriver};
SafetyMonitor safetyMonitor;
AdcSampler adcSampler;

static void SystemClock_Config();
static void DefaultTask(void*);

extern "C" void Error_Handler(void) {
    __disable_irq();

    // Force all INJ outputs LOW (PE8-PE15).
    GPIOE->BSRR = static_cast<uint32_t>(0xFFU) << 24; // BR8..BR15
    // Force all HS enables LOW (PC6-PC13).
    GPIOC->BSRR = static_cast<uint32_t>(0xFFU) << 22; // BR6..BR13
    // Force BOOST_PWM LOW (PD0).
    GPIOD->BSRR = static_cast<uint32_t>(1U) << 16;    // BR0

    while (true) {
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    board::init();

    safetyMonitor.init(&adcSampler);
    high_side::init();
    (void)injectorControl.init();
    (void)injectorBridge.init(injectorControl, safetyMonitor);
    (void)canDriver.init();
    (void)adcSampler.init();
    boost::init();

    (void)xTaskCreate(DefaultTask, "default", 512, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    vTaskStartScheduler();

    // Should never reach here.
    while (true) {
    }
}

static void DefaultTask(void*) {
    for (;;) {
        safetyMonitor.checkSystemHealth();

        const bool healthy = safetyMonitor.isSystemHealthy();
        high_side::update(healthy);
        boost::update(healthy);

        adcSampler.sampleAll();

        // Process GDI CAN protocol (rusEFI)
        {
            CanMessage rxMsg;
            while (canDriver.receiveMessage(rxMsg, 0)) {
                gdiProtocol.processReceivedMessage(rxMsg);
            }
        }

        // Periodic GDI status/config transmit (~100 ms)
        gdiProtocol.periodicTransmit();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void SystemClock_Config() {
    // Based on STM32CubeG4 template (STM32G474E-EVAL1):
    // System Clock source = PLL (HSI), SYSCLK/HCLK/PCLK1/PCLK2 = 170 MHz.
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    // Enable voltage range 1 boost mode for frequency above 150 MHz.
    __HAL_RCC_PWR_CLK_ENABLE();
    (void)HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
    __HAL_RCC_PWR_CLK_DISABLE();

    // Activate PLL with HSI as source.
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
    RCC_OscInitStruct.PLL.PLLN = 85;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers.
    RCC_ClkInitStruct.ClockType =
        (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_8) != HAL_OK) {
        Error_Handler();
    }
}
