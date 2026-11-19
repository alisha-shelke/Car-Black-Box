/* 
 * File:   external_eeprom.h
 * Author: alish
 *
 * Created on 17 November, 2026, 5:30 PM
 */

#ifndef EXTERNAL_EEPROM_H
#define EXTERNAL_EEPROM_H

#define EEPROM_SLAVE_WRITE      0xA0
#define EEPROM_SLAVE_READ       0xA1

void write_external_eeprom(unsigned char address, unsigned char data);
unsigned char read_external_eeprom(unsigned char address);

#endif

