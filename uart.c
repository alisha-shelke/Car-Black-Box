/*
 * File:   uart.c
 * Author: Alisha Shelke 
 *
 * Created on 17 November, 2026, 5:41 PM
 */
#include <xc.h>
#include "uart.h"

void init_uart(unsigned long baud)
{
    /* Baud Rate Setting Register : 20 MHz, 9600 -> 129 */
    SPBRG = (FOSC / (16 * baud)) - 1;

    /* High speed baud rate */
    BRGH = 1;

    /* Asynchronous mode */
    SYNC = 0;

    /* Setting RC6 and RC7 to work as Serial Port */
    SPEN = 1;

    /* Transmit Enable Bit */
    TXEN = 1;

    /* Continuous Reception Enable Bit */
    CREN = 1;
}

unsigned char getchar(void)
{
    /* Wait for the byte to be received */
    while (RCIF != 1)
    {
        continue;
    }

    /* Return the data to the caller */
    return RCREG;
}

void putchar(unsigned char data)
{
    /* Transmit the data to the Serial Port */
    TXREG = data;

    /* Wait till the transmission is complete */
    while (TXIF != 1)
    {
        continue;
    }
}

void puts(const char *s)
{
    /* Send the string character by character */
    while (*s)
    {
        putchar(*s++);
    }
}