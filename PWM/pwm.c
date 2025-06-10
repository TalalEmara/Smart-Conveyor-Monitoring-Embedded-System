// pwm.c
#include "pwm.h"
#include "Gpio.h"
#include <Rcc.h>
#include "GPIO_Private.h"

void PWM_Init(void) {
    // Enable clock for TIM3
    Rcc_Enable(RCC_TIM3);
    Rcc_Enable(RCC_GPIOB);

    // Initialize PB0 as alternate function
    Gpio_Init(GPIO_B, 0, GPIO_AF, GPIO_PUSH_PULL);

    GPIO_Device* gpioB = (GPIO_Device*)GPIOB_BASE_ADDR;
    gpioB->GPIO_AFRL &= ~(0xF << (0 * 4));  // Clear bits for PB0
    gpioB->GPIO_AFRL |=  (0x2 << (0 * 4));  // Set AF2 for TIM3_CH3

    TIMER3->PSC = 16 - 1;   // prescaler
    TIMER3->ARR = 1000 - 1; // auto-reload

    TIMER3->CCR3 = 0;         // Initial duty cycle 0% for CH3
    TIMER3->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1 for CH3
    TIMER3->CCER |= TIM_CCER_CC3E;  // Enable channel 3 output
    TIMER3->CR1 |= TIM_CR1_CEN;     // Enable timer
}

void PWM_SetDutyCycle(uint8 duty) {
    if (duty > 100) duty = 100;
    TIMER3->CCR3 = (TIMER3->ARR + 1) * duty / 100; // Use CCR3 for channel 3
}