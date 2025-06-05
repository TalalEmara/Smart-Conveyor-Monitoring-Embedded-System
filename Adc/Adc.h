#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include <stdbool.h>

#define ADC_MAX_VALUE           4095    // 12-bit ADC max value
#define ADC_REFERENCE_VOLTAGE   3.3f    // Reference voltage in volts

typedef enum {
    ADC_OK = 0,
    ADC_ERROR,
    ADC_BUSY,
    ADC_TIMEOUT
} ADC_Status_t;

typedef struct {
    uint8_t channel;        // 0-18 ADC channel
    uint8_t sampling_time;  // 0-7, ADC_SMPRx sampling time config
    bool continuous_mode;   // false = single conversion mode
} ADC_Config_t;

ADC_Status_t ADC_Init(void);
ADC_Status_t ADC_Configure(ADC_Config_t *config);
ADC_Status_t ADC_StartConversion(uint8_t channel);
bool ADC_IsConversionComplete(void);
uint16_t ADC_ReadValue(void);
uint16_t ADC_ReadBlocking(uint8_t channel);

float ADC_RawToVoltage(uint16_t raw_value);
uint8_t ADC_RawToPercentage(uint16_t raw_value);

void ADC_Enable(void);
void ADC_Disable(void);

#endif // ADC_H
