#ifndef PWM_H
#define PWM_H

#include "stm32f4xx.h"

void PWM_Init(void);
void PWM_SetDutyCycle(uint8_t duty);

#endif
