/*
 * File:   dkp.c
 * Author: Alisha Shelke 
 *
 * Created on 17 November, 2026, 5:40 PM
 */
#include <xc.h>
#include "dkp.h"

#define _XTAL_FREQ  20000000
#define DEBOUNCE_MS 50

void init_digital_keypad(void)
{
    KEYPAD_PORT_DDR = KEYPAD_PORT_DDR | INPUT_LINES;
}

unsigned char read_digital_keypad(unsigned char mode)
{
    static unsigned char once = 1;
    unsigned char key = KEYPAD_PORT & INPUT_LINES;

    if (mode == LEVEL_DETECTION)
    {
        return key;
    }

    if (key != ALL_RELEASED && once)
    {
        __delay_ms(DEBOUNCE_MS);                       /* wait for bounce to stop */
        if ((KEYPAD_PORT & INPUT_LINES) == key)        /* still pressed = real press */
        {
            once = 0;
            return key;
        }
    }
    else if (key == ALL_RELEASED && !once)
    {
        __delay_ms(DEBOUNCE_MS);                       /* wait for release bounce */
        if ((KEYPAD_PORT & INPUT_LINES) == ALL_RELEASED)
        {
            once = 1;                                  /* ready for next press */
        }
    }

    return ALL_RELEASED;
}