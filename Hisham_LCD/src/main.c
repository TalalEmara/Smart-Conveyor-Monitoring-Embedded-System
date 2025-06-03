#include "RCC.h"
#include "Gpio.h"
#include "Adc.h"
#include "lcd.h"
#include "Std_Types.h"
#include "pwm.h"
#include <stdio.h>

volatile uint8 counter = 0x00;
volatile uint8 button = 0;
volatile uint8 toggle = 1;

#define NUMBER_OF_CYCLES    1000000

extern volatile uint16_t adc_result;
extern volatile bool adc_new_data_available;

void delay_millis(uint32_t delay) {
    for (volatile uint32_t i = 0; i < (NUMBER_OF_CYCLES / 1000) * delay; i++) {
        __asm__("nop");
    }
}

// Simple function to convert float to string (voltage)
void float_to_string(float value, char* buffer, uint8_t decimal_places) {
    int integer_part = (int)value;
    int fractional_part = (int)((value - integer_part) * 100); // 2 decimal places

    // Convert integer part
    if (integer_part == 0) {
        buffer[0] = '0';
        buffer[1] = '.';
    } else {
        char temp[10];
        int index = 0;
        int temp_int = integer_part;

        // Extract digits
        while (temp_int > 0) {
            temp[index++] = '0' + (temp_int % 10);
            temp_int /= 10;
        }

        // Reverse and copy
        int buf_index = 0;
        for (int i = index - 1; i >= 0; i--) {
            buffer[buf_index++] = temp[i];
        }
        buffer[buf_index++] = '.';
    }

    // Add fractional part
    int buf_len = 0;
    while (buffer[buf_len] != '.') buf_len++;
    buf_len++; // Move past the decimal point

    buffer[buf_len++] = '0' + (fractional_part / 10);
    buffer[buf_len++] = '0' + (fractional_part % 10);
    buffer[buf_len++] = ' ';
    buffer[buf_len++] = 'V';
    buffer[buf_len] = '\0';
}

// Simple function to convert integer to string
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

    // Reverse
    for (int i = 0; i < index; i++) {
        buffer[i] = temp[index - 1 - i];
    }
    buffer[index] = '\0';
}

int main(void) {
    // Initialize RCC first
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);

    // Add ADC clock enable if not done in Rcc_Init()
    // Make sure your RCC module enables ADC1 clock, or add it manually:
    // RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    // Initialize the pins
    Gpio_Init(GPIO_A, 0, GPIO_ANALOG, GPIO_NO_PULL_DOWN); // ADC pin for potentiometer
    Gpio_Init(GPIO_B, 0, GPIO_OUTPUT, GPIO_PUSH_PULL); // PWM output pin

    // Initialize the LCD first and test it
    LCD_Init();
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("Initializing...");
    delay_millis(500); // Give some time to see this message

    // Initialize PWM
    PWM_Init();

    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("PWM OK");
    delay_millis(500);

    // Init and configure ADC with debug messages
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("ADC Init...     "); // Clear previous message

    ADC_Status_t adc_status = ADC_Init();
    if (adc_status != ADC_OK) {
        LCD_SetCursor(LCD_ROW_1, 0);
        LCD_PrintString("ADC Init FAIL");
        while(1); // Stop here if ADC init fails
    }

    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("ADC Init OK   ");
    delay_millis(500);

    ADC_Config_t config = {
        .channel = 0,            // PA0 = ADC1_IN0
        .sampling_time = 5,      // 84 cycles
        .continuous_mode = true
    };

    adc_status = ADC_Configure(&config);
    if (adc_status != ADC_OK) {
        LCD_SetCursor(LCD_ROW_1, 0);
        LCD_PrintString("ADC Cfg FAIL");
        while(1); // Stop here if ADC config fails
    }

    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("ADC Cfg OK    ");
    delay_millis(500);

    adc_status = ADC_StartConversion(config.channel);
    if (adc_status != ADC_OK) {
        LCD_SetCursor(LCD_ROW_1, 0);
        LCD_PrintString("ADC Start FAIL");
        while(1); // Stop here if ADC start fails
    }

    // Clear LCD and show normal operation
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("Voltage:        ");
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("Speed:          ");

    uint8_t duty;
    char voltage_str[16];
    char duty_str[8];
    char speed_display[16];
    uint32_t loop_counter = 0;

    while (1) {
        // Convert raw ADC value to voltage
        float voltage = ADC_RawToVoltage(adc_result);

        // Convert voltage to string manually
        float_to_string(voltage, voltage_str, 2);

        duty = (uint8_t)(adc_result * 100 / 4095);    // Convert to 0-100% duty cycle

        PWM_SetDutyCycle(duty);              // Set PWM output
        LCD_SetCursor(LCD_ROW_0, 9);         // Set cursor to the first row, after "Voltage: "
        LCD_PrintString(voltage_str);        // Print voltage value

        LCD_SetCursor(LCD_ROW_1, 0);
        // Manually create "Speed: XXX%" string
        speed_display[0] = 'S';
        speed_display[1] = 'p';
        speed_display[2] = 'e';
        speed_display[3] = 'e';
        speed_display[4] = 'd';
        speed_display[5] = ':';
        speed_display[6] = ' ';

        // Convert duty to string and add to display
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

        // Debug: Show that we're in the main loop
        if (loop_counter % 10 == 0) {  // Every second
            LCD_SetCursor(LCD_ROW_1, 14);
            LCD_PrintString((loop_counter / 10) % 2 ? "*" : " ");
        }
    }

    return 0;
}