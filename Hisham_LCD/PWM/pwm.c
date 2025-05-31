#include "Gpio.h"
#include "Gpio.c"
void PWM_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_TIM1EN;

    // Set PA8 to AF Push-Pull
    GPIOA->CRH &= ~(0xF << 0);
    GPIOA->CRH |= (0xB << 0);

    TIM1->PSC = 72 - 1;
    TIM1->ARR = 1000;
    TIM1->CCR1 = 0;

    TIM1->CCMR1 |= 0x60;
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->CR1 |= TIM_CR1_CEN;
}

void PWM_SetDuty(uint16_t duty) {
    if (duty > 1000) duty = 1000;
    TIM1->CCR1 = duty;
}
