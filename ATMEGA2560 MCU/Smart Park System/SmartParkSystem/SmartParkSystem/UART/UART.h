/*
 * UART.h
 *
 * Created: 11/1/2023 11:53:36 AM
 *  Author: Emma
 */ 
#ifndef UART_H
#define UART_H

void UART_init();
unsigned char UART_RxChar();
void UART_TxChar(char ch);
void UART_SendString(char *str);
char* readStringFromInput(int maxLength);
int readIntFromInput();
void uart_send_string(uint8_t *c);
void uart_send_int(int num);

#endif 