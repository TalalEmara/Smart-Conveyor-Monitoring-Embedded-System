#ifndef TIMECAPTURE_H
#define TIMECAPTURE_H

#include "stm32f4xx.h"


typedef struct
{
   uint32_t CR1;         /*!< TIM control register 1,              Address offset: 0x00 */
   uint32_t CR2;         /*!< TIM control register 2,              Address offset: 0x04 */
   uint32_t SMCR;        /*!< TIM slave mode control register,     Address offset: 0x08 */
   uint32_t DIER;        /*!< TIM DMA/interrupt enable register,   Address offset: 0x0C */
   uint32_t SR;          /*!< TIM status register,                 Address offset: 0x10 */
   uint32_t EGR;         /*!< TIM event generation register,       Address offset: 0x14 */
   uint32_t CCMR1;       /*!< TIM capture/compare mode register 1, Address offset: 0x18 */
   uint32_t CCMR2;       /*!< TIM capture/compare mode register 2, Address offset: 0x1C */
   uint32_t CCER;        /*!< TIM capture/compare enable register, Address offset: 0x20 */
   uint32_t CNT;         /*!< TIM counter register,                Address offset: 0x24 */
   uint32_t PSC;         /*!< TIM prescaler,                       Address offset: 0x28 */
   uint32_t ARR;         /*!< TIM auto-reload register,            Address offset: 0x2C */
   uint32_t RCR;         /*!< TIM repetition counter register,     Address offset: 0x30 */
   uint32_t CCR1;        /*!< TIM capture/compare register 1,      Address offset: 0x34 */
   uint32_t CCR2;        /*!< TIM capture/compare register 2,      Address offset: 0x38 */
   uint32_t CCR3;        /*!< TIM capture/compare register 3,      Address offset: 0x3C */
   uint32_t CCR4;        /*!< TIM capture/compare register 4,      Address offset: 0x40 */
   uint32_t BDTR;        /*!< TIM break and dead-time register,    Address offset: 0x44 */
   uint32_t DCR;         /*!< TIM DMA control register,            Address offset: 0x48 */
   uint32_t DMAR;        /*!< TIM DMA address for full transfer,   Address offset: 0x4C */
   uint32_t OR;          /*!< TIM option register,                 Address offset: 0x50 */
} TIMER_TypeDef;
#define TIMER2_BASE             ( 0x40000000UL + 0x0000UL)

#define TIMER2              ((TIMER_TypeDef *) TIMER2_BASE)

//Masks
#define UIF                   (0x1UL << (0U))
#define UPDATE_GENERATION_MSK (0x1UL << (0U))
#define COUNTER_ENABLE_MSK    (0x1UL << (0U))
#define CC1S_MSK                  (0x3UL << (0U))
#define IC1F_MSK                     (0xFUL << (4U))
#define CC1P_Msk              (0x1UL << (1U))
#define CC1NP_MSK        (0x1UL << (3U))
#define CAPTURE_ENABLE_MSK    (0x1UL << (0U))

#define CC1_IF (0x1UL << (1U))
// Time Capture Functions
void TimeCapture_Init(void);
uint32_t TimeCapture_GetPeriod(void);
void TimeCapture_Start(void);
void TimeCapture_Stop(void);
void ProcessInputCapture(void);

// Shared variables
extern volatile uint8_t captureFlag;
extern volatile uint32_t period;

#endif