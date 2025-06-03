// main.c
#include "stm32f4xx.h"
#include "TimeCapture.h"

void UART2_Init(void);
void UART2_SendString(char *str);
void Delay_ms(uint32_t ms);
uint8_t CheckSignalPresent(void);

int main(void) {
    // Initialize UART2 for debugging
    UART2_Init();
    UART2_SendString("System Initialized\r\n");

    // Initialize Time Capture on PA5
    TimeCapture_Init();
    UART2_SendString("Time Capture Ready on PA5\r\n");

    while(1) {
        // 1. Check PA5 as input
        // GPIOA->MODER &= ~GPIO_MODER_MODER5; // Input mode
        // if(GPIOA->IDR & GPIO_IDR_ID5) {
        //     UART2_SendString("PA5 HIGH\r\n");
        // } else {
        //     UART2_SendString("PA5 LOW\r\n");
        // }
        //
        // // 2. Switch back to AF mode for capture
        // GPIOA->MODER |= GPIO_MODER_MODER5_1; // AF mode
        // GPIOA->AFR[0] |= (1 << (5*4)); // AF1 (TIM2_CH1)

        // 3. Attempt capture
        TimeCapture_Start();
        uint32_t timeout = 0;

        while(!captureFlag && timeout++ < 1000000) {
            ProcessInputCapture();
        }

        if(captureFlag) {
            timeout = 0;
            while(captureFlag && timeout++ < 2000000) {
                ProcessInputCapture();
            }

            if(period != 0) {
                UART2_SendString("P:");
                char buf[10];
                int i = 0;
                uint32_t num = period;
                do { buf[i++] = (num % 10) + '0'; num /= 10; } while(num > 0);
                while(i > 0) {
                    while(!(USART2->SR & USART_SR_TXE));
                    USART2->DR = buf[--i];
                }
                UART2_SendString("us\r\n");
            }
        }

        Delay_ms(1000);
    }
}

// Signal presence check function
uint8_t CheckSignalPresent(void) {
    // Temporarily configure PA5 as input
    GPIOA->MODER &= ~GPIO_MODER_MODER5;

    uint8_t samples = 0;
    uint8_t high_count = 0;

    // Sample the pin 100 times
    for(samples = 0; samples < 100; samples++) {
        if(GPIOA->IDR & GPIO_IDR_ID5) {
            high_count++;
        }
        Delay_ms(1);
    }

    // Restore alternate function mode
    GPIOA->MODER |= GPIO_MODER_MODER5_1;

    // Consider signal present if we detected any highs
    return (high_count > 10);  // At least 10% high samples
}

// UART2 Implementation (PA2 as RX, PA3 as TX)
void UART2_Init(void) {
    // 1. Enable GPIO and USART clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // 2. Configure PA2 (RX) and PA3 (TX)
    GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
    GPIOA->AFR[0] |= (7 << (2 * 4)) | (7 << (3 * 4));

    // 3. Configure USART2
    USART2->BRR = (16000000 + 9600/2) / 9600;
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void UART2_SendString(char *str) {
    while(*str) {
        while(!(USART2->SR & USART_SR_TXE));
        USART2->DR = (*str & 0xFF);
        str++;
    }
}

void Delay_ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms * 1000; i++) {
        __NOP();
    }
}