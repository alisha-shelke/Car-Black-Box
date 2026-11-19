/*
 * File:   external_eeprom.c
 * Author: Alisha Shelke 
 *
 * Created on 17 November, 2026, 5:32 PM
 */

#include <xc.h>
#include "i2c.h"
#include "external_eeprom.h"

#ifndef _XTAL_FREQ
#define _XTAL_FREQ  20000000
#endif

void write_external_eeprom(unsigned char address, unsigned char data)
{
    i2c_start();
    i2c_write(EEPROM_SLAVE_WRITE);
    i2c_write(address);
    i2c_write(data);
    i2c_stop();

    /* EEPROM internal write cycle time */
    __delay_ms(5);
}

unsigned char read_external_eeprom(unsigned char address)
{
    unsigned char data;

    i2c_start();
    i2c_write(EEPROM_SLAVE_WRITE);
    i2c_write(address);
    i2c_rep_start();
    i2c_write(EEPROM_SLAVE_READ);
    data = i2c_read(1);         /* 1 = NACK, last byte */
    i2c_stop();

    return data;
}