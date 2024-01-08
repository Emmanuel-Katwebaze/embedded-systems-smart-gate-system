/*
 * UART.c
 *
 * Created: 11/1/2023 11:53:23 AM
 *  Author: Emma
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "UART.h"

void UART_init(){
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
	UBRR0 = 0x67;
}

unsigned char UART_RxChar(){
	while ((UCSR0A & (1 << RXC0)) == 0); // wait till data is received
	return(UDR0);
}

void UART_TxChar(char ch){
	while (! (UCSR0A & (1 << UDRE0))); // wait for empty transmit buffer
	UDR0 = ch;
}

void UART_SendString(char *str){
	unsigned char j=0;
	while (str[j] != '\0') // send string till null
	{
		UART_TxChar(str[j]);
		j++;
	}
}

// Function to read a string from the serial terminal and return it as a string
char* readStringFromInput(int maxLength) {
	char* str = (char*)malloc(maxLength);
	if (str == NULL) {
		// Memory allocation failed, handle the error
		return NULL;
	}

	int index = 0;
	while (index < maxLength - 1) {
		char c = UART_RxChar();
		UART_TxChar(c);  // Echo back the character
		if (c == '\n' || c == '\r') {
			break;
		}
		str[index] = c;
		index++;
	}
	str[index] = '\0';
	return str;
}

// Function to read an integer from the serial terminal
int readIntFromInput() {
	char buffer[10];
	int index = 0;
	while (1) {
		char c = UART_RxChar();
		UART_TxChar(c);  // Echo back the character
		if (c == '\n' || c == '\r') {
			break;
		}
		buffer[index] = c;
		index++;
	}
	buffer[index] = '\0';
	return atoi(buffer);  // Convert the string to an integer
}


void uart_send_string(uint8_t *c){
	uint16_t i = 0;
	do{
		UART_TxChar(c[i]);
		i++;
		
	}while(c[i] != '\0');
	UART_TxChar(c[i]);
}

void uart_send_int(int num) {
	int max_num_digits = 20; // Maximum digits for a 32-bit integer with sign
	char* str_buffer = (char*)malloc(max_num_digits + 1); // +1 for the null-terminator

	if (str_buffer) {
		sprintf(str_buffer, "%d", num);
		uart_send_string(str_buffer);
		free(str_buffer); // Don't forget to free the allocated memory
		} else {
		// Handle memory allocation failure
		uart_send_string("Memory allocation failed");
	}
}

