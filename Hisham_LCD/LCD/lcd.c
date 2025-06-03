#include "lcd.h"
#include "Gpio.h"

// Define pins and ports
#define LCD_PORT GPIO_A
#define RS_PIN 1
#define RW_PIN 2
#define E_PIN  3
#define D4_PIN 4
#define D5_PIN 5
#define D6_PIN 6
#define D7_PIN 7

#define HIGH 1
#define LOW  0

// Adjust this value for your clock speed
#define NUMBER_OF_CYCLES 1000000

static void LCD_EnablePulse(void);
static void LCD_SendNibble(uint8_t nibble);

void delay_ms(uint32_t delay) {
    for (volatile uint32_t i = 0; i < (NUMBER_OF_CYCLES / 1000) * delay; i++) {
        __asm__("nop");
    }
}

void LCD_Init(void) {
    // Configure GPIO pins as output
    Gpio_Init(LCD_PORT, RS_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_PORT, RW_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_PORT, E_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_PORT, D4_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_PORT, D5_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_PORT, D6_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_Init(LCD_PORT, D7_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);

    Gpio_WritePin(LCD_PORT, RS_PIN, LOW);
    Gpio_WritePin(LCD_PORT, RW_PIN, LOW);
    Gpio_WritePin(LCD_PORT, E_PIN, LOW);

    delay_ms(20); // Wait for power stabilization

    // Initialize LCD in 4-bit mode
    LCD_SendNibble(0x03);
    delay_ms(5);
    LCD_SendNibble(0x03);
    delay_ms(1);
    LCD_SendNibble(0x03);
    LCD_SendNibble(0x02);

    // Send configuration commands
    LCD_SendCommand(LCD_CMD_FUNCTION_SET);
    LCD_SendCommand(LCD_CMD_DISPLAY_ON);
    LCD_SendCommand(LCD_CMD_ENTRY_MODE);
    LCD_Clear();
}

void LCD_SendCommand(LCD_Command cmd) {
    Gpio_WritePin(LCD_PORT, RS_PIN, LOW);
    Gpio_WritePin(LCD_PORT, RW_PIN, LOW);

    LCD_SendNibble(cmd >> 4);
    LCD_SendNibble(cmd);
}

void LCD_PrintChar(char data) {
    Gpio_WritePin(LCD_PORT, RS_PIN, HIGH);
    Gpio_WritePin(LCD_PORT, RW_PIN, LOW);

    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data);
}

void LCD_PrintString(const char *str) {
    while (*str) {
        LCD_PrintChar(*str++);
    }
}

void LCD_SetCursor(LCD_Row row, uint8_t col) {
    uint8_t address = (row == LCD_ROW_0) ? col : (0x40 + col);
    LCD_SendCommand(0x80 | address);
}

void LCD_Clear(void) {
    LCD_SendCommand(LCD_CMD_CLEAR);
    delay_ms(2);
}

static void LCD_EnablePulse(void) {
    Gpio_WritePin(LCD_PORT, E_PIN, HIGH);
    delay_ms(1);
    Gpio_WritePin(LCD_PORT, E_PIN, LOW);
    delay_ms(1);
}

static void LCD_SendNibble(uint8_t nibble) {
    Gpio_WritePin(LCD_PORT, D4_PIN, (nibble >> 0) & 0x01);
    Gpio_WritePin(LCD_PORT, D5_PIN, (nibble >> 1) & 0x01);
    Gpio_WritePin(LCD_PORT, D6_PIN, (nibble >> 2) & 0x01);
    Gpio_WritePin(LCD_PORT, D7_PIN, (nibble >> 3) & 0x01);
    LCD_EnablePulse();
}
