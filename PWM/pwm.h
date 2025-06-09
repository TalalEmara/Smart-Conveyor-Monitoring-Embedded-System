#ifndef PWM_H
#define PWM_H

#include "Std_Types.h"
#define TIMER3_BASE  0x40000400U

typedef struct {
    volatile uint32 CR1;
    volatile uint32 CR2;
    volatile uint32 SMCR;
    volatile uint32 DIER;
    volatile uint32 SR;
    volatile uint32 EGR;
    volatile uint32 CCMR1;
    volatile uint32 CCMR2;
    volatile uint32 CCER;
    volatile uint32 CNT;
    volatile uint32 PSC;
    volatile uint32 ARR;
    volatile uint32 RCR;
    volatile uint32 CCR1;
    volatile uint32 CCR2;
    volatile uint32 CCR3;
    volatile uint32 CCR4;
    volatile uint32 BDTR;
    volatile uint32 DCR;
    volatile uint32 DMAR;
    volatile uint32 OR;
} PWM_TypeDef;

#define TIMER3 ((PWM_TypeDef *)TIMER3_BASE)


#define TIM_CCMR2_OC3M_Pos 4
#define TIM_CCMR2_OC3M_1 (1 << (TIM_CCMR2_OC3M_Pos + 1))
#define TIM_CCMR2_OC3M_2 (1 << (TIM_CCMR2_OC3M_Pos + 2))

#define TIM_CCER_CC3E (1 << 8)

#define TIM_CR1_CEN (1 << 0)

void PWM_Init(void);
void PWM_SetDutyCycle(uint8 duty);

#endif
