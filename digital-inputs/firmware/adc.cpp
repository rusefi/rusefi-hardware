
#include "adc.h"

/*
 * IN0 PF4  ADC3_IN14
 * IN1 PF5  ADC3_IN15
 * IN2 PF3  ADC3_IN9
 *
 */

#define ADC_DEVICE  ADCD3

#define ADC_GRP_BUF_DEPTH      8
#define SAMPLING_RATE ADC_SAMPLE_3

static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH];

static int adcConversionCounter = 0;

static void adccallback(ADCDriver *adcp) {

  if (adcIsBufferComplete(adcp)) {
    adcConversionCounter++;
  }
}

/*
 * ADC conversion group.
 * Mode:        Continuous, 8 samples of 3 channels, SW triggered.
 */
static const ADCConversionGroup adcgrpcfg = {
  .circular = TRUE,
  .num_channels = ADC_GRP_NUM_CHANNELS,
  .end_cb = adccallback,
  .error_cb = nullptr,
  .cr1 = 0,
  .cr2 = ADC_CR2_SWSTART,
  .smpr1 = ADC_SMPR1_SMP_AN14(SAMPLING_RATE) |
           ADC_SMPR1_SMP_AN15(SAMPLING_RATE),
  .smpr2 = ADC_SMPR2_SMP_AN9 (SAMPLING_RATE),
  .htr = 0,
  .ltr = 0,
  .sqr1= 0,
  .sqr2 = 0,
  .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN14)   |
          ADC_SQR3_SQ2_N(ADC_CHANNEL_IN15)   |
          ADC_SQR3_SQ3_N(ADC_CHANNEL_IN9),
};

static adcsample_t getAvgAdcValue(int index, adcsample_t *samples, int bufDepth, int numChannels) {
	uint32_t result = 0;
	for (int i = 0; i < bufDepth; i++) {
		result += samples[index];
		index += numChannels;
	}

	// this truncation is guaranteed to not be lossy - the average can't be larger than adcsample_t
	return static_cast<adcsample_t>(result / bufDepth);
}

adcsample_t getAdcRawValue(int channel) {
    return getAvgAdcValue(channel, samples, ADC_GRP_BUF_DEPTH, ADC_GRP_NUM_CHANNELS);
}

float getAdcValue(int channel) {
    return (float)getAdcRawValue(channel) * ADC_VREF / 4096;
}

void initAnalogInputs() {
  palSetPadMode(GPIOF, 3, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOF, 4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOF, 5, PAL_MODE_INPUT_ANALOG);

  adcStart(&ADC_DEVICE, NULL);
  adcSTM32EnableTSVREFE();
  /*
   * Starts an ADC continuous conversion.
   */
  adcStartConversion(&ADC_DEVICE, &adcgrpcfg, samples, ADC_GRP_BUF_DEPTH);
}
