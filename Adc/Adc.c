#include "Adc.h"
#include "stm32f401xc.h"
#include <stddef.h>

// Private variables
static bool adc_initialized = false;

ADC_Status_t ADC_Init(void) {
    if (adc_initialized) {
        return ADC_OK;
    }

    // Reset ADC
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    // Wait for reset to complete
    for (volatile int i = 0; i < 1000; i++);

    // Configure ADC:
    ADC1->CR1 &= ~ADC_CR1_RES;            // 12-bit resolution
    ADC1->CR2 &= ~ADC_CR2_ALIGN;          // Right alignment
    ADC1->CR2 &= ~ADC_CR2_CONT;           // Single conversion mode
    ADC1->CR2 &= ~ADC_CR2_EXTEN;          // No external trigger
    ADC1->SQR1 &= ~ADC_SQR1_L;            // Sequence length = 1

    // Default sampling time for channel 0 (84 cycles = 5)
    ADC1->SMPR2 &= ~(0x7 << (0 * 3));
    ADC1->SMPR2 |= (0x5 << (0 * 3));

    // Power on ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    // Wait for ADC to stabilize
    for (volatile int i = 0; i < 10000; i++);

    adc_initialized = true;
    return ADC_OK;
}

ADC_Status_t ADC_Configure(ADC_Config_t *config) {
    if (!adc_initialized || config == NULL) {
        return ADC_ERROR;
    }

    if (config->channel > 15) {
        return ADC_ERROR;
    }

    // Set sampling time
    if (config->channel < 10) {
        uint32_t shift = config->channel * 3;
        ADC1->SMPR2 &= ~(0x7UL << shift);
        ADC1->SMPR2 |= ((uint32_t)config->sampling_time << shift);
    } else {
        uint32_t shift = (config->channel - 10) * 3;
        ADC1->SMPR1 &= ~(0x7UL << shift);
        ADC1->SMPR1 |= ((uint32_t)config->sampling_time << shift);
    }

    if (config->continuous_mode) {
        ADC1->CR2 |= ADC_CR2_CONT;
    } else {
        ADC1->CR2 &= ~ADC_CR2_CONT;
    }

    return ADC_OK;
}

ADC_Status_t ADC_StartConversion(uint8_t channel) {
    if (!adc_initialized || channel > 15) {
        return ADC_ERROR;
    }

    ADC1->SQR3 &= ~ADC_SQR3_SQ1;
    ADC1->SQR3 |= ((uint32_t)channel << ADC_SQR3_SQ1_Pos);

    ADC1->SR &= ~ADC_SR_EOC; // Clear EOC flag
    ADC1->CR2 |= ADC_CR2_SWSTART;

    return ADC_OK;
}

bool ADC_IsConversionComplete(void) {
    return (ADC1->SR & ADC_SR_EOC) != 0;
}

uint16_t ADC_ReadValue(void) {
    return (ADC1->DR & 0xFFF);
}

uint16_t ADC_ReadBlocking(uint8_t channel) {
    ADC_StartConversion(channel);
    while (!ADC_IsConversionComplete());
    return ADC_ReadValue();
}

float ADC_RawToVoltage(uint16_t raw_value) {
    return ((float)raw_value * ADC_REFERENCE_VOLTAGE) / (float)ADC_MAX_VALUE;
}

uint8_t ADC_RawToPercentage(uint16_t raw_value) {
    if (raw_value > ADC_MAX_VALUE) {
        raw_value = ADC_MAX_VALUE;
    }
    return (uint8_t)((raw_value * 100UL) / ADC_MAX_VALUE);
}

void ADC_Enable(void) {
    ADC1->CR2 |= ADC_CR2_ADON;
}

void ADC_Disable(void) {
    ADC1->CR2 &= ~ADC_CR2_ADON;
}
