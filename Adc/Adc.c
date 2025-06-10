#include "adc.h"
#include "stm32f401xc.h"
#include <stddef.h>  // Include for NULL definition
#include <stdbool.h>

static bool adc_initialized = false;

ADC_Status_t ADC_Init(void) {
    if (adc_initialized) {
        return ADC_OK;
    }

    // Reset ADC registers
    ADC1->CR1 = 0;  // Control Register 1 (used for resolution, scan mode, etc.)
    ADC1->CR2 = 0; // Control Register 2 (used for trigger, alignment, etc.)

    // Wait briefly for reset
    for (volatile int i = 0; i < 1000; i++);

    // ADC configuration
    ADC1->CR1 &= ~ADC_CR1_RES;       // 12-bit resolution (bits 24:25 = 00)
    ADC1->CR2 &= ~ADC_CR2_ALIGN;     // Right alignment
    ADC1->CR2 &= ~ADC_CR2_CONT;      // Single conversion mode
    ADC1->CR2 &= ~ADC_CR2_EXTEN;     // Disable external trigger
    ADC1->SQR1 &= ~ADC_SQR1_L;       // One conversion in sequence

    // Default sampling time for channel 0 (84 cycles)
    // We'll override this per channel on ADC_Configure
    ADC1->SMPR2 &= ~(0x7 << (0 * 3));
    ADC1->SMPR2 |= (0x5 << (0 * 3));

    // Power on ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    // Wait for ADC stabilization (~10us or so)
    for (volatile int i = 0; i < 10000; i++);

    adc_initialized = true;
    return ADC_OK;
}

ADC_Status_t ADC_Configure(ADC_Config_t *config) {
    if (!adc_initialized || config == NULL) {
        return ADC_ERROR;
    }

    if (config->channel > 18) { // STM32F4 has channels up to 18 (internal temp sensor, etc)
        return ADC_ERROR;
    }

    // Configure sampling time for the requested channel
    if (config->channel <= 9) {
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
    if (!adc_initialized || channel > 18) {
        return ADC_ERROR;
    }

    // Set the channel for the first conversion in the regular sequence
    ADC1->SQR3 &= ~ADC_SQR3_SQ1;  // Clear bits
    ADC1->SQR3 |= ((uint32_t)channel << ADC_SQR3_SQ1_Pos);

    ADC1->SR &= ~ADC_SR_EOC; // Clear End Of Conversion flag

    ADC1->CR2 |= ADC_CR2_SWSTART; // Start conversion

    return ADC_OK;
}

bool ADC_IsConversionComplete(void) {
    return (ADC1->SR & ADC_SR_EOC) != 0;
}

uint16_t ADC_ReadValue(void) {
    return (uint16_t)(ADC1->DR & 0xFFF); // 12-bit data is lower 12 bits
}


uint16_t ADC_ReadBlocking(uint8_t channel) {
    // Clear sequence register and set channel
    ADC1->SQR3 = 0;
    ADC1->SQR3 |= (channel << 0);

    // Clear any previous EOC flag
    ADC1->SR &= ~ADC_SR_EOC;

    // Start conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;

    // Wait for conversion to complete
    while (!(ADC1->SR & ADC_SR_EOC));

    // Read and return the result
    return (ADC1->DR & 0xFFF);
}
float ADC_RawToVoltage(uint16_t raw_value) {
    if(raw_value > ADC_MAX_VALUE) raw_value = ADC_MAX_VALUE;
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
