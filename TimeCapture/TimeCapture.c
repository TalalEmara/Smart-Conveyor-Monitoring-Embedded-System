#include "TimeCapture.h"

// Time Capture Implementation
volatile uint32_t capture1 = 0;
volatile uint32_t capture2 = 0;
volatile uint32_t period = 0;
volatile uint8_t captureFlag = 0;
volatile uint32_t overflow_count = 0;

void TimeCapture_Init(void) {
    // Enable GPIOA and TIM2 clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Configure PA5 as AF1 (TIM2_CH1) with proper alternate function
    GPIOA->MODER &= ~GPIO_MODER_MODER5;      // Clear mode bits
    GPIOA->MODER |= GPIO_MODER_MODER5_1;     // Set to AF mode (10)
    GPIOA->AFR[0] &= ~(0xF << (5*4));        // Clear AF bits for PA5
    GPIOA->AFR[0] |= (0x1 << (5*4));         // Set AF1 for TIM2_CH1

    // Configure TIM2 for input capture
    TIMER2->PSC = 16000-1;                          // Did not work 84MHz/(83+1) = 1MHz (1µs resolution)
    TIMER2->EGR |= UPDATE_GENERATION_MSK;                 // Force update event to load new PSC
    TIMER2->SR &= ~UIF;
    TIMER2->ARR = 0xFFFFFFFF  ;                 //Hisham dont change this
    TIMER2->CR1 &= ~COUNTER_ENABLE_MSK;               // Ensure timer is stopped during config

    // Configure Channel 1 for input capture
    TIMER2->CCMR1 &= ~(CC1S_MSK | IC1F_MSK);  // Clear capture selection and filter bits
    TIMER2->CCMR1 |= TIM_CCMR1_CC1S_0;        // CC1S = 01: TI1 mapped to CC1

    // TIM2->CCMR1 |= (0x3 << 4);              // IC1F = 0011: fSAMPLING=fCK_INT, N=8

    // Configure capture on rising edge
    TIMER2->CCER &= ~(CC1P_Msk | CC1NP_MSK); // Clear polarity bits (rising edge)
    TIMER2->CCER |= CAPTURE_ENABLE_MSK;             // Enable capture

    // Enable update interrupt for overflow handling (even though you can't use interrupts)
    // We'll poll for these flags instead
    TIMER2->DIER = 0;                          // Clear all interrupt enables first
    TIMER2->SR = 0;                            // Clear all flags

    // Enable the timer
    TIMER2->CR1 |= COUNTER_ENABLE_MSK;
}

uint32_t TimeCapture_GetPeriod(void) {
    return period;
}
//
// void TimeCapture_Start(void) {
//     TIM2->CNT = 0;
//     captureFlag = 0;
//     capture1 = 0;
//     capture2 = 0;
//     period = 0;
//     overflow_count = 0;
//     TIM2->SR = 0;  // Clear all flags
// }

void TimeCapture_Stop(void) {
    TIMER2->CR1 &= ~COUNTER_ENABLE_MSK;
}

// Call this function regularly in your main loop to process captures
// Fixed calculation in ProcessInputCapture
void ProcessInputCapture(void) {
    uint32_t sr = TIMER2->SR;

    // Handle timer overflow
    if(sr & UIF) {
        overflow_count++;
        TIMER2->SR &= ~UIF;  // Clear overflow flag
    }

    // Handle input capture
    if(sr & CC1_IF) {
        uint32_t current_capture = TIMER2->CCR1;

        if(!captureFlag) {
            // First capture - store reference point
            capture1 = current_capture;
            captureFlag = 1;
            overflow_count = 0;  // Reset overflow counter for this measurement
        } else {
            // Second capture - calculate period
            capture2 = current_capture;

            // Fixed calculation for period
            if(overflow_count == 0) {
                // Simple case - no overflow between captures
                if(capture2 >= capture1) {
                    period = capture2 - capture1;
                } else {
                    // This shouldn't happen if no overflow
                    period = 0;  // Error case
                }
            } else {
                // Handle overflow case - use 64-bit arithmetic
                uint64_t temp_period;
                temp_period = ((uint64_t)overflow_count << 32) + capture2 - capture1;

                // Check if result fits in 32-bit
                if(temp_period > 0xFFFFFFFF) {
                    period = 0xFFFFFFFF;  // Saturate to maximum
                } else {
                    period = (uint32_t)temp_period;
                }
            }

            captureFlag = 0;
            overflow_count = 0;
        }

        TIM2->SR &= ~TIM_SR_CC1IF;  // Clear capture flag
    }
}

// Also fix the start function to ensure clean start
void TimeCapture_Start(void) {
    TIMER2->CR1 &= ~COUNTER_ENABLE_MSK;       // Stop timer first
    TIMER2->CNT = 0;                   // Reset counter to 0
    captureFlag = 0;
    capture1 = 0;
    capture2 = 0;
    period = 0;
    overflow_count = 0;
    TIMER2->SR = 0;                    // Clear all flags
    TIMER2->CR1 |= COUNTER_ENABLE_MSK;        // Restart timer
}
// Helper functions (optional - you can remove if not needed)
// uint8_t TimeCapture_HasNewPeriod(void) {
//     static uint32_t lastPeriod = 0;
//
//     if(period != lastPeriod && period != 0) {
//         lastPeriod = period;
//         return 1;
//     }
//     return 0;
// }

// uint32_t TimeCapture_GetFrequencyHz(void) {
//     if(period == 0) return 0;
//     return (1000000 + period/2) / period;  // 1MHz / period with rounding
// }