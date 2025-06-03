#include "RCC.h"
#include "Gpio.h"
#include "ADC.h"
#include "lcd.h"
#include "Std_Types.h"
#include "pwm.h"
#include <stdio.h>

volatile uint8 counter = 0x00;
volatile uint8 button = 0;
volatile uint8 toggle = 1;

#define NUMBER_OF_CYCLES    1000000

// void delay_ms(uint32 delay) {
//     uint32 i;
//     /* number of cycles / 1000 = 1ms */
//     for (i = 0; i < (NUMBER_OF_CYCLES/1000) * delay; i++) {
//         // Do nothing, just wait
//         __asm__("nop");
//     }
// }

/* //test pwm
 void LCD_PrintDutyCycle(uint8_t duty) {
    char buf[16];
    sprintf(buf, "Duty: %3d%%", duty);
    LCD_SetCursor(0, 0);
    LCD_PrintString("            ");  // Clear line by printing spaces
    LCD_SetCursor(0, 0);
    LCD_PrintString(buf);
} */

int main(void) {
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);

    // HAL_Init();
    // SystemClock_Config();

    // Initialize the LCD
    LCD_Init();
    ADC_Init();
    PWM_Init();

    // Display static text
    LCD_PrintString("Conveyor Speed:");
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("Motor Speed:");

    uint16_t adc_value;
    uint8_t duty;
    char buf[16];

    while (1) {
        adc_value = ADC_Read();              // Read potentiometer ADC value (0-4095)
        duty = (adc_value * 100) / 4095;    // Convert to 0-100% duty cycle

        PWM_SetDutyCycle(duty);              // Set PWM output
        LCD_SetCursor(1, 0);
        sprintf(buf, "Speed: %3d%%", duty);
        LCD_PrintString(buf);
    }
/* //testing pwm
    uint8_t duty = 0;

    while(1) {
        PWM_SetDutyCycle(duty);
        LCD_PrintDutyCycle(duty);

        duty += 5;
        if (duty > 100) duty = 0;

        for (volatile int i = 0; i < 500000; i++);  // Delay, adjust for speed
    } */

    return 0;
}