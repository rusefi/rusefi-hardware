#include "high_side_control.hpp"

#include "board_pins.hpp"

#include "stm32g4xx_hal.h"

namespace {

#ifndef GDI_ENABLE_HS_VBAT
#define GDI_ENABLE_HS_VBAT 0
#endif

#ifndef GDI_ENABLE_HS_VBOOST
#define GDI_ENABLE_HS_VBOOST 0
#endif

#ifndef GDI_HS_VBAT_MASK
#define GDI_HS_VBAT_MASK 0
#endif

#ifndef GDI_HS_VBOOST_MASK
#define GDI_HS_VBOOST_MASK 0
#endif

#ifndef GDI_HS_ENABLE_DELAY_MS
#define GDI_HS_ENABLE_DELAY_MS 500
#endif

uint32_t g_startTick = 0;

static void applyMask(const board::GpioPin* pins, size_t count, uint32_t mask) {
    for (size_t i = 0; i < count; ++i) {
        const bool en = ((mask >> i) & 0x1U) != 0;
        HAL_GPIO_WritePin(pins[i].port, pins[i].pin, en ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

static void sanitizeMasks(uint32_t& vbatMask, uint32_t& vboostMask) {
    // Safety: never enable both VBAT and VBOOST high-side selects for the same bank.
    // If both are requested, force both OFF for that bank to avoid rail contention/backfeed.
    const uint32_t conflict = (vbatMask & vboostMask) & 0xFU;
    vbatMask &= ~conflict;
    vboostMask &= ~conflict;
}

static void forceAllLow() {
    applyMask(board::kHsVbat, 4, 0);
    applyMask(board::kHsVboost, 4, 0);
}

} // namespace

namespace high_side {

void init() {
    g_startTick = HAL_GetTick();
    forceAllLow();
}

void update(bool systemHealthy) {
    if (!systemHealthy) {
        forceAllLow();
        return;
    }

    const uint32_t now = HAL_GetTick();
    if ((now - g_startTick) < static_cast<uint32_t>(GDI_HS_ENABLE_DELAY_MS)) {
        // Keep outputs disabled during initial bring-up window.
        forceAllLow();
        return;
    }

    uint32_t vbatMask = 0;
    uint32_t vboostMask = 0;

#if (GDI_ENABLE_HS_VBAT != 0)
    vbatMask = static_cast<uint32_t>(GDI_HS_VBAT_MASK);
#endif
#if (GDI_ENABLE_HS_VBOOST != 0)
    vboostMask = static_cast<uint32_t>(GDI_HS_VBOOST_MASK);
#endif

    sanitizeMasks(vbatMask, vboostMask);
    applyMask(board::kHsVbat, 4, vbatMask);
    applyMask(board::kHsVboost, 4, vboostMask);
}

} // namespace high_side
