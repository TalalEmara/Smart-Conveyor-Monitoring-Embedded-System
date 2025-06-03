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


    // Reset control registers
    ADC1->CR1 = 0;
    ADC1->CR2 = 0;

    // Set resolution to 12-bit
    ADC1->CR1 &= ~ADC_CR1_RES;

    // Right data alignment
    ADC1->CR2 &= ~ADC_CR2_ALIGN;

    // Disable continuous conversion by default
    ADC1->CR2 &= ~ADC_CR2_CONT;

    // Disable external trigger
    ADC1->CR2 &= ~ADC_CR2_EXTEN;

    // Regular sequence length = 1
    ADC1->SQR1 &= ~ADC_SQR1_L;

    // Enable EOC interrupt
    ADC1->CR1 |= ADC_CR1_EOCIE;

    // Enable ADC1 interrupt in NVIC
    NVIC_EnableIRQ(ADC_IRQn);

    // Power on ADC
    ADC1->CR2 |= ADC_CR2_ADON;

    // Delay for ADC stabilization
    for (volatile int i = 0; i < 1000; i++);

    adc_initialized = true;
    return ADC_OK;
}



ADC_Status_t ADC_Configure(ADC_Config_t *config) {
    if (!adc_initialized || config == NULL) {
        return ADC_ERROR;
    }

    // Set sampling time
    if (config->channel < 10) {
        uint32_t shift = config->channel * 3;
        ADC1->SMPR2 &= ~(0x7 << shift);
        ADC1->SMPR2 |= (config->sampling_time << shift);
    } else {
        uint32_t shift = (config->channel - 10) * 3;
        ADC1->SMPR1 &= ~(0x7 << shift);
        ADC1->SMPR1 |= (config->sampling_time << shift);
    }

    // Set continuous mode if enabled
    if (config->continuous_mode) {
        ADC1->CR2 |= ADC_CR2_CONT;
    } else {
        ADC1->CR2 &= ~ADC_CR2_CONT;
    }

    return ADC_OK;
}


/**
 * @brief Start ADC conversion for specified channel
 */
ADC_Status_t ADC_StartConversion(uint8_t channel) {
    if (!adc_initialized) {
        return ADC_ERROR;
    }
    
    // Check if ADC is busy
    if (ADC1->SR & ADC_SR_STRT) {
        return ADC_BUSY;
    }
    
    
    // Select the channel in regular sequence (first conversion)
    ADC1->SQR3 &= ~ADC_SQR3_SQ1;
    ADC1->SQR3 |= (channel << ADC_SQR3_SQ1_Pos);
    
    // Start conversion
    ADC1->CR2 |= ADC_CR2_SWSTART;
    
    return ADC_OK;
}


/**
 * @brief Convert ADC raw value to voltage
 */
float ADC_RawToVoltage(uint16_t raw_value) {
    return ((float)raw_value * ADC_REFERENCE_VOLTAGE) / (float)ADC_MAX_VALUE;
}

/**
 * @brief Convert ADC raw value to percentage
 */
uint8_t ADC_RawToPercentage(uint16_t raw_value) {
    // Clamp the value to maximum
    if (raw_value > ADC_MAX_VALUE) {
        raw_value = ADC_MAX_VALUE;
    }
    
    return (uint8_t)((raw_value * 100UL) / ADC_MAX_VALUE);
}


/**
 * @brief Enable ADC peripheral
 */
void ADC_Enable(void) {
    ADC1->CR2 |= ADC_CR2_ADON;
}

/**
 * @brief Disable ADC peripheral
 */
void ADC_Disable(void) {
    ADC1->CR2 &= ~ADC_CR2_ADON;
}

/**
 * @brief Get potentiometer value as percentage for motor speed control
 */
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

void ADC_IRQHandler(void) {
    if (ADC1->SR & ADC_SR_EOC) {
        // Read the result and clear EOC flag by reading ADC1->DR
        adc_result = ADC1->DR & 0xFFF;
        adc_new_data_available = true;
    }
}
