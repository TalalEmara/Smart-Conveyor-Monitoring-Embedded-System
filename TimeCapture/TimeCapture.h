#ifndef TIMECAPTURE_H
#define TIMECAPTURE_H

#include "stm32f4xx.h"

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