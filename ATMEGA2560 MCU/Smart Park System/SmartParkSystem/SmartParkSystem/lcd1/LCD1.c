/*
 * LCD11.c
 *
 * Created: 11/1/2023 9:04:18 AM
 *  Author: Emma
 */ 
#include <avr/io.h>
#include <util/delay.h>

#include "LCD1.h"

// Function to send a command to the LCD1
void lcd1_command(uint8_t cmd) {
	LCD1_RS_PORT &= ~(1 << LCD1_RS_PIN);
	LCD1_DATA_PORT = cmd;
	LCD1_EN_PORT |= (1 << LCD1_EN_PIN);
	_delay_ms(50); 
	LCD1_EN_PORT &= ~(1 << LCD1_EN_PIN); 
	_delay_ms(50);
}

// Function to send data to the LCD1
void lcd1_data(char data) {
	LCD1_RS_PORT |= (1 << LCD1_RS_PIN); 
	LCD1_DATA_PORT = data; 
	LCD1_EN_PORT |= (1 << LCD1_EN_PIN); 
	_delay_ms(10); 
	LCD1_EN_PORT &= ~(1 << LCD1_EN_PIN); 
	_delay_ms(10); 
}

// Function to initialize the LCD1
void lcd1_init(uint8_t dispAttr) {
	// Initialize your data direction, send initialization sequence, etc.
	LCD1_RS_PORT &= ~(1 << LCD1_RS_PIN);
	LCD1_DATA_PORT = dispAttr;
	LCD1_EN_PORT |= (1 << LCD1_EN_PIN); 
	_delay_ms(10); 
	LCD1_EN_PORT &= ~(1 << LCD1_EN_PIN);
	_delay_ms(10); 
}

// Function to set the cursor position
void lcd1_gotoxy(uint8_t x, uint8_t y) {
	// Calculate the address and send the command
	uint8_t address = (y == 0) ? 0x80 : 0xC0;
	address += x;
	lcd1_command(address);
}

// Function to display a string on the LCD1
void lcd1_print(const char *str) {
	// Iterate through the string and send each character to lcd1_data
	while (*str) {
		lcd1_data(*str);
		str++;
	}
}

void lcd1_printNumber(int number) {
	char buffer[16];  // Assuming a maximum of 16 characters for the number
	itoa(number, buffer, 10);  // Convert the integer to a string
	lcd1_print(buffer);  // Display the number as a string
}


// Function to clear the LCD1
void lcd1_clear() {
	lcd1_command(0x01); 
	_delay_ms(10); 
}

// Function to return the cursor to the home position
void lcd1_home() {
	lcd1_command(0x02); 
	_delay_ms(10); 
}
