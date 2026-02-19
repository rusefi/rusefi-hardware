#ifndef SAFETY_MONITOR_HPP
#define SAFETY_MONITOR_HPP

#include <cstdint>

class AdcSampler;

enum SafetyErrorCode : uint32_t {
    ERROR_NONE = 0,
    ERROR_WATCHDOG_TIMEOUT   = 0x00000001,
    ERROR_VOBOOST_OOB        = 0x00000002,
    ERROR_VINPBOOST_OOB      = 0x00000003,
    ERROR_CAN_BUS            = 0x00000004,
    ERROR_STACK_OVERFLOW     = 0x00000005,
    ERROR_MALLOC_FAILED      = 0x00000006,
};

class SafetyMonitor {
public:
    static constexpr uint32_t kErrorRingSize = 8;
    static constexpr uint32_t kErrorThreshold = 10;

    // ADC voltage thresholds (millivolts at the ADC pin, not at the sense net).
    static constexpr uint32_t kVoboostMinMv = 100;
    static constexpr uint32_t kVoboostMaxMv = 3200;
    static constexpr uint32_t kVinPboostMinMv = 100;
    static constexpr uint32_t kVinPboostMaxMv = 3200;

    SafetyMonitor();
    ~SafetyMonitor();

    void init(AdcSampler* adc = nullptr);
    void kickWatchdog();
    void reportError(uint32_t errorCode);
    void checkSystemHealth();
    bool isSystemHealthy() const;
    void clearErrors();

    // Ring buffer of the last kErrorRingSize error codes.
    const uint32_t* lastErrors() const;
    uint32_t errorCount() const;

private:
    bool systemHealthy_;
    uint32_t errorCount_;
    uint32_t lastWatchdogKick_;
    uint32_t errorRing_[kErrorRingSize];
    uint32_t errorRingIdx_;
    AdcSampler* adc_;
};

#endif // SAFETY_MONITOR_HPP
