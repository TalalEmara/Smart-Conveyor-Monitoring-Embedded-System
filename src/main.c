#include <TimeCapture.h>

#include "RCC.h"
#include "Gpio.h"
#include "Adc.h"
#include "lcd.h"
#include "pwm.h"
#include "EXTI.h"

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
volatile uint8_t object_detected = 0;  // Flag for interrupt-based detection

uint8_t duty = 0;
float prev_voltage = -1.0f;
uint8_t prev_duty = 0xFF;
int prev_conv_speed = -1;
uint8_t prev_object_count = 0xFF;

// State machine for TimeCapture
typedef enum {
    CAPTURE_IDLE,
    CAPTURE_WAITING_START,
    CAPTURE_WAITING_END
} capture_state_t;

capture_state_t capture_state = CAPTURE_IDLE;
uint32_t capture_timeout = 0;
uint32_t last_speed_update = 0;

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


void int_to_string_padded(int value, char* buffer, uint8_t width) {
    if (width == 2) {
        buffer[0] = '0' + (value / 10);
        buffer[1] = '0' + (value % 10);
        buffer[2] = '\0';
    } else if (width == 3) {
        buffer[0] = '0' + (value / 100);
        buffer[1] = '0' + ((value / 10) % 10);
        buffer[2] = '0' + (value % 10);
        buffer[3] = '\0';
    } else {
        int_to_string(value, buffer);
    }
}

void LCD_PrintStatus(void) {
    LCD_SetCursor(LCD_ROW_0, 0);
    if (emergencyStop) {
        LCD_PrintString("!!! EMERGENCY !!!");
        LCD_SetCursor(LCD_ROW_1, 0);
        LCD_PrintString("SYSTEM STOPPED  ");
    } else {
        LCD_PrintString("Object Count:   ");
        LCD_SetCursor(LCD_ROW_1, 0);
        LCD_PrintString("Conv:    Mot:  %"); // Shorter labels, more space

        prev_conv_speed = -1;
        prev_duty = 0xFF;
        prev_object_count = 0xFF;
    }
}

void LCD_UpdateObjectCount(void) {
    if (object_count != prev_object_count) {
        char count_str[4];
        int_to_string_padded(object_count > 99 ? 99 : object_count, count_str, 2);
        LCD_SetCursor(LCD_ROW_0, 13);
        LCD_PrintString(count_str);
        prev_object_count = object_count;
    }
}

void LCD_UpdateConvSpeed(int speed) {
    if (speed != prev_conv_speed) {
        char conv_speed_buf[6];
        int_to_string(speed, conv_speed_buf);
        LCD_SetCursor(LCD_ROW_1, 5); // Position after "C:"
        LCD_PrintString("    "); // Clear remaining space up to "M:"
        LCD_SetCursor(LCD_ROW_1, 5); // Position after "C:"
        LCD_PrintString(conv_speed_buf);
        prev_conv_speed = speed;
    }
}

void LCD_UpdateMotorDuty(void) {
    if (duty != prev_duty) {
        char duty_str[4];
        int_to_string_padded(duty > 99 ? 99 : duty, duty_str, 2);
        LCD_SetCursor(LCD_ROW_1, 13);  // Position after "M:"
        LCD_PrintString(duty_str);
        prev_duty = duty;
    }
}

// OPTION 1: Interrupt-based IR sensor detection
void EXTI15_10_IRQHandler(void) {
    if (EXTI_REGISTERS->EXTI_PR & (1 << IR_BUTTON_PIN)) {
        EXTI_ClearPending(IR_BUTTON_PIN);
        if (!emergencyStop) {
            object_detected = 1;  // Set flag instead of incrementing directly
        }
    }
}

void EXTI9_5_IRQHandler(void) {
    if (EXTI_REGISTERS->EXTI_PR & (1 << EMERGENCY_STOP_PIN)) {
        EXTI_ClearPending(EMERGENCY_STOP_PIN);
        emergencyStop = 1;
        PWM_SetDutyCycle(0);
        LCD_PrintStatus();
    }

    if (EXTI_REGISTERS->EXTI_PR & (1 << RESET_BUTTON_PIN)) {
        EXTI_ClearPending(RESET_BUTTON_PIN);
        if (emergencyStop) {
            emergencyStop = 0;
            prev_voltage = -999.0f;
            prev_duty = 0xFF;
            prev_conv_speed = -1;
            prev_object_count = 0xFF;
            LCD_PrintStatus();
        }
    }
}

