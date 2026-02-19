#ifndef INJECTOR_CONTROL_HPP
#define INJECTOR_CONTROL_HPP

#include "board_pins.hpp"
#include <cstdint>
#include <cstddef>

static constexpr size_t NUM_INJECTORS = board::kNumInjectors;

struct InjectorConfig {
    uint32_t onTimeUs;
    uint32_t offTimeUs;
    bool active;
};

class InjectorControl {
public:
    InjectorControl();
    ~InjectorControl();

    bool init();
    void setInjectorTiming(uint8_t injectorId, uint32_t onTimeUs, uint32_t offTimeUs);
    void fireInjector(uint8_t injectorId);
    void stopInjector(uint8_t injectorId);
    void fireAllInjectors();
    void stopAllInjectors();
    void setGlobalTiming(uint32_t onTimeUs, uint32_t offTimeUs);

    // ISR entrypoint (TIM1 CC). Not for application code.
    void onTim1CompareIsr();

private:
    InjectorConfig injectorConfigs[NUM_INJECTORS];
    uint32_t globalOnTimeUs;
    uint32_t globalOffTimeUs;

    // TIM1 runs at 1 MHz. These are 16-bit time values in TIM1 ticks (wraps ~65 ms).
    uint16_t pulseEndTick_[NUM_INJECTORS];
    uint16_t cooldownUntilTick_[NUM_INJECTORS];
    bool ocRunning_;

    void configureGPIO();
    void configureTimer();
    void scheduleNextCompare(uint16_t nowTick);
};

#endif // INJECTOR_CONTROL_HPP
