/* 
 * File:   uart.h
 * Author: Alisha Shelke 
 *
 * Created on 17 November, 2026, 5:40 PM
 */

#ifndef UART_H
#define	UART_H

#define FOSC                20000000

void init_uart(unsigned long baud);
unsigned char getchar(void);
void putchar(unsigned char data);
void puts(const char *s);

#endif

