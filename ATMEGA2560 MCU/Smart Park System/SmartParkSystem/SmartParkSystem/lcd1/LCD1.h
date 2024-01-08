/*
 * LCD1.h
 *
 * Created: 11/1/2023 9:04:37 AM
 *  Author: Emma
 */ 
#ifndef LCD1_H
#define LCD1_H

// Define your ports and pins for connecting the LCD to your microcontroller
#define LCD1_RS_PORT   PORTH
#define LCD1_RS_PIN    4  // Define the appropriate pin number
#define LCD1_EN_PORT   PORTH
#define LCD1_EN_PIN    6  // Define the appropriate pin number
#define LCD1_DATA_PORT PORTJ // Example: 4-bit data interface, so only 4 pins are used
#define LCD1_DATA_PIN  5


// LCD commands
#define LCD1_DISPLAY_ON_CURSOR 0x0E // Display on and cursor on
#define LCD1_CLEAR_DISPLAY  0x01
#define LCD1_RETURN_HOME    0x02
#define LCD1_ENTRY_MODE     0x06
#define LCD1_DISPLAY_ON     0x0C
#define LCD1_DISPLAY_OFF    0x08
#define LCD1_FUNCTION_SET   0x3F
#define LCD1_SET_CGRAM_ADDR 0x40

void lcd1_command(uint8_t cmd);
void lcd1_data(char data);
void lcd1_init(uint8_t dispAttr);
void lcd1_gotoxy(uint8_t x, uint8_t y);
void lcd1_print(const char *string);
void lcd1_printNumber(int number);
void lcd1_clear();
void lcd1_home();

#endif // LCD_H
