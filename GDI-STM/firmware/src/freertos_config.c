// FreeRTOS hook implementations for GDI-STM firmware.

#include "FreeRTOS.h"
#include "task.h"

extern void Error_Handler(void);

void vApplicationMallocFailedHook(void) {
    Error_Handler();
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void)pxTask;
    (void)pcTaskName;
    Error_Handler();
}

void vApplicationIdleHook(void) {
}
