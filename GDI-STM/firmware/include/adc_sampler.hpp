#ifndef ADC_SAMPLER_HPP
#define ADC_SAMPLER_HPP

#include <cstdint>

// Lightweight polling ADC sampler for PCB-defined sense nets.
// Uses STM32 HAL ADC (no DMA) to keep bring-up simple and deterministic.
class AdcSampler {
public:
    enum class Signal : uint8_t {
        // Boost sense
        Voboost,     // PA8  — ADC5_IN1
        // Output voltage sense (Vo1..Vo4)
        Vo1,         // PA2  — ADC1_IN3
        Vo2,         // PA6  — ADC2_IN3
        Vo3,         // PB1  — ADC1_IN12
        Vo4,         // PB12 — ADC1_IN11
        // Negative input sense (VinM1..VinM4, VinMboost)
        VinM1,       // PC5  — ADC2_IN11
        VinM2,       // PA5  — ADC2_IN13
        VinM3,       // PB2  — ADC2_IN12
        VinM4,       // PB10 — no ADC channel; readOnce returns false
        VinMboost,   // PB15 — ADC4_IN5
        // Positive input sense (VinP1..VinP4, VinPboost)
        VinP1,       // PA7  — ADC2_IN4
        VinP2,       // PB0  — ADC1_IN15
        VinP3,       // PB13 — ADC3_IN5
        VinP4,       // PB11 — ADC2_IN14
        VinPboost,   // PC3  — ADC2_IN9
        // Injector current sense (Vinj1..Vinj8)
        Vinj1,       // PC0  — ADC2_IN6
        Vinj2,       // PC1  — ADC2_IN7
        Vinj3,       // PC2  — ADC2_IN8
        Vinj4,       // PA0  — ADC1_IN1
        Vinj5,       // PA1  — ADC1_IN2
        Vinj6,       // PA3  — ADC1_IN4
        Vinj7,       // PA4  — ADC2_IN17
        Vinj8,       // PC4  — ADC2_IN5
        Count,
    };

    AdcSampler();
    ~AdcSampler();

    bool init();

    // Samples all configured signals once.
    // Safe to call from a single task; not ISR-safe.
    void sampleAll(uint32_t timeoutMs = 2);

    uint16_t raw(Signal s) const;

    // Converts a raw reading to millivolts using the configured VDDA (default 3300 mV).
    uint32_t millivolts(Signal s) const;

    void setVddaMillivolts(uint32_t vddaMv);

private:
    uint16_t values_[static_cast<uint8_t>(Signal::Count)];
    uint32_t vddaMv_;

    bool readOnce(Signal s, uint32_t timeoutMs, uint16_t& out);
};

#endif // ADC_SAMPLER_HPP
