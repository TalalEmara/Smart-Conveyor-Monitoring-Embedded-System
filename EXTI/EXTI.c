#include "EXTI.h"
#include "GPIO.h"
#include "EXTI_Private.h"

SYSCFG_EXTILineConfig* EXTI_SYSCFG = (SYSCFG_EXTILineConfig*) SYSCFG_EXTI_BaseAddr;
uint8 CFG_Options[] = {CFG_PA, CFG_PB, CFG_PC, CFG_PD, CFG_PE, CFG_PH};
NVIC_Type* NVIC = (NVIC_Type*) NVIC_ISER_BASE_Addr;

void EXTI_Init(uint8 PortName, uint8 LineNumber, uint8 TriggerType)
{
    // set corresponding SYSCONFIG register to correct port
    // set correct rising or falling trigger registers
    uint8 register_index = LineNumber / 4;
    uint8 register_offset = LineNumber % 4;
    uint8 CFG_option = CFG_Options[PortName - GPIO_A];

    EXTI_SYSCFG->EXTI_CR[register_index] &= ~(0xF << (register_offset * 4));
    EXTI_SYSCFG->EXTI_CR[register_index] |= (CFG_option << (register_offset * 4));

    switch (TriggerType)
    {
        case FALLING_EDGE_TRIGGERED:
            EXTI_REGISTERS->EXTI_FTSR |= (1 << LineNumber);
            EXTI_REGISTERS->EXTI_RTSR &= ~(1 << LineNumber);
            break;

        case RISING_EDGE_TRIGGERED:
            EXTI_REGISTERS->EXTI_FTSR &= ~(1 << LineNumber);
            EXTI_REGISTERS->EXTI_RTSR |= (1 << LineNumber);
            break;

        case EDGE_TRIGGERED:
            EXTI_REGISTERS->EXTI_FTSR |= (1 << LineNumber);
            EXTI_REGISTERS->EXTI_RTSR |= (1 << LineNumber);
            break;

        default:
            break;
    }
}

void EXTI_Enable(uint8 LineNumber)
{
    // enable mask at both EXTI and NVIC
    EXTI_REGISTERS->EXTI_IMR |= (1 << LineNumber);

    uint8 NVIC_line;
    if (LineNumber < 5) NVIC_line = LineNumber + 6;
    else if (LineNumber < 10) NVIC_line = 23;
    else NVIC_line = 40;

    if (NVIC_line == 40) NVIC->NVIC_ISER[1] |= (1 << (NVIC_line - 32));
    else NVIC->NVIC_ISER[0] |= (1 << NVIC_line);
}

void EXTI_Disable(uint8 LineNumber)
{
    // disable mask at both EXTI and NVIC
    EXTI_REGISTERS->EXTI_IMR &= ~(1 << LineNumber);

    uint8 NVIC_line;
    if (LineNumber < 5) NVIC_line = LineNumber + 6;
    else if (LineNumber < 10) NVIC_line = 23;
    else NVIC_line = 40;

    if (NVIC_line == 40) NVIC->NVIC_ICER[1] |= (1 << (NVIC_line - 32));
    else NVIC->NVIC_ICER[0] |= (1 << NVIC_line);
}

// In EXTI.c
void EXTI_ClearPending(uint8 LineNumber) {
    EXTI_REGISTERS->EXTI_PR |= (1 << LineNumber);
}