#ifndef EXTI_H
#define EXTI_H

#include "Std_Types.h"

#define EXTI_BaseAddr 0x40013C00

#define FALLING_EDGE_TRIGGERED 0
#define RISING_EDGE_TRIGGERED 1
#define EDGE_TRIGGERED 2

typedef struct
{
    volatile uint32 EXTI_IMR;
    volatile uint32 EXTI_EMR;
    volatile uint32 EXTI_RTSR;
    volatile uint32 EXTI_FTSR;
    volatile uint32 EXTI_SWIER;
    volatile uint32 EXTI_PR;
} EXTI_Type;

#define EXTI_REGISTERS ((volatile EXTI_Type*) EXTI_BaseAddr)

void EXTI_Init(uint8 PortName, uint8 LineNumber, uint8 TriggerType);

void EXTI_Enable(uint8 LineNumber);

void EXTI_Disable(uint8 LineNumber);

void EXTI_ClearPending(uint8 LineNumber);

#endif //EXTI_H
