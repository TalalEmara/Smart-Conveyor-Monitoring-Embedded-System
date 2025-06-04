#include "RCC.h"
#include "Gpio.h"
#include "lcd.h"
#include "Std_Types.h"
#include "pwm.h"
#include "EXTI.h"
#include <stdio.h>

#define DELAY_LOOP 50000
#define DEBOUNCE_DELAY_MS   50

#define IR_BUTTON_PORT GPIO_A
#define IR_BUTTON_PIN  15

volatile uint8_t emergencyStop = 0;
uint8_t duty = 0; // DO NOT FORGET : when change with potentiometer during stop , should be changedd
volatile uint16 object_count = 0;
volatile uint32_t system_ms = 0;


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

void delay(volatile uint32_t count) {
    for(volatile uint32_t i = 0; i < count; i++) {
        __asm__("nop");
    }
    system_ms += (count/1000);  // Track time
}

void LCD_PrintStatus(void) {
    if (emergencyStop) {
        LCD_SetCursor(0, 0);
        LCD_PrintString("!!! EMERGENCY !!!");
        LCD_SetCursor(1, 0);
        LCD_PrintString("SYSTEM STOPPED  ");
    } else {
        char buf[16];
        int_to_string(duty, buf);
        LCD_SetCursor(0, 0);
        LCD_PrintString("System Running  ");
        LCD_SetCursor(1, 0);
        LCD_PrintString("                ");
        LCD_SetCursor(1, 0);
        LCD_PrintString(buf);
    }
}

void EXTI9_5_IRQHandler(void) {
    // Check if interrupt is from line 8 (PA8) - Emergency Stop
    if ((EXTI_REGISTERS->EXTI_PR & (1 << 8)) != 0) {
        EXTI_ClearPending(8);
        delay(50000); // debounce delay
        emergencyStop = 1;
        PWM_SetDutyCycle(0); // stop PWM immediately
        LCD_PrintStatus();
    }

    // Check if interrupt is from line 9 (PA9) - Reset after emergency
    if ((EXTI_REGISTERS->EXTI_PR & (1 << 9)) != 0) {
        EXTI_ClearPending(9);
        delay(50000); // debounce delay
        if (emergencyStop) {
            emergencyStop = 0;
            // DO NOT FORGET : when change with potentiometer during stop , should be changedd
            // LCD_PrintStatus();
        }
    }
}

// Falling edge detection with debouncing
uint8 detect_falling_edge(uint8 button_port, uint8 button_pin) {
    static uint8_t previous_state = 0;
    static uint8_t button_pressed = 0;
    static uint32_t last_change_time = 0;

    uint8_t current_state = Gpio_ReadPin(button_port, button_pin);

    // Detect transition from released (1) to pressed (0) = falling edge
    if (previous_state == 1 && current_state == 0) {
        // Check debounce time
        if (system_ms - last_change_time > DEBOUNCE_DELAY_MS) {
            button_pressed = 1;
            last_change_time = system_ms;
        }
    }

    // Reset when button is released
    if (current_state == 1) {
        button_pressed = 0;
    }

    previous_state = current_state;

    // Return 1 only once per button press
    if (button_pressed && current_state == 0) {
        button_pressed = 0;  // Reset flag
        return 1;
    }

    return 0;
}

void update_display(void) {
    char count_str[12];

    // Clear and update first line
    LCD_SetCursor(LCD_ROW_0, 0);
    LCD_PrintString("Objects Counted:");

    // Update count on second line
    LCD_SetCursor(LCD_ROW_1, 0);
    int_to_string(object_count, count_str);
    LCD_PrintString(count_str);
    LCD_PrintString("               ");  // Clear any remaining characters
}

int main(void) {
    Rcc_Init();

    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);
    Rcc_Enable(RCC_TIM3);

    LCD_Init();
    PWM_Init();

    // Configure PA8 and PA9 as input pull-up
    Gpio_Init(GPIO_A, 8, GPIO_INPUT, GPIO_PULL_UP);
    Gpio_Init(GPIO_A, 9, GPIO_INPUT, GPIO_PULL_UP);

    Gpio_Init(IR_BUTTON_PORT, IR_BUTTON_PIN, GPIO_INPUT, GPIO_PULL_UP);

    delay(10000);

    // Configure EXTI lines for PA8 and PA9 falling edge trigger
    EXTI_Init(GPIO_A, 8, FALLING_EDGE_TRIGGERED);
    EXTI_Init(GPIO_A, 9, FALLING_EDGE_TRIGGERED);
    EXTI_Enable(8);
    EXTI_Enable(9);

    LCD_SetCursor(0, 0);
    LCD_PrintString("Motor speed");

    delay(20000);

    emergencyStop = 0;
    duty = 0;
    LCD_PrintStatus();

    while(1) {
        if (!emergencyStop) {
            PWM_SetDutyCycle(duty);
            duty += 10;
            if (duty > 100) duty = 0;
            if (detect_falling_edge(IR_BUTTON_PORT, IR_BUTTON_PIN)) {
                object_count++;
                // update_display();

                // Optional: Show brief "DETECTED!" message
                LCD_SetCursor(LCD_ROW_1, 10);
                LCD_PrintString("NEW!");
            }
            LCD_PrintStatus();
            delay(DELAY_LOOP);
        } else {
            delay(DELAY_LOOP);
        }
    }

    return 0;
}