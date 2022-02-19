
#include "adc.h"

/*
 * IN0 PF4  ADC3_IN14
 * IN1 PF5  ADC3_IN15
 * IN2 PF3  ADC3_IN9
 *
 */


#define ADC_GRP_BUF_DEPTH      8

static adcsample_t samples[ADC_GRP_NUM_CHANNELS * ADC_GRP_BUF_DEPTH];

static void adccallback(ADCDriver *adcp) {

  if (adcIsBufferComplete(adcp)) {
  }
}

/*
 * ADC conversion group.
 * Mode:        Continuous, 8 samples of 3 channels, SW triggered.
 * Channels:    IN11, IN12, IN11, IN12, IN11, IN12, Sensor, VRef.
 */
static const ADCConversionGroup adcgrpcfg = {
  /*.circular = */TRUE,
  /*.num_channels = */ADC_GRP_NUM_CHANNELS,
  adccallback,
  nullptr,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN14(ADC_SAMPLE_56) |
  ADC_SMPR1_SMP_AN15(ADC_SAMPLE_56),
  ADC_SMPR2_SMP_AN9 (ADC_SAMPLE_56),  /* SMPR2 */
  /*.htr = */0,                        /* HTR */
  /*.ltr = */0,                        /* LTR */
  /*.sqr1= */0,                        /* SQR1 */
  /*.sqr2 = */ADC_SQR3_SQ3_N(ADC_CHANNEL_IN9)    |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN14)   |
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN15),
  /*.sqr3 = */0
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

adcsample_t getAdcValue(int channel) {
    return getAvgAdcValue(channel, samples, ADC_GRP_BUF_DEPTH, ADC_GRP_NUM_CHANNELS);
}

void initAnalogInputs() {
  /*
   * Starts an ADC continuous conversion.
   */
  adcStartConversion(&ADCD3, &adcgrpcfg, samples, ADC_GRP_BUF_DEPTH);

}