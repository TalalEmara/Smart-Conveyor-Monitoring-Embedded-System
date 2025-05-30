// main.c

#include "RCC.h"
#include "Gpio.h"
#include "Keypad.h"
#include "lcd.h"
#include "Std_Types.h"


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

    // Display static text
    LCD_PrintString("Conveyor Speed:");
    LCD_SetCursor(LCD_ROW_1, 0);
    LCD_PrintString("Motor Speed:");

    while (1) {
        // Update LCD with dynamic data (e.g., speed, object count)
    }

    return 0;
}
