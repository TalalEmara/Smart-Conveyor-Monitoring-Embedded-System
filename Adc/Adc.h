#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <stdbool.h>

// ADC Configuration Constants
#define ADC_MAX_VALUE           4095    // 12-bit ADC
#define ADC_REFERENCE_VOLTAGE   3.3f    // Reference voltage in volts
#define ADC_RESOLUTION          12

// ADC Status Codes
typedef enum {
    ADC_OK = 0,
    ADC_ERROR,
    ADC_BUSY,
    ADC_TIMEOUT
} ADC_Status_t;

// ADC Configuration Structure
typedef struct {
    uint8_t channel;          // ADC channel number (0-15)
    uint8_t sampling_time;    // Sampling time configuration (0-7)
    bool continuous_mode;     // Not used in polling mode, can be false
} ADC_Config_t;

// Core API
ADC_Status_t ADC_Init(void);
ADC_Status_t ADC_Configure(ADC_Config_t *config);
ADC_Status_t ADC_StartConversion(uint8_t channel);
uint16_t ADC_ReadValue(void);
uint16_t ADC_ReadBlocking(uint8_t channel);

// Conversion Utilities
float ADC_RawToVoltage(uint16_t raw_value);
uint8_t ADC_RawToPercentage(uint16_t raw_value);

// Optional utilities
bool ADC_IsConversionComplete(void);
void ADC_Enable(void);
void ADC_Disable(void);

#endif // ADC_H
