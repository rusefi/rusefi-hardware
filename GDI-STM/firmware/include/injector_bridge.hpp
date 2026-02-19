#ifndef INJECTOR_BRIDGE_HPP
#define INJECTOR_BRIDGE_HPP

#include <cstdint>

class InjectorControl;
class SafetyMonitor;

/**
 * Bridges conditioned input trigger signals (STARTx) to injector control outputs (INJx).
 *
 * Current behavior:
 *  - START rising edge  -> assert INJ output (via InjectorControl)
 *  - START falling edge -> deassert INJ output
 *  - A per-channel maximum on-time is programmed to prevent a stuck ON if a falling edge is missed.
 *
 * Notes:
 *  - START8 is on PA14 (SWCLK). If SWD is active, this pin may not behave as a normal GPIO.
 */
class InjectorBridge {
public:
    bool init(InjectorControl& injectorControl, SafetyMonitor& safetyMonitor);

    // ISR entrypoints (called by HAL callbacks / IRQ handlers).
    void onGpioExtiIsr(uint16_t gpioPin);
    void onTim6TickIsr();

private:
    InjectorControl* injectorControl_{nullptr};
    SafetyMonitor* safetyMonitor_{nullptr};

    // Polling for START8 (PA14) to avoid EXTI14 conflict with START1 (PB14).
    bool start8LastHigh_{false};

    void applyStartLevel(uint8_t channel, bool high);
};

#endif // INJECTOR_BRIDGE_HPP

