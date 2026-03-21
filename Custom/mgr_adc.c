#include "mgr_adc.h"

extern ADC_HandleTypeDef hadc1;

extern DMA_HandleTypeDef hdma_adc1;

uint32_t adc_data[NUM_ADC_CHANNELS];

void adc_init(void)
{
    HAL_ADC_Start_DMA(&hadc1,adc_data,NUM_ADC_CHANNELS);
}

uint32_t* adc_GetAdcBufferPtr(void)
{
    return adc_data;
}

uint16_t adc_GetRawAdcValue(uint8_t channel)
{
    if (channel < NUM_ADC_CHANNELS)
    {
        return (uint16_t)(adc_data[channel]);
    }
    else
    {
        return 0;
    }
}
