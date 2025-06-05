#include "RCC.h"
#include "Gpio.h"
#include "Adc.h"
#include "lcd.h"
#include "pwm.h"
#include "EXTI.h"
#include <stdio.h>

#define NUMBER_OF_CYCLES 1000000
#define POTENTIOMETER_ADC_CHANNEL 10
#define DEBOUNCE_DELAY_MS 50

#define IR_BUTTON_PORT GPIO_A
#define IR_BUTTON_PIN  15

#define EMERGENCY_STOP_PIN 8  // PA8
#define RESET_BUTTON_PIN   9  // PA9

volatile uint8_t emergencyStop = 0;
volatile uint32_t system_ms = 0;
volatile uint8_t object_count = 0;

uint8_t duty = 0;
float prev_voltage = -1.0f;

void delay_millis(uint32_t delay) {
    for (volatile uint32_t i = 0; i < (NUMBER_OF_CYCLES / 1000) * delay; i++);
    system_ms += delay;
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

void LCD_PrintStatus(void) {
    if (emergencyStop) {
        LCD_SetCursor(0, 0);
        LCD_PrintString("!!! EMERGENCY !!!");
        LCD_SetCursor(1, 0);
        LCD_PrintString("SYSTEM STOPPED  ");
    } else {
        LCD_SetCursor(0, 0);
        LCD_PrintString("Voltage:        ");
        LCD_SetCursor(1, 0);
        char buf[16];
        sprintf(buf, "Speed: %3d%%    ", duty);
        LCD_PrintString(buf);
    }
}

void EXTI9_5_IRQHandler(void) {
    if (EXTI_REGISTERS->EXTI_PR & (1 << EMERGENCY_STOP_PIN)) {
        EXTI_ClearPending(EMERGENCY_STOP_PIN);
        delay_millis(DEBOUNCE_DELAY_MS);
        emergencyStop = 1;
        PWM_SetDutyCycle(0);
        LCD_PrintStatus();
    }

    if (EXTI_REGISTERS->EXTI_PR & (1 << RESET_BUTTON_PIN)) {
        EXTI_ClearPending(RESET_BUTTON_PIN);
        delay_millis(DEBOUNCE_DELAY_MS);
        if (emergencyStop) {
            emergencyStop = 0;
            prev_voltage = -999.0f;  // Force voltage update on next loop
            LCD_PrintStatus();
        }
    }
}

uint8 detect_falling_edge(uint8 button_port, uint8 button_pin) {
    static uint8_t previous_state = 0;
    static uint8_t button_pressed = 0;
    static uint32_t last_change_time = 0;

    uint8_t current_state = Gpio_ReadPin(button_port, button_pin);

    if (previous_state == 1 && current_state == 0) {
        if (system_ms - last_change_time > DEBOUNCE_DELAY_MS) {
            button_pressed = 1;
            last_change_time = system_ms;
        }
    }

    if (current_state == 1) {
        button_pressed = 0;
    }

    previous_state = current_state;

    if (button_pressed && current_state == 0) {
        button_pressed = 0;
        return 1;
    }

    return 0;
}

int main(void) {
    // Initialize clocks
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);
    Rcc_Enable(RCC_ADC1);

    // GPIO setup
    Gpio_Init(GPIO_C, 0, GPIO_ANALOG, GPIO_NO_PULL_DOWN); // ADC input PC0
    Gpio_Init(GPIO_B, 0, GPIO_OUTPUT, GPIO_PUSH_PULL);    // PWM output PB0
    Gpio_Init(GPIO_A, EMERGENCY_STOP_PIN, GPIO_INPUT, GPIO_PULL_UP);
    Gpio_Init(GPIO_A, RESET_BUTTON_PIN, GPIO_INPUT, GPIO_PULL_UP);

    // Initialize LCD, PWM, ADC
    LCD_Init();
    PWM_Init();
    ADC_Init();

    // Configure EXTI for buttons
    EXTI_Init(GPIO_A, EMERGENCY_STOP_PIN, FALLING_EDGE_TRIGGERED);
    EXTI_Init(GPIO_A, RESET_BUTTON_PIN, FALLING_EDGE_TRIGGERED);
    EXTI_Enable(EMERGENCY_STOP_PIN);
    EXTI_Enable(RESET_BUTTON_PIN);

    // Enable NVIC interrupt for EXTI lines 9 to 5
    NVIC->ISER[0] = (1 << EXTI9_5_IRQn);

    LCD_PrintStatus();

    uint8_t prev_duty = 0xFF;

    while (1) {

        if (!emergencyStop) {
            uint16_t raw_value = ADC_ReadBlocking(POTENTIOMETER_ADC_CHANNEL);
            if (raw_value > 4095) raw_value = 4095;

            float voltage = (raw_value * 3.3f) / 4095.0f;
            uint8_t new_duty = (uint8_t)((raw_value * 100.0f) / 4095.0f);

            if (new_duty != prev_duty) {
                duty = new_duty;
                PWM_SetDutyCycle(duty);
                char speed_str[16];
                sprintf(speed_str, "Speed: %3d%%", duty);
                LCD_SetCursor(1, 0);
                LCD_PrintString(speed_str);
                prev_duty = duty;
            }

            if (voltage < prev_voltage - 0.01f || voltage > prev_voltage + 0.01f || prev_voltage < 0) {
                char voltage_str[16];
                float_to_string(voltage, voltage_str, 2);
                LCD_SetCursor(0, 9);
                LCD_PrintString(voltage_str);
                prev_voltage = voltage;
            }

            if (detect_falling_edge(IR_BUTTON_PORT, IR_BUTTON_PIN)) {
                object_count++;

                LCD_Clear();  // Clear the screen
                LCD_SetCursor(0, 4);
                LCD_PrintString("NEW OBJECT");

                char count_str[16];
                sprintf(count_str, "Count: %3d", object_count);
                LCD_SetCursor(1, 4);
                LCD_PrintString(count_str);

                delay_millis(600);

                LCD_PrintStatus();
                prev_voltage = -999.0f;
                prev_duty = 0xFF;
            }

        }

    }

    return 0;
}