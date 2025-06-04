#include "RCC.h"
#include "Gpio.h"
#include "Adc.h"
#include "lcd.h"
#include "Std_Types.h"
#include "pwm.h"
#include <stdio.h>

#define NUMBER_OF_CYCLES 1000000

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
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);
    Rcc_Enable(RCC_ADC1);


    Gpio_Init(GPIO_C, 0, GPIO_ANALOG, GPIO_NO_PULL_DOWN); 
    Gpio_Init(GPIO_B, 0, GPIO_OUTPUT, GPIO_PUSH_PULL);

    LCD_Init();
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("Initializing...");
    delay_millis(500);

    PWM_Init();
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("PWM OK");
    delay_millis(500);

    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("ADC Init...     ");

    if (ADC_Init() != ADC_OK) {
        LCD_SetCursor(LCD_ROW_1, 0);
        LCD_PrintString("ADC Init FAIL");
        while (1);
    }

    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("ADC Init OK   ");
    delay_millis(500);

    ADC_Config_t config = {
        .channel = 0,
        .sampling_time = 5,
        .continuous_mode = false // Not needed in polling
    };

    if (ADC_Configure(&config) != ADC_OK) {
        LCD_SetCursor(LCD_ROW_1, 0);
        LCD_PrintString("ADC Cfg FAIL");
        while (1);
    }

    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("ADC Cfg OK    ");
    delay_millis(500);

    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("Voltage:        ");
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("Speed:          ");

    char voltage_str[16];
    char duty_str[8];
    char speed_display[16];
    uint32_t loop_counter = 0;

    while (1) {
        uint16_t raw_value = ADC_ReadBlocking(0);
        float voltage = ADC_RawToVoltage(raw_value);
        uint8_t duty = (uint8_t)(raw_value * 100 / 4095);

        PWM_SetDutyCycle(duty);
        float_to_string(voltage, voltage_str, 2);
        LCD_SetCursor(LCD_ROW_0, 9);
        LCD_PrintString(voltage_str);

        LCD_SetCursor(LCD_ROW_1, 0);
        speed_display[0] = 'S'; speed_display[1] = 'p';
        speed_display[2] = 'e'; speed_display[3] = 'e';
        speed_display[4] = 'd'; speed_display[5] = ':'; speed_display[6] = ' ';

        int_to_string(duty, duty_str);
        int i = 0;
        while (duty_str[i] != '\0') {
            speed_display[7 + i] = duty_str[i];
            i++;
        }
        speed_display[7 + i] = '%';
        speed_display[8 + i] = '\0';

        LCD_PrintString(speed_display);

        delay_millis(100);
        loop_counter++;

        if (loop_counter % 10 == 0) {
            LCD_SetCursor(LCD_ROW_1, 14);
            LCD_PrintString((loop_counter / 10) % 2 ? "*" : " ");
        }
    }

    return 0;
}
