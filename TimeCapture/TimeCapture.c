#include "TimeCapture.h"

// Time Capture Implementation
volatile uint32_t capture1 = 0;
volatile uint32_t capture2 = 0;
volatile uint32_t period = 0;
volatile uint8_t captureFlag = 0;
void TimeCapture_Init(void) {
    // Enable GPIOA and TIM2 clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Configure PA5 as AF1 (TIM2_CH1)
    GPIOA->MODER &= ~GPIO_MODER_MODER5;
    GPIOA->MODER |= GPIO_MODER_MODER5_1;
    GPIOA->AFR[0] |= (1 << (5*4));

    // Configure TIM2
    TIM2->PSC = 83;  // 84MHz/84 = 1MHz (1us resolution)
    TIM2->ARR = 0xFFFFFFFF;
    TIM2->CCMR1 = TIM_CCMR1_CC1S_0;  // TI1->CC1
    TIM2->CCER = TIM_CCER_CC1E;      // Capture on rising edge
    TIM2->DIER = 0;                  // No interrupts

    // Enable the timer
    TIM2->CR1 |= TIM_CR1_CEN;
}

uint32_t TimeCapture_GetPeriod(void) {
    return period;
}

void TimeCapture_Start(void) {
    TIM2->CNT = 0;
    captureFlag = 0;
    capture1 = 0;
    capture2 = 0;
    period = 0;
}

void TimeCapture_Stop(void) {
    TIM2->CR1 &= ~TIM_CR1_CEN;
}
//
// void ProcessInputCapture(void) {
//     if(TIM2->SR & TIM_SR_CC1IF) {
//         if(!captureFlag) {
//             capture1 = TIM2->CCR1;
//             captureFlag = 1;
//         } else {
//             capture2 = TIM2->CCR1;
//             if (capture2 > capture1) {
//                 period = capture2 - capture1;
//             } else {
//                 period = (0xFFFFFFFF - capture1) + capture2;
//             }
//             captureFlag = 0;
//         }
//         TIM2->SR &= ~TIM_SR_CC1IF;
//     }
// }
// Add these global variables
volatile uint32_t last_capture = 0;
volatile uint32_t overflow_count = 0;

void ProcessInputCapture(void) {
    // Check for both capture and overflow events
    uint32_t sr = TIM2->SR;

    if(sr & TIM_SR_UIF) {
        overflow_count++;
        TIM2->SR &= ~TIM_SR_UIF;
    }

    if(sr & TIM_SR_CC1IF) {
        uint32_t current_capture = TIM2->CCR1;

        if(!captureFlag) {
            last_capture = current_capture;
            captureFlag = 1;
            // Switch to opposite edge detection
            TIM2->CCER ^= TIM_CCER_CC1P;
        } else {
            // Calculate period considering overflows
            period = (overflow_count * 0xFFFFFFFF) + current_capture - last_capture;
            captureFlag = 0;
            overflow_count = 0;
            // Switch back to initial edge detection
            TIM2->CCER ^= TIM_CCER_CC1P;
        }
        TIM2->SR &= ~TIM_SR_CC1IF;
    }
}