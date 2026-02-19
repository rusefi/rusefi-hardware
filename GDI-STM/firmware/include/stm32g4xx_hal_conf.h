/* Minimal STM32G4 HAL configuration for this project. */

#ifndef STM32G4xx_HAL_CONF_H
#define STM32G4xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED

#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED
#define HAL_FDCAN_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED

/* ########################## Oscillator Values ############################# */
#ifndef HSE_VALUE
#define HSE_VALUE ((uint32_t)8000000)
#endif
#ifndef HSE_STARTUP_TIMEOUT
#define HSE_STARTUP_TIMEOUT ((uint32_t)100)
#endif
#ifndef HSI_VALUE
#define HSI_VALUE ((uint32_t)16000000)
#endif
#ifndef HSI48_VALUE
#define HSI48_VALUE ((uint32_t)48000000)
#endif
#ifndef LSI_VALUE
#define LSI_VALUE ((uint32_t)32000)
#endif
#ifndef LSE_VALUE
#define LSE_VALUE ((uint32_t)32768)
#endif
#ifndef LSE_STARTUP_TIMEOUT
#define LSE_STARTUP_TIMEOUT ((uint32_t)5000)
#endif
#ifndef EXTERNAL_CLOCK_VALUE
#define EXTERNAL_CLOCK_VALUE ((uint32_t)8000000)
#endif

/* ########################## System Configuration ########################## */
#define VDD_VALUE ((uint32_t)3300)
#define TICK_INT_PRIORITY ((uint32_t)0x0F)
#define USE_RTOS 0
#define PREFETCH_ENABLE 0

/* ########################## Assert Selection ############################## */
/* Uncomment to enable full assert.
 * #define USE_FULL_ASSERT 1
 */

/* ########################## HAL Includes ################################## */
#include "stm32g4xx_hal_rcc.h"
#include "stm32g4xx_hal_rcc_ex.h"
#include "stm32g4xx_hal_flash.h"
#include "stm32g4xx_hal_flash_ex.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_dma.h"
#include "stm32g4xx_hal_dma_ex.h"
#include "stm32g4xx_hal_cortex.h"
#include "stm32g4xx_hal_pwr.h"
#include "stm32g4xx_hal_pwr_ex.h"
#include "stm32g4xx_hal_tim.h"
#include "stm32g4xx_hal_tim_ex.h"
#include "stm32g4xx_hal_spi.h"
#include "stm32g4xx_hal_fdcan.h"
#include "stm32g4xx_hal_iwdg.h"
#include "stm32g4xx_hal_adc.h"
#include "stm32g4xx_hal_adc_ex.h"

#ifdef USE_FULL_ASSERT
#include "stm32_assert.h"
#else
#ifndef assert_param
#define assert_param(expr) ((void)0U)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* STM32G4xx_HAL_CONF_H */
