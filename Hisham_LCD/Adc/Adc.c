#include "Adc.h"
#include "stm32f401xc.h"
#include <stddef.h>

// Private variables
static bool adc_initialized = false;

volatile uint16_t adc_result = 0;
volatile bool adc_new_data_available = false;

ADC_Status_t ADC_Init(void) {
    if (adc_initialized) {
        return ADC_OK;
    }

    // Enable ADC1 clock first (this might be missing in your RCC setup)
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    // Small delay after enabling clock
    for (volatile int i = 0; i < 100; i++);

    // Reset control registers
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    // Set resolution to 12-bit (bits should be 00 for 12-bit)
    ADC1->CR1 &= ~ADC_CR1_RES;

    // Right data alignment (bit should be 0)
    ADC1->CR2 &= ~ADC_CR2_ALIGN;

    // Disable continuous conversion by default
    ADC1->CR2 &= ~ADC_CR2_CONT;

    // Disable external trigger
    ADC1->CR2 &= ~ADC_CR2_EXTEN;

    // Regular sequence length = 1 (bits should be 000 for 1 conversion)
    ADC1->SQR1 &= ~ADC_SQR1_L;

    // Enable EOC interrupt
    ADC1->CR1 |= ADC_CR1_EOCIE;

    // Enable ADC1 interrupt in NVIC with proper priority
    NVIC_SetPriority(ADC_IRQn, 5);  // Set priority
    NVIC_EnableIRQ(ADC_IRQn);

    // Power on ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    // Wait for ADC to be ready (important!)
    // ADC needs time to stabilize after being powered on
    for (volatile int i = 0; i < 10000; i++);

    adc_initialized = true;
    return ADC_OK;
}

ADC_Status_t ADC_Configure(ADC_Config_t *config) {
    if (!adc_initialized || config == NULL) {
        return ADC_ERROR;
    }

    // Validate channel number
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

    // Set continuous mode if enabled
    if (config->continuous_mode) {
        ADC1->CR2 |= ADC_CR2_CONT;
    } else {
        ADC1->CR2 &= ~ADC_CR2_CONT;
    }

    return ADC_OK;
}

ADC_Status_t ADC_StartConversion(uint8_t channel) {
    if (!adc_initialized) {
        return ADC_ERROR;
    }

    // Validate channel
    if (channel > 15) {
        return ADC_ERROR;
    }

    // For continuous mode, check if conversion is already running
    if (ADC1->CR2 & ADC_CR2_CONT) {
        // In continuous mode, just check if ADC is enabled
        if (!(ADC1->CR2 & ADC_CR2_ADON)) {
            return ADC_ERROR;
        }
    } else {
        // For single conversion, check if busy
        if (ADC1->SR & ADC_SR_STRT) {
            return ADC_BUSY;
        }
    }

    // Select the channel in regular sequence (first conversion)
    ADC1->SQR3 &= ~ADC_SQR3_SQ1;
    ADC1->SQR3 |= ((uint32_t)channel << ADC_SQR3_SQ1_Pos);

    // Start conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;

    return ADC_OK;
}

float ADC_RawToVoltage(uint16_t raw_value) {
    return ((float)raw_value * ADC_REFERENCE_VOLTAGE) / (float)ADC_MAX_VALUE;
}

uint8_t ADC_RawToPercentage(uint16_t raw_value) {
    // Clamp the value to maximum
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

ADC_Status_t ADC_GetPotentiometerPercentage(uint8_t *percentage) {
    if (percentage == NULL) {
        return ADC_ERROR;
    }

    // Check if new data is available
    if (!adc_new_data_available) {
        return ADC_BUSY;
    }

    // Clear the data available flag
    adc_new_data_available = false;

    // Convert the latest ADC result to percentage
    *percentage = ADC_RawToPercentage(adc_result);

    return ADC_OK;
}

// Make sure this interrupt handler name matches your vector table
void ADC_IRQHandler(void) {
    // Check for End of Conversion
    if (ADC1->SR & ADC_SR_EOC) {
        // Read the result (this also clears EOC flag automatically)
        adc_result = ADC1->DR & 0xFFF;  // Mask to 12 bits
        adc_new_data_available = true;

        // Clear EOC flag explicitly (in case it doesn't clear automatically)
        ADC1->SR &= ~ADC_SR_EOC;
    }

    // Check for overrun error
    if (ADC1->SR & ADC_SR_OVR) {
        // Clear the overrun flag
        ADC1->SR &= ~ADC_SR_OVR;
        // You might want to set an error flag here
    }
}