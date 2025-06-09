#include <Std_Types.h>
#include "GPIO.h"
#include "GPIO_Private.h"

uint32 address_map[4] = {GPIOA_BASE_ADDR, GPIOB_BASE_ADDR, GPIOC_BASE_ADDR, GPIOD_BASE_ADDR};

void Gpio_Init(uint8 PortName, uint8 PinNumber, uint8 PinMode, uint8 DefaultState)
{
    uint8 port_address_index = PortName - GPIO_A;

    GPIO_Device* Device = (GPIO_Device*) address_map[port_address_index];

    Device -> GPIO_MODER &= ~(0x03 << (PinNumber * 2));
    Device -> GPIO_MODER |= (PinMode << (PinNumber * 2));

    if (PinMode == GPIO_INPUT)
    {
        Device -> GPIO_PUPDR &= ~(0x03 << (PinNumber * 2));
        Device -> GPIO_PUPDR |= DefaultState << (PinNumber * 2);
    } else
    {
        Device -> GPIO_OTYPER &= ~(0x1 << PinNumber);
        Device -> GPIO_OTYPER |= DefaultState << PinNumber;
    }

}

uint8 Gpio_WritePin(uint8 PortName, uint8 PinNumber, uint8 Data) {
    uint8 port_address_index = PortName - GPIO_A;
    GPIO_Device* Device = (GPIO_Device*) address_map[port_address_index];

    uint8 pinMode = (Device -> GPIO_MODER & (0x03 << (PinNumber * 2))) >> (PinNumber * 2);
    if (pinMode == GPIO_INPUT) return NOK;

    Device -> GPIO_ODR &= ~(0x01 << PinNumber);
    Device -> GPIO_ODR |= Data << PinNumber;
    return OK;
}

uint8 Gpio_ReadPin(uint8 PortName, uint8 PinNumber) {
    uint8 port_address_index = PortName - GPIO_A;
    GPIO_Device* Device = (GPIO_Device*) address_map[port_address_index];

    return (Device -> GPIO_IDR & (0x01 << PinNumber)) >> PinNumber;
}