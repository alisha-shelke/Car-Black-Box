/*
 * File:   main.c
 * Author: Alisha Shelke 
 *
 * Created on 17 November, 2026, 5:17 PM
 */

#include <xc.h>
#include "clcd.h"
#include "dkp.h"
#include "adc.h"
#include "i2c.h"
#include "ds1307.h"
#include "uart.h"
#include "external_eeprom.h"
#include "car_black_box.h"

#pragma config WDTE = OFF        // Watchdog Timer disabled
#pragma config LVP = OFF         // RB3 used as normal I/O

static void init_config(void)
{
    init_clcd();
    init_digital_keypad();
    init_adc();
    init_i2c(100000);           /* 100 kHz */
    init_ds1307();
    init_uart(9600);
    init_black_box();
}

void main(void)
{
    unsigned char key;
    unsigned char screen = DASHBOARD_SCREEN;
    unsigned char new_screen = DASHBOARD_SCREEN;
    unsigned char first = 1;

    init_config();

    while (1)
    {
        /* one key press = one key value */
        key = read_digital_keypad(STATE);

        switch (screen)
        {
            case DASHBOARD_SCREEN:
                new_screen = dashboard(key, first);
                break;
            case PASSWORD_SCREEN:
                new_screen = password_screen(key, first);
                break;
            case MENU_SCREEN:
                new_screen = menu_screen(key, first);
                break;
            case VIEW_LOG_SCREEN:
                new_screen = view_log_screen(key, first);
                break;
            case CLEAR_LOG_SCREEN:
                new_screen = clear_log_screen(key, first);
                break;
            case DOWNLOAD_LOG_SCREEN:
                new_screen = download_log_screen(key, first);
                break;
            case SET_TIME_SCREEN:
                new_screen = set_time_screen(key, first);
                break;
            case CHANGE_PASSWORD_SCREEN:
                new_screen = change_password_screen(key, first);
                break;
            default:
                new_screen = DASHBOARD_SCREEN;
                break;
        }

        /* screen changed -> next loop it is a fresh screen */
        if (new_screen != screen)
        {
            screen = new_screen;
            first = 1;
        }
        else
        {
            first = 0;
        }
    }
}