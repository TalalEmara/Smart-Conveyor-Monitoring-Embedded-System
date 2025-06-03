// pwm.c
#include "pwm.h"
// Updated PWM_Init() - use PB0 instead of PA7
void PWM_Init(void) {
    // Enable Timer clock TIM3
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Configure PB0 as alternate function for PWM output (TIM3_CH3)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER &= ~(0x3 << (0*2)); // Clear PB0
    GPIOB->MODER |= (0x2 << (0*2));  // Set PB0 to Alternate Function
    GPIOB->AFR[0] &= ~(0xF << (0*4)); // Clear AF bits for PB0
    GPIOB->AFR[0] |= (0x2 << (0*4));  // AF2 = TIM3_CH3

    // Timer configuration for PWM Mode 1
    TIM3->PSC = 16 - 1;   // prescaler
    TIM3->ARR = 1000 - 1; // auto-reload

    TIM3->CCR3 = 0;         // Initial duty cycle 0% for CH3
    TIM3->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1 for CH3
    TIM3->CCER |= TIM_CCER_CC3E;  // Enable channel 3 output
    TIM3->CR1 |= TIM_CR1_CEN;     // Enable timer
}

void PWM_SetDutyCycle(uint8_t duty) {
    if (duty > 100) duty = 100;
    TIM3->CCR3 = (TIM3->ARR + 1) * duty / 100; // Use CCR3 for channel 3
}