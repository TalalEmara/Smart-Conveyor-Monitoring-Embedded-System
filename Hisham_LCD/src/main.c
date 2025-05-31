// main.c

#include "RCC.h"
#include "Gpio.h"
#include "lcd.h"
#include "adc.h"
#include "pwm.h"
#include "Std_Types.h"
#include <stdint.h>
volatile uint8 counter = 0x00;
volatile uint8 button = 0;
volatile uint8 toggle = 1;
uint16_t adc_value;
uint8_t pwm_percent;
char buffer[16];
extern void ADC_Init(void);
extern uint16_t ADC_Read(void);
extern void PWM_Init(void);
extern void PWM_SetDuty(uint16_t duty);



#define NUMBER_OF_CYCLES    1000000

// void delay_ms(uint32 delay) {
//     uint32 i;
//     /* number of cycles / 1000 = 1ms */
//     for (i = 0; i < (NUMBER_OF_CYCLES/1000) * delay; i++) {
//         // Do nothing, just wait
//         __asm__("nop");
//     }
// }

int main(void) {
    Rcc_Init();
    Rcc_Enable(RCC_GPIOA);
    Rcc_Enable(RCC_GPIOB);
    Rcc_Enable(RCC_GPIOC);
    Rcc_Enable(RCC_SYSCFG);

    Rcc_Enable(RCC_TIM2);
    Rcc_Enable(RCC_ADC1);



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

    while (1) {
        adc_value = ADC_Read();        // 0–4095
        uint16_t duty = adc_value / 4; // 0–1023 scaled to 0–1000
        PWM_SetDuty(duty);

        pwm_percent = duty / 10; // 0–100%

        // Display on LCD
        LCD_SetCursor(LCD_ROW_1, 0);
        printf(buffer, "PWM: %3d%%     ", pwm_percent);
        LCD_PrintString(buffer);

    }

    return 0;
}
