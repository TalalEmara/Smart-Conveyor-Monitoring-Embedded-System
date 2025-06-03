#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <stdbool.h>

// ADC Configuration Constants
#define ADC_MAX_VALUE           4095    // 12-bit ADC (2^12 - 1)
#define ADC_REFERENCE_VOLTAGE   3.3f    // Reference voltage in volts
#define ADC_RESOLUTION          12      // 12-bit resolution

// ADC Channel Definitions
// #define ADC_POTENTIOMETER_CHANNEL   0   // Channel 0 for potentiometer

// ADC Status
typedef enum {
    ADC_OK = 0,
    ADC_ERROR,
    ADC_BUSY,
    ADC_TIMEOUT
} ADC_Status_t;

// ADC Configuration Structure
typedef struct {
    uint8_t channel;        // ADC channel number
    uint8_t sampling_time; 
    bool continuous_mode;  
} ADC_Config_t;


ADC_Status_t ADC_Init(void);
ADC_Status_t ADC_Configure(ADC_Config_t *config);
ADC_Status_t ADC_StartConversion(uint8_t channel);
ADC_Status_t ADC_ReadValue(uint8_t channel, uint16_t *result);
ADC_Status_t ADC_ReadBlocking(uint8_t channel, uint16_t *result);

/**
 * @brief Convert ADC raw value to voltage
 * @param raw_value: Raw ADC value (0-4095)
 * @return float - Voltage value in volts
 */
float ADC_RawToVoltage(uint16_t raw_value);

/**
 * @brief Convert ADC raw value to percentage (0-100%)
 * @param raw_value: Raw ADC value (0-4095)
 * @return uint8_t - Percentage value (0-100)
 */
uint8_t ADC_RawToPercentage(uint16_t raw_value);

/**
 * @brief Check if ADC conversion is complete
 * @param None
 * @return bool - true if conversion complete, false otherwise
 */
bool ADC_IsConversionComplete(void);

void ADC_Enable(void);

/**
 * @brief Disable ADC peripheral
 * @param None
 * @return None
 */
void ADC_Disable(void);
ADC_Status_t ADC_GetPotentiometerPercentage(uint8_t *percentage);

#endif // ADC_H