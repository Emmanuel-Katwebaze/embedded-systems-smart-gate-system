/*
 * LCD2.h
 *
 * Created: 11/1/2023 10:27:56 AM
 *  Author: Emma
 */ 
#ifndef LCD2_H
#define LCD2_H

// Define your ports and pins for connecting the LCD to your microcontroller
#define LCD2_RS_PORT   PORTA
#define LCD2_RS_PIN    4  // Define the appropriate pin number
#define LCD2_EN_PORT   PORTA
#define LCD2_EN_PIN    6  // Define the appropriate pin number
#define LCD2_DATA_PORT PORTB // Example: 4-bit data interface, so only 4 pins are used
#define LCD2_DATA_PIN  5


// LCD commands
#define LCD2_DISPLAY_ON_CURSOR 0x0E // Display on and cursor on
#define LCD2_CLEAR_DISPLAY  0x01
#define LCD2_RETURN_HOME    0x02
#define LCD2_ENTRY_MODE     0x06
#define LCD2_DISPLAY_ON     0x0C
#define LCD2_DISPLAY_OFF    0x08
#define LCD2_FUNCTION_SET   0x3F
#define LCD2_SET_CGRAM_ADDR 0x40

void lcd2_command(uint8_t cmd);
void lcd2_data(char data);
void lcd2_init(uint8_t dispAttr);
void lcd2_gotoxy(uint8_t x, uint8_t y);
void lcd2_print(const char *string);
void lcd2_clear();
void lcd2_home();

#endif // 