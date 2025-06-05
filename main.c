#include "RCC.h"
#include "Gpio.h"
#include "Adc.h"
#include "lcd.h"
#include "Std_Types.h"
#include "pwm.h"
#include <stdio.h>

#define NUMBER_OF_CYCLES 1000000
#define POTENTIOMETER_ADC_CHANNEL 10  // PC0 is ADC1_IN10

void delay_millis(uint32_t delay) {
    for (volatile uint32_t i = 0; i < (NUMBER_OF_CYCLES / 1000) * delay; i++);
}

void float_to_string(float value, char* buffer, uint8_t decimal_places) {
    int integer_part = (int)value;
    int fractional_part = (int)((value - integer_part) * 100);

    char temp[10];
    int index = 0, buf_index = 0;

    if (integer_part == 0) {
        buffer[buf_index++] = '0';
    } else {
        while (integer_part > 0) {
            temp[index++] = '0' + (integer_part % 10);
            integer_part /= 10;
        }
        for (int i = index - 1; i >= 0; i--) buffer[buf_index++] = temp[i];
    }

    buffer[buf_index++] = '.';
    buffer[buf_index++] = '0' + (fractional_part / 10);
    buffer[buf_index++] = '0' + (fractional_part % 10);
    buffer[buf_index++] = ' ';
    buffer[buf_index++] = 'V';
    buffer[buf_index] = '\0';
}

void int_to_string(int value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    char temp[10];
    int index = 0;
    while (value > 0) {
        temp[index++] = '0' + (value % 10);
        value /= 10;
    }
    for (int i = 0; i < index; i++) {
        buffer[i] = temp[index - 1 - i];
    }
    buffer[index] = '\0';
}



int main(void) {
    // Initialize system clocks
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);
    Rcc_Enable(RCC_ADC1);

    // Initialize GPIOs
    Gpio_Init(GPIO_C, 0, GPIO_ANALOG, GPIO_NO_PULL_DOWN); // Potentiometer on PC0
    Gpio_Init(GPIO_B, 0, GPIO_OUTPUT, GPIO_PUSH_PULL);     // PWM output on PB0

    // Initialize LCD
    LCD_Init();
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("Initializing...");
    delay_millis(500);

    // Initialize PWM
    PWM_Init();
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("PWM OK");
    delay_millis(500);


    ADC1->CR1 = 0;
    ADC1->CR2 = 0;
    delay_millis(1);

    ADC1->CR1 &= ~ADC_CR1_RES;       // 12-bit resolution (00)
    ADC1->CR2 &= ~ADC_CR2_ALIGN;     // Right alignment
    ADC1->CR2 &= ~ADC_CR2_CONT;      // Single conversion mode
    ADC1->CR1 &= ~ADC_CR1_SCAN;      // Disable scan mode
    ADC1->SQR1 = 0;                  // One conversion in sequence


    ADC1->SMPR1 &= ~(0x7 << 0);      // Clear channel 10 sampling time bits
    ADC1->SMPR1 |= (0x7 << 0);       // Set maximum sampling time (480 cycles)

    // Power on ADC
    ADC1->CR2 |= ADC_CR2_ADON;
    delay_millis(2);

    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("ADC Init OK   ");
    delay_millis(500);

    uint16_t test_val1 = ADC_ReadBlocking(POTENTIOMETER_ADC_CHANNEL);
    delay_millis(10);
    uint16_t test_val2 = ADC_ReadBlocking(POTENTIOMETER_ADC_CHANNEL);

    char test_str[16];
    sprintf(test_str, "T1:%4d T2:%4d", test_val1, test_val2);
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("                "); // Clear line
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString(test_str);
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("Turn pot & wait");
    delay_millis(3000);

    // Main display setup
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("Voltage:        ");
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("Speed:          ");

    char voltage_str[16];
    char speed_display[16];

    while (1) {
        uint16_t raw_value = ADC_ReadBlocking(POTENTIOMETER_ADC_CHANNEL);
        if (raw_value > 4095) raw_value = 4095;

        float voltage = (raw_value * 3.3f) / 4095.0f;
        uint8_t duty = (uint8_t)((raw_value * 100.0f) / 4095.0f);

        PWM_SetDutyCycle(duty);

        // Display voltage
        float_to_string(voltage, voltage_str, 2);
        LCD_SetCursor(LCD_ROW_0, 9);
        LCD_PrintString(voltage_str);

        // Display speed
        LCD_SetCursor(LCD_ROW_1, 0);
        sprintf(speed_display, "Speed: %3d%%", duty);
        LCD_PrintString(speed_display);

        delay_millis(100);
    }


    return 0;
}