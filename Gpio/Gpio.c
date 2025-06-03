/**
 * Gpio.c
 *
 *  Created on: 4/15/2025
 *  Author    : AbdallahDarwish
 */


#include <Std_Types.h>
#include "Gpio.h"
#include "Utils.h"
#include "Bit_Operations.h"
#include "Gpio_Private.h"

void Gpio_Init(uint8 PortName, uint8 PinNumber, uint8 PinMode, uint8 DefaultState) {
    switch (PortName) {
        case GPIO_A:
            GPIOA_MODER &= ~(0x03 << (PinNumber * 2));
            GPIOA_MODER |= (PinMode << (PinNumber * 2));

            if (PinMode == GPIO_INPUT) {
                GPIOA_PUPDR &= ~(0x03 << (PinNumber * 2));
                GPIOA_PUPDR |= (DefaultState << (PinNumber * 2));
            } else {
                GPIOA_OTYPER  &=~(0x1 << PinNumber);
                GPIOA_OTYPER  |= (DefaultState << (PinNumber));
            }
            break;
        case GPIO_B:
            GPIOB_MODER &= ~(0x03 << (PinNumber * 2));
            GPIOB_MODER |= (PinMode << (PinNumber * 2));

            if (PinMode == GPIO_INPUT) {
                GPIOB_PUPDR &= ~(0x03 << (PinNumber * 2));
                GPIOB_PUPDR |= (DefaultState << (PinNumber * 2));
            } else {
                GPIOB_OTYPER  &=~(0x1 << PinNumber);
                GPIOB_OTYPER  |= (DefaultState << (PinNumber));
                // if (DefaultState == GPIO_PUSH_PULL) {
                //     CLEAR_BIT(GPIOB_OTYPER, PinNumber);
                // }else if (DefaultState == GPIO_OPEN_DRAIN) {
                //     SET_BIT(GPIOB_OTYPER, PinNumber);
                // }
            }
            break;
        case GPIO_C:
            GPIOC_MODER &= ~(0x03 << (PinNumber * 2));
            GPIOC_MODER |= (PinMode << (PinNumber * 2));

            if (PinMode == GPIO_INPUT) {
                GPIOC_PUPDR &= ~(0x03 << (PinNumber * 2));
                GPIOC_PUPDR |= (DefaultState << (PinNumber * 2));
            } else {
                GPIOC_OTYPER  &=~(0x1 << PinNumber);
                GPIOC_OTYPER  |= (DefaultState << (PinNumber));
            }
            break;
        case GPIO_D:
            GPIOD_MODER &= ~(0x03 << (PinNumber * 2));
            GPIOD_MODER |= (PinMode << (PinNumber * 2));

            if (PinMode == GPIO_INPUT) {
                GPIOD_PUPDR &= ~(0x03 << (PinNumber * 2));
                GPIOD_PUPDR |= (DefaultState << (PinNumber * 2));
            } else {
                GPIOD_OTYPER  &=~(0x1 << PinNumber);
                GPIOD_OTYPER  |= (DefaultState << (PinNumber));
            }
            break;
        default:
            break;
    }

}

uint8 Gpio_WritePin(uint8 PortName, uint8 PinNumber, uint8 Data) {
    uint32 mode = 0;
    
    // Check if pin is configured as output
    switch (PortName) {
        case GPIO_A:
            mode = (GPIOA_MODER >> (PinNumber * 2)) & 0x3;
            if (mode == 0) { // Assuming 0 is INPUT mode
                return NOK;
            }
            GPIOA_ODR &= ~(0x1 << PinNumber);
            GPIOA_ODR |= ((Data & 0x1) << PinNumber);
            break;
        case GPIO_B:
            mode = (GPIOB_MODER >> (PinNumber * 2)) & 0x3;
            if (mode == 0) {
                return NOK;
            }
            GPIOB_ODR &= ~(0x1 << PinNumber);
            GPIOB_ODR |= ((Data & 0x1) << PinNumber);
            break;
        case GPIO_C:
            mode = (GPIOC_MODER >> (PinNumber * 2)) & 0x3;
            if (mode == 0) {
                return NOK;
            }
            GPIOC_ODR &= ~(0x1 << PinNumber);
            GPIOC_ODR |= ((Data & 0x1) << PinNumber);
            break;
        case GPIO_D:
            mode = (GPIOD_MODER >> (PinNumber * 2)) & 0x3;
            if (mode == 0) {
                return NOK;
            }
            GPIOD_ODR &= ~(0x1 << PinNumber);
            GPIOD_ODR |= ((Data & 0x1) << PinNumber);
            break;
        default:
            return NOK;
    }
    
    return OK;
}

uint8 Gpio_ReadPin(uint8 PortName, uint8 PinNumber) {
    uint8 pinValue = 0;
    
    switch (PortName) {
        case GPIO_A:
            pinValue = (GPIOA_IDR >> PinNumber) & 0x1;
            break;
        case GPIO_B:
            pinValue = (GPIOB_IDR >> PinNumber) & 0x1;
            break;
        case GPIO_C:
            pinValue = (GPIOC_IDR >> PinNumber) & 0x1;
            break;
        case GPIO_D:
            pinValue = (GPIOD_IDR >> PinNumber) & 0x1;
            break;
        default:
            return NOK;
    }
    
    return pinValue;
}