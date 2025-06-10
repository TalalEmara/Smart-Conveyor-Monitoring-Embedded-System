#ifndef LCD_H
#define LCD_H

#include "stm32f4xx.h"

// Enum for LCD rows
typedef enum {
    LCD_ROW_0 = 0,
    LCD_ROW_1
} LCD_Row;

// Enum for configuration options
typedef enum {
    LCD_CMD_CLEAR = 0x01,       // Clear display
    LCD_CMD_RETURN_HOME = 0x02, // Return cursor to home position
    LCD_CMD_ENTRY_MODE = 0x06,  // Increment cursor, no shift
    LCD_CMD_DISPLAY_ON = 0x0C,  // Display ON, Cursor OFF
    LCD_CMD_FUNCTION_SET = 0x28 // 4-bit mode, 2 lines, 5x8 dots
} LCD_Command;

// Function prototypes
void LCD_Init(void);
void LCD_SendCommand(LCD_Command cmd);
void LCD_PrintChar(char data);
void LCD_PrintString(const char *str);
void LCD_SetCursor(LCD_Row row, uint8_t col);
void LCD_Clear(void);

#endif // LCD_H
