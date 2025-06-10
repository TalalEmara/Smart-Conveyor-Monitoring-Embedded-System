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

#define ADC1_BASE       (0x40012000UL)
#define ADC1            ((ADC_TypeDef *)ADC1_BASE)

// ADC register bit defines (copied from stm32f401xc.h for standalone portability)
#define ADC_CR1_RES_Pos        24
#define ADC_CR1_RES_Msk        (0x3UL << ADC_CR1_RES_Pos)
#define ADC_CR1_RES            ADC_CR1_RES_Msk

#define ADC_CR2_ALIGN          (1UL << 11)
#define ADC_CR2_CONT           (1UL << 1)
#define ADC_CR2_EXTEN          (0x3UL << 28)
#define ADC_CR2_ADON           (1UL << 0)
#define ADC_CR2_SWSTART        (1UL << 30)

#define ADC_SR_EOC             (1UL << 1)

#define ADC_SQR1_L             (0xF << 20)
#define ADC_SQR3_SQ1_Pos       0
#define ADC_SQR3_SQ1           (0x1F << ADC_SQR3_SQ1_Pos)

#define ADC_DR_DATA            0xFFF


// ADC Registers
typedef struct
{
uint32_t SR;     /*!< ADC status register,                         Address offset: 0x00 */
uint32_t CR1;    /*!< ADC control register 1,                      Address offset: 0x04 */
uint32_t CR2;    /*!< ADC control register 2,                      Address offset: 0x08 */
uint32_t SMPR1;  /*!< ADC sample time register 1,                  Address offset: 0x0C */
uint32_t SMPR2;  /*!< ADC sample time register 2,                  Address offset: 0x10 */
uint32_t JOFR1;  /*!< ADC injected channel data offset register 1, Address offset: 0x14 */
uint32_t JOFR2;  /*!< ADC injected channel data offset register 2, Address offset: 0x18 */
uint32_t JOFR3;  /*!< ADC injected channel data offset register 3, Address offset: 0x1C */
uint32_t JOFR4;  /*!< ADC injected channel data offset register 4, Address offset: 0x20 */
uint32_t HTR;    /*!< ADC watchdog higher threshold register,      Address offset: 0x24 */
uint32_t LTR;    /*!< ADC watchdog lower threshold register,       Address offset: 0x28 */
uint32_t SQR1;   /*!< ADC regular sequence register 1,             Address offset: 0x2C */
uint32_t SQR2;   /*!< ADC regular sequence register 2,             Address offset: 0x30 */
uint32_t SQR3;   /*!< ADC regular sequence register 3,             Address offset: 0x34 */
uint32_t JSQR;   /*!< ADC injected sequence register,              Address offset: 0x38*/
uint32_t JDR1;   /*!< ADC injected data register 1,                Address offset: 0x3C */
uint32_t JDR2;   /*!< ADC injected data register 2,                Address offset: 0x40 */
uint32_t JDR3;   /*!< ADC injected data register 3,                Address offset: 0x44 */
uint32_t JDR4;   /*!< ADC injected data register 4,                Address offset: 0x48 */
uint32_t DR;     /*!< ADC regular data register,                   Address offset: 0x4C */
} ADC_TypeDef;


ADC_Status_t ADC_Init(void);
ADC_Status_t ADC_Configure(ADC_Config_t *config);
ADC_Status_t ADC_StartConversion(uint8_t channel);
uint16_t ADC_ReadValue();
uint16_t ADC_ReadBlocking(uint8_t channel);
float ADC_RawToVoltage(uint16_t raw_value);
uint8_t ADC_RawToPercentage(uint16_t raw_value);
bool ADC_IsConversionComplete(void);
void ADC_Enable(void);
void ADC_Disable();

#endif // ADC_H