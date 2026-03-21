#ifndef MGR_ADC
#define MGR_ADC

#include "adc.h"
#include "appl_main.h"

#define NUM_ADC_CHANNELS 5

void adc_init(void);
uint16_t adc_GetRawAdcValue(uint8_t channel);


#endif