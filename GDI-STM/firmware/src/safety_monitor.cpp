#include "safety_monitor.hpp"
#include "adc_sampler.hpp"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_iwdg.h"
#include <cstring>

static IWDG_HandleTypeDef hiwdg;

SafetyMonitor::SafetyMonitor() :
    systemHealthy_(true),
    errorCount_(0),
    lastWatchdogKick_(0),
    errorRing_{0},
    errorRingIdx_(0),
    adc_(nullptr)
{
}

SafetyMonitor::~SafetyMonitor() {
}

void SafetyMonitor::init(AdcSampler* adc) {
    adc_ = adc;

    // Initialize Independent Watchdog
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32;  // ~1ms per tick at 32kHz LSI
    hiwdg.Init.Window = IWDG_WINDOW_DISABLE;
    hiwdg.Init.Reload = 1000;  // ~1 second timeout

    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        reportError(ERROR_WATCHDOG_TIMEOUT);
    }

    // Initial watchdog kick
    kickWatchdog();
}

void SafetyMonitor::kickWatchdog() {
    HAL_IWDG_Refresh(&hiwdg);
    lastWatchdogKick_ = HAL_GetTick();
}

void SafetyMonitor::reportError(uint32_t errorCode) {
    errorCount_++;
    errorRing_[errorRingIdx_ % kErrorRingSize] = errorCode;
    errorRingIdx_++;
}

void SafetyMonitor::checkSystemHealth() {
    // Check if watchdog needs kicking (every 500ms)
    if (HAL_GetTick() - lastWatchdogKick_ > 500) {
        kickWatchdog();
    }

    bool healthy = true;

    // Check for excessive errors
    if (errorCount_ > kErrorThreshold) {
        healthy = false;
    }

    // ADC voltage range checks
    if (adc_) {
        const uint32_t voboost = adc_->millivolts(AdcSampler::Signal::Voboost);
        if (voboost < kVoboostMinMv || voboost > kVoboostMaxMv) {
            reportError(ERROR_VOBOOST_OOB);
            healthy = false;
        }

        const uint32_t vinPboost = adc_->millivolts(AdcSampler::Signal::VinPboost);
        if (vinPboost < kVinPboostMinMv || vinPboost > kVinPboostMaxMv) {
            reportError(ERROR_VINPBOOST_OOB);
            healthy = false;
        }
    }

    systemHealthy_ = healthy;
}

bool SafetyMonitor::isSystemHealthy() const {
    return systemHealthy_;
}

void SafetyMonitor::clearErrors() {
    errorCount_ = 0;
    errorRingIdx_ = 0;
    memset(errorRing_, 0, sizeof(errorRing_));
}

const uint32_t* SafetyMonitor::lastErrors() const {
    return errorRing_;
}

uint32_t SafetyMonitor::errorCount() const {
    return errorCount_;
}