// OPTION 2: Non-blocking polling function
uint8 detect_falling_edge_nonblocking(uint8 button_port, uint8 button_pin) {
    static uint8_t previous_state = 1;  // Assume pulled up initially
    static uint32_t last_change_time = 0;
    static uint8_t edge_detected = 0;

    uint8_t current_state = Gpio_ReadPin(button_port, button_pin);

    // Detect falling edge with debouncing
    if (system_ms - last_change_time > DEBOUNCE_DELAY_MS) {
        if (previous_state == 1 && current_state == 0) {
            edge_detected = 1;
            last_change_time = system_ms;
        }
    }

    previous_state = current_state;

    if (edge_detected) {
        edge_detected = 0;
        return 1;
    }

    return 0;
}

// Non-blocking TimeCapture processing
void ProcessTimeCaptureNonBlocking(void) {
    switch (capture_state) {
        case CAPTURE_IDLE:
            TimeCapture_Start();
            capture_state = CAPTURE_WAITING_START;
            capture_timeout = 0;
            break;

        case CAPTURE_WAITING_START:
            ProcessInputCapture();
            capture_timeout++;

            if (captureFlag) {
                capture_state = CAPTURE_WAITING_END;
                capture_timeout = 0;
            } else if (capture_timeout > 10000) {  // Timeout after many iterations
                capture_state = CAPTURE_IDLE;
            }
            break;

        case CAPTURE_WAITING_END:
            ProcessInputCapture();
            capture_timeout++;

            if (!captureFlag && period != 0) {
                // Successfully captured period
                int conv_speed = (int)(1000000.0/period);
                LCD_UpdateConvSpeed(conv_speed);
                last_speed_update = system_ms;
                capture_state = CAPTURE_IDLE;
            } else if (capture_timeout > 10000) {  // Timeout
                capture_state = CAPTURE_IDLE;
            }
            break;
    }
}

int main(void) {
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);
    Rcc_Enable(RCC_ADC1);

    Gpio_Init(GPIO_C, 0, GPIO_ANALOG, GPIO_NO_PULL_DOWN); // PC0 - ADC
    Gpio_Init(GPIO_B, 0, GPIO_OUTPUT, GPIO_PUSH_PULL);    // PB0 - PWM output
    Gpio_Init(GPIO_A, EMERGENCY_STOP_PIN, GPIO_INPUT, GPIO_PULL_UP);
    Gpio_Init(GPIO_A, RESET_BUTTON_PIN, GPIO_INPUT, GPIO_PULL_UP);
    Gpio_Init(GPIO_A, IR_BUTTON_PIN, GPIO_INPUT, GPIO_PULL_UP);  // IR sensor

    LCD_Init();
    PWM_Init();
    ADC_Init();
    TimeCapture_Init();

    // Setup interrupts
    EXTI_Init(GPIO_A, EMERGENCY_STOP_PIN, FALLING_EDGE_TRIGGERED);
    EXTI_Init(GPIO_A, RESET_BUTTON_PIN, FALLING_EDGE_TRIGGERED);

    // OPTION 1: Enable interrupt for IR sensor (recommended)
    EXTI_Init(GPIO_A, IR_BUTTON_PIN, FALLING_EDGE_TRIGGERED);
    EXTI_Enable(IR_BUTTON_PIN);
    NVIC->ISER[1] = (1 << (EXTI15_10_IRQn - 32));  // Enable EXTI15_10 interrupt

    EXTI_Enable(EMERGENCY_STOP_PIN);
    EXTI_Enable(RESET_BUTTON_PIN);
    NVIC->ISER[0] = (1 << EXTI9_5_IRQn);

    LCD_PrintStatus();

    while (1) {
        if (!emergencyStop) {
            // OPTION 1: Handle interrupt-based object detection
            if (object_detected) {
                object_detected = 0;  // Clear flag
                object_count++;
                LCD_UpdateObjectCount();
            }

            // OPTION 2: Alternative - Non-blocking polling (comment out if using Option 1)
            /*
            if (detect_falling_edge_nonblocking(IR_BUTTON_PORT, IR_BUTTON_PIN)) {
                object_count++;
                LCD_UpdateObjectCount();
            }
            */

            // Non-blocking conveyor speed measurement
            ProcessTimeCaptureNonBlocking();

            // ADC and PWM processing (fast, non-blocking)
            uint16_t raw_value = ADC_ReadBlocking(POTENTIOMETER_ADC_CHANNEL);
            if (raw_value > 4095) raw_value = 4095;

            duty = (uint8_t)((raw_value * 100.0f) / 4095.0f);
            PWM_SetDutyCycle(duty);
            LCD_UpdateMotorDuty();

            // Update object count display regularly
            LCD_UpdateObjectCount();
        }

        // Very small delay to prevent CPU hogging
        delay_millis(1);
    }

    return 0;
}