#include "adc_sampler.hpp"
#include "board_pins.hpp"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_adc.h"
#include "stm32g4xx_hal_adc_ex.h"

extern "C" void Error_Handler(void);

// NOTE: Vboost sense (PE7) has no ADC channel on STM32G474 LQFP-80.
//       VinM4 sense (PB10) has no ADC channel.

namespace {

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;
ADC_HandleTypeDef hadc4;
ADC_HandleTypeDef hadc5;

static bool initAdcCommonClocks() {
    __HAL_RCC_ADC12_CLK_ENABLE();
    __HAL_RCC_ADC345_CLK_ENABLE();

    RCC_PeriphCLKInitTypeDef periphClk = {0};
    periphClk.PeriphClockSelection = RCC_PERIPHCLK_ADC12 | RCC_PERIPHCLK_ADC345;
    periphClk.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
    periphClk.Adc345ClockSelection = RCC_ADC345CLKSOURCE_SYSCLK;
    return (HAL_RCCEx_PeriphCLKConfig(&periphClk) == HAL_OK);
}

static bool initSingleAdc(ADC_HandleTypeDef& hadc, ADC_TypeDef* instance) {
    hadc.Instance = instance;
    hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.GainCompensation = 0;
    hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.NbrOfConversion = 1;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    hadc.Init.OversamplingMode = DISABLE;

    if (HAL_ADC_Init(&hadc) != HAL_OK) {
        return false;
    }
    if (HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED) != HAL_OK) {
        return false;
    }
    return true;
}

static bool readVrefintMv(uint32_t& outVddaMv) {
    ADC_ChannelConfTypeDef config = {0};
    config.Channel = ADC_CHANNEL_VREFINT;
    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    config.SingleDiff = ADC_SINGLE_ENDED;
    config.OffsetNumber = ADC_OFFSET_NONE;
    config.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &config) != HAL_OK) {
        return false;
    }
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return false;
    }
    if (HAL_ADC_PollForConversion(&hadc1, 2) != HAL_OK) {
        (void)HAL_ADC_Stop(&hadc1);
        return false;
    }
    const uint32_t raw = HAL_ADC_GetValue(&hadc1);
    (void)HAL_ADC_Stop(&hadc1);

    if (raw == 0) {
        return false;
    }
    outVddaMv = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(raw, ADC_RESOLUTION_12B);
    return true;
}

struct ChannelSel {
    ADC_HandleTypeDef* hadc;
    uint32_t channel;
};

static ChannelSel channelFor(AdcSampler::Signal s) {
    switch (s) {
        // Boost
        case AdcSampler::Signal::Voboost:   return {&hadc5, ADC_CHANNEL_1};   // PA8:  ADC5_IN1
        // Vo1..Vo4
        case AdcSampler::Signal::Vo1:       return {&hadc1, ADC_CHANNEL_3};   // PA2:  ADC1_IN3
        case AdcSampler::Signal::Vo2:       return {&hadc2, ADC_CHANNEL_3};   // PA6:  ADC2_IN3
        case AdcSampler::Signal::Vo3:       return {&hadc1, ADC_CHANNEL_12};  // PB1:  ADC1_IN12
        case AdcSampler::Signal::Vo4:       return {&hadc1, ADC_CHANNEL_11};  // PB12: ADC1_IN11
        // VinM1..VinM4, VinMboost
        case AdcSampler::Signal::VinM1:     return {&hadc2, ADC_CHANNEL_11};  // PC5:  ADC2_IN11
        case AdcSampler::Signal::VinM2:     return {&hadc2, ADC_CHANNEL_13};  // PA5:  ADC2_IN13
        case AdcSampler::Signal::VinM3:     return {&hadc2, ADC_CHANNEL_12};  // PB2:  ADC2_IN12
        case AdcSampler::Signal::VinM4:     return {nullptr, 0};              // PB10: no ADC
        case AdcSampler::Signal::VinMboost: return {&hadc4, ADC_CHANNEL_5};   // PB15: ADC4_IN5
        // VinP1..VinP4, VinPboost
        case AdcSampler::Signal::VinP1:     return {&hadc2, ADC_CHANNEL_4};   // PA7:  ADC2_IN4
        case AdcSampler::Signal::VinP2:     return {&hadc1, ADC_CHANNEL_15};  // PB0:  ADC1_IN15
        case AdcSampler::Signal::VinP3:     return {&hadc3, ADC_CHANNEL_5};   // PB13: ADC3_IN5
        case AdcSampler::Signal::VinP4:     return {&hadc2, ADC_CHANNEL_14};  // PB11: ADC2_IN14
        case AdcSampler::Signal::VinPboost: return {&hadc2, ADC_CHANNEL_9};   // PC3:  ADC2_IN9
        // Vinj1..Vinj8
        case AdcSampler::Signal::Vinj1:     return {&hadc2, ADC_CHANNEL_6};   // PC0:  ADC2_IN6
        case AdcSampler::Signal::Vinj2:     return {&hadc2, ADC_CHANNEL_7};   // PC1:  ADC2_IN7
        case AdcSampler::Signal::Vinj3:     return {&hadc2, ADC_CHANNEL_8};   // PC2:  ADC2_IN8
        case AdcSampler::Signal::Vinj4:     return {&hadc1, ADC_CHANNEL_1};   // PA0:  ADC1_IN1
        case AdcSampler::Signal::Vinj5:     return {&hadc1, ADC_CHANNEL_2};   // PA1:  ADC1_IN2
        case AdcSampler::Signal::Vinj6:     return {&hadc1, ADC_CHANNEL_4};   // PA3:  ADC1_IN4
        case AdcSampler::Signal::Vinj7:     return {&hadc2, ADC_CHANNEL_17};  // PA4:  ADC2_IN17
        case AdcSampler::Signal::Vinj8:     return {&hadc2, ADC_CHANNEL_5};   // PC4:  ADC2_IN5
        default: return {nullptr, 0};
    }
}

} // namespace

