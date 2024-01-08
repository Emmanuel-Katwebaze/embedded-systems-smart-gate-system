/*
 * LCD2.c
 *
 * Created: 11/1/2023 10:27:38 AM
 *  Author: Emma
 */ 
#include <avr/io.h>
#include <util/delay.h>

#include "LCD2.h"

// Function to send a command to the LCD2
void lcd2_command(uint8_t cmd) {
	LCD2_RS_PORT &= ~(1 << LCD2_RS_PIN);
	LCD2_DATA_PORT = cmd;
	LCD2_EN_PORT |= (1 << LCD2_EN_PIN);
	_delay_ms(50);
	LCD2_EN_PORT &= ~(1 << LCD2_EN_PIN);
	_delay_ms(50);
}

// Function to send data to the LCD2
void lcd2_data(char data) {
	LCD2_RS_PORT |= (1 << LCD2_RS_PIN);
	LCD2_DATA_PORT = data;
	LCD2_EN_PORT |= (1 << LCD2_EN_PIN);
	_delay_ms(50);
	LCD2_EN_PORT &= ~(1 << LCD2_EN_PIN);
	_delay_ms(50);
}

// Function to initialize the LCD2
void lcd2_init(uint8_t dispAttr) {
	// Initialize your data direction, send initialization sequence, etc.
	LCD2_RS_PORT &= ~(1 << LCD2_RS_PIN);
	LCD2_DATA_PORT = dispAttr;
	LCD2_EN_PORT |= (1 << LCD2_EN_PIN);
	_delay_ms(50);
	LCD2_EN_PORT &= ~(1 << LCD2_EN_PIN);
	_delay_ms(50);
}

// Function to set the cursor position
void lcd2_gotoxy(uint8_t x, uint8_t y) {
	// Calculate the address and send the command
	uint8_t address = (y == 0) ? 0x80 : (y == 1) ? 0xC0 : (y == 2) ? 0x94 : 0xD4;
	address += x;
	lcd2_command(address);
}

// Function to display a string on the LCD2
void lcd2_print(const char *str) {
	// Iterate through the string and send each character to lcd2_data
	while (*str) {
		lcd2_data(*str);
		str++;
	}
}

void lcd2_printNumber(int number) {
	char buffer[16];  // Assuming a maximum of 16 characters for the number
	itoa(number, buffer, 10);  // Convert the integer to a string
	lcd2_print(buffer);  // Display the number as a string
}


// Function to clear the LCD2
void lcd2_clear() {
	lcd2_command(0x01);
	_delay_ms(50);
}

// Function to return the cursor to the home position
void lcd2_home() {
	lcd2_command(0x02);
	_delay_ms(10);
}
