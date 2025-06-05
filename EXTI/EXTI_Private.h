#ifndef EXTI_PRIVATE_H
#define EXTI_PRIVATE_H

#define SYSCFG_EXTI_BaseAddr 0x40013808

#define NVIC_ISER_BASE_Addr 0xE000E100
#define NVIC_ICER_BASE 0xE000E180

typedef struct
{
    volatile uint32 EXTI_CR[4];
} SYSCFG_EXTILineConfig;


typedef struct
{
    volatile uint32 NVIC_ISER[8];
    volatile uint32 NVIC_ICER[8];
} NVIC_Type;


#define CFG_PA 0x0
#define CFG_PB 0x1
#define CFG_PC 0x2
#define CFG_PD 0x3
#define CFG_PE 0x4
#define CFG_PH 0x7



#endif //EXTI_PRIVATE_H