AdcSampler::AdcSampler() : values_{0}, vddaMv_(3300) {}

AdcSampler::~AdcSampler() {
    (void)HAL_ADC_DeInit(&hadc1);
    (void)HAL_ADC_DeInit(&hadc2);
    (void)HAL_ADC_DeInit(&hadc3);
    (void)HAL_ADC_DeInit(&hadc4);
    (void)HAL_ADC_DeInit(&hadc5);
}

bool AdcSampler::init() {
    if (!initAdcCommonClocks()) {
        return false;
    }

    if (!initSingleAdc(hadc1, ADC1)) {
        return false;
    }
    if (!initSingleAdc(hadc2, ADC2)) {
        return false;
    }
    if (!initSingleAdc(hadc3, ADC3)) {
        return false;
    }
    if (!initSingleAdc(hadc4, ADC4)) {
        return false;
    }
    if (!initSingleAdc(hadc5, ADC5)) {
        return false;
    }

    uint32_t vddaMv = 0;
    if (readVrefintMv(vddaMv) && vddaMv >= 1500 && vddaMv <= 3600) {
        vddaMv_ = vddaMv;
    }

    return true;
}

void AdcSampler::sampleAll(uint32_t timeoutMs) {
    for (uint8_t i = 0; i < static_cast<uint8_t>(Signal::Count); ++i) {
        uint16_t v = 0;
        if (readOnce(static_cast<Signal>(i), timeoutMs, v)) {
            values_[i] = v;
        }
    }
}

uint16_t AdcSampler::raw(Signal s) const {
    const auto idx = static_cast<uint8_t>(s);
    if (idx >= static_cast<uint8_t>(Signal::Count)) {
        return 0;
    }
    return values_[idx];
}

uint32_t AdcSampler::millivolts(Signal s) const {
    const uint32_t r = raw(s);
    return (r * vddaMv_) / 4095U;
}

void AdcSampler::setVddaMillivolts(uint32_t vddaMv) {
    if (vddaMv == 0) {
        return;
    }
    vddaMv_ = vddaMv;
}

bool AdcSampler::readOnce(Signal s, uint32_t timeoutMs, uint16_t& out) {
    const ChannelSel sel = channelFor(s);
    if (!sel.hadc) {
        return false;
    }

    ADC_ChannelConfTypeDef config = {0};
    config.Channel = sel.channel;
    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLETIME_47CYCLES_5;
    config.SingleDiff = ADC_SINGLE_ENDED;
    config.OffsetNumber = ADC_OFFSET_NONE;
    config.Offset = 0;

    if (HAL_ADC_ConfigChannel(sel.hadc, &config) != HAL_OK) {
        return false;
    }
    if (HAL_ADC_Start(sel.hadc) != HAL_OK) {
        return false;
    }
    if (HAL_ADC_PollForConversion(sel.hadc, timeoutMs) != HAL_OK) {
        (void)HAL_ADC_Stop(sel.hadc);
        return false;
    }
    const uint32_t v = HAL_ADC_GetValue(sel.hadc);
    (void)HAL_ADC_Stop(sel.hadc);

    out = static_cast<uint16_t>(v & 0xFFFFU);
    return true;
}
