#include "stm32g4xx_hal.h"

extern void Error_Handler(void);

void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
    (void)htim;
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base) {
    if (!htim_base) {
        return;
    }

    if (htim_base->Instance == TIM1) {
        __HAL_RCC_TIM1_CLK_ENABLE();

        // TIM1 capture/compare IRQ is used by injector pulse scheduling.
        HAL_NVIC_SetPriority(TIM1_CC_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    } else if (htim_base->Instance == TIM7) {
        __HAL_RCC_TIM7_CLK_ENABLE();

        // TIM7 update IRQ is used by boost converter software PWM.
        HAL_NVIC_SetPriority(TIM7_DAC_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(TIM7_DAC_IRQn);
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base) {
    if (!htim_base) {
        return;
    }

    if (htim_base->Instance == TIM1) {
        __HAL_RCC_TIM1_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
    } else if (htim_base->Instance == TIM7) {
        __HAL_RCC_TIM7_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM7_DAC_IRQn);
    }
}

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan) {
    if (!hfdcan) {
        return;
    }

    if (hfdcan->Instance == FDCAN1) {
        RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
            Error_Handler();
        }

        __HAL_RCC_FDCAN_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // CAN_TX -> PA12, CAN_RX -> PA11 (AF9).
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // Must be a "low" priority (numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
        // if using FreeRTOS FromISR APIs.
        HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
    }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan) {
    if (!hfdcan) {
        return;
    }

    if (hfdcan->Instance == FDCAN1) {
        __HAL_RCC_FDCAN_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
        HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
    }
}
