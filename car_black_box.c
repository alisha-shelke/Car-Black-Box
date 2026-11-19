/*
 * File:   car_black_box.c
 * Author: Alisha Shelke 
 *
 * Created on 17 November, 2026, 5:23 PM
 */

#include <xc.h>
#include "car_black_box.h"
#include "clcd.h"
#include "dkp.h"
#include "adc.h"
#include "ds1307.h"
#include "uart.h"
#include "external_eeprom.h"

#define PASSWORD_LEN    4
#define MAX_LOGS        10
#define LOG_SIZE        5           /* HH, MM, SS, event, speed */
#define LOCK_SECONDS    60          /* blocked time after 3 wrong passwords */

/* addresses inside the external EEPROM */
#define ADDR_LOG_POS    0x60
#define ADDR_LOG_COUNT  0x61
#define ADDR_PWD_FLAG   0x70
#define ADDR_PWD        0x71        /* 0x71 to 0x74 */

static const char ev_name[10][3] = {"ON", "GN", "G1", "G2", "G3", "G4", "GR", "C ", "CL", "CP"};

static const char menu_item[5][16] = {
    "View log       ",
    "Clear log      ",
    "Download log   ",
    "Set time       ",
    "Change password"
};

static unsigned char gear = 0;              /* 0 = GN, 1 to 4 = G1 to G4, 5 = GR */
static unsigned char event = EV_ON;
static unsigned char log_pos = 0;           /* where the next log will be stored */
static unsigned char log_count = 0;         /* how many logs are stored (max 10) */
static char password[PASSWORD_LEN];

/* ---------------------------------------------------------------- */
/* small helper functions                                           */
/* ---------------------------------------------------------------- */

static void wait_ms(unsigned int ms)
{
    while (ms--)
    {
        __delay_ms(1);
    }
}

static unsigned char bcd_to_dec(unsigned char bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static unsigned char dec_to_bcd(unsigned char dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

/* t[0] = HH, t[1] = MM, t[2] = SS  (all in BCD) */
static void get_time(unsigned char *t)
{
    t[0] = read_ds1307(HOUR_ADDR) & 0x3F;
    t[1] = read_ds1307(MIN_ADDR) & 0x7F;
    t[2] = read_ds1307(SEC_ADDR) & 0x7F;
}

/* BCD time -> "HH:MM:SS" */
static void time_to_text(const unsigned char *t, char *s)
{
    s[0] = (t[0] >> 4) + '0';
    s[1] = (t[0] & 0x0F) + '0';
    s[2] = ':';
    s[3] = (t[1] >> 4) + '0';
    s[4] = (t[1] & 0x0F) + '0';
    s[5] = ':';
    s[6] = (t[2] >> 4) + '0';
    s[7] = (t[2] & 0x0F) + '0';
    s[8] = '\0';
}

/* speed 0 to 99 from the potentiometer (ADC 0 to 1023) */
static unsigned char get_speed(void)
{
    unsigned short speed = read_adc(0) / 10;

    if (speed > 99)
    {
        speed = 99;
    }

    return (unsigned char) speed;
}

/* ---------------------------------------------------------------- */
/* event log (stored in external EEPROM)                            */
/* ---------------------------------------------------------------- */

static void save_log_pointers(void)
{
    write_external_eeprom(ADDR_LOG_POS, log_pos);
    write_external_eeprom(ADDR_LOG_COUNT, log_count);
}

static void log_event(unsigned char ev)
{
    unsigned char t[3];
    unsigned char addr;

    get_time(t);
    addr = log_pos * LOG_SIZE;

    write_external_eeprom(addr, t[0]);
    write_external_eeprom(addr + 1, t[1]);
    write_external_eeprom(addr + 2, t[2]);
    write_external_eeprom(addr + 3, ev);
    write_external_eeprom(addr + 4, get_speed());

    log_pos++;
    if (log_pos == MAX_LOGS)
    {
        log_pos = 0;                        /* oldest log is overwritten */
    }

    if (log_count < MAX_LOGS)
    {
        log_count++;
    }

    save_log_pointers();
}

/* index 0 = latest log, 1 = one before ... : "0 HH:MM:SS EV SP" */
static void make_log_text(unsigned char index, char *s)
{
    unsigned char rec[LOG_SIZE];
    unsigned char slot, addr, i, ev, sp;

    slot = (log_pos + MAX_LOGS - 1 - index) % MAX_LOGS;
    addr = slot * LOG_SIZE;

    for (i = 0; i < LOG_SIZE; i++)
    {
        rec[i] = read_external_eeprom(addr + i);
    }

    ev = rec[3];
    if (ev > 9)
    {
        ev = 0;
    }

    sp = rec[4];
    if (sp > 99)
    {
        sp = 99;
    }

    s[0] = index + '0';
    s[1] = ' ';
    time_to_text(rec, s + 2);               /* fills s[2] to s[9] */
    s[10] = ' ';
    s[11] = ev_name[ev][0];
    s[12] = ev_name[ev][1];
    s[13] = ' ';
    s[14] = (sp / 10) + '0';
    s[15] = (sp % 10) + '0';
    s[16] = '\0';
}

void init_black_box(void)
{
    unsigned char i;

    /* first time (blank EEPROM) -> default password 1010 */
    if (read_external_eeprom(ADDR_PWD_FLAG) != 0x55)
    {
        write_external_eeprom(ADDR_PWD + 0, '1');
        write_external_eeprom(ADDR_PWD + 1, '0');
        write_external_eeprom(ADDR_PWD + 2, '1');
        write_external_eeprom(ADDR_PWD + 3, '0');
        write_external_eeprom(ADDR_PWD_FLAG, 0x55);
    }

    for (i = 0; i < PASSWORD_LEN; i++)
    {
        password[i] = read_external_eeprom(ADDR_PWD + i);
    }

    log_pos = read_external_eeprom(ADDR_LOG_POS);
    log_count = read_external_eeprom(ADDR_LOG_COUNT);

    /* blank or wrong values -> start fresh */
    if (log_pos >= MAX_LOGS || log_count > MAX_LOGS)
    {
        log_pos = 0;
        log_count = 0;
        save_log_pointers();
    }

    /* power ON is also an event */
    log_event(EV_ON);
}

/* ---------------------------------------------------------------- */
/* 1. dashboard                                                     */
/*    SW1 = collision, SW2 = gear up, SW3 = gear down,              */
/*    SW6 = reverse gear, SW4 = go to password screen               */
/* ---------------------------------------------------------------- */

unsigned char dashboard(unsigned char key, unsigned char first)
{
    static unsigned char last_sec = 0xFF;
    static unsigned char last_speed = 0xFF;
    static unsigned char last_event = 0xFF;
    unsigned char t[3];
    unsigned char speed;
    char text[9];

    if (first)
    {
        clcd_print("TIME     EV  SP ", LINE1(0));
        clcd_print("                ", LINE2(0));

        /* force everything to be drawn again */
        last_sec = 0xFF;
        last_speed = 0xFF;
        last_event = 0xFF;
    }

    if (key == SW1)                         /* collision */
    {
        event = EV_C;
        log_event(event);
    }
    else if (key == SW2)                    /* gear up */
    {
        if (event == EV_ON)                 /* first press after power on -> GN */
        {
            event = EV_GN;
            log_event(event);
        }
        else if (gear < 4)                  /* GN -> G1 -> G2 -> G3 -> G4 */
        {
            gear++;
            event = gear + 1;
            log_event(event);
        }
        else if (gear == 5)                 /* reverse -> neutral */
        {
            gear = 0;
            event = EV_GN;
            log_event(event);
        }
    }
    else if (key == SW3)                    /* gear down */
    {
        if (gear > 0 && gear <= 4)          /* G4 -> G3 -> G2 -> G1 -> GN */
        {
            gear--;
            event = gear + 1;
            log_event(event);
        }
    }
    else if (key == SW6)                    /* reverse */
    {
        if (gear != 5)
        {
            gear = 5;
            event = EV_GR;
            log_event(event);
        }
    }
    else if (key == SW4)                    /* go to password screen */
    {
        return PASSWORD_SCREEN;
    }

    /* time : redraw only when the second changes */
    get_time(t);
    if (t[2] != last_sec)
    {
        last_sec = t[2];
        time_to_text(t, text);
        clcd_print(text, LINE2(0));
    }

    /* speed : redraw only when it changes */
    speed = get_speed();
    if (speed != last_speed)
    {
        last_speed = speed;
        clcd_putch((speed / 10) + '0', LINE2(13));
        clcd_putch((speed % 10) + '0', LINE2(14));
    }

    /* event : redraw only when it changes */
    if (event != last_event)
    {
        last_event = event;
        clcd_print(ev_name[event], LINE2(9));
    }

    return DASHBOARD_SCREEN;
}

/* ---------------------------------------------------------------- */
/* 2. password screen                                               */
/*    SW1 = 0, SW2 = 1, SW5 = back                                  */
/* ---------------------------------------------------------------- */

static char entered[PASSWORD_LEN];
static unsigned char pw_index = 0;
static unsigned char attempts = 0;

static void draw_password_screen(void)
{
    clcd_print("ENTER PASSWORD  ", LINE1(0));
    clcd_print("                ", LINE2(0));
    pw_index = 0;
}

/* wrong password 3 times -> wait 60 seconds (counted with the RTC) */
static void lockout_wait(void)
{
    unsigned char left = LOCK_SECONDS;
    unsigned char last, now;
    char text[17];

    clcd_print("YOU ARE BLOCKED ", LINE1(0));
    clcd_print("TRY AFTER 60 SEC", LINE2(0));

    last = read_ds1307(SEC_ADDR);

    while (left > 0)
    {
        now = read_ds1307(SEC_ADDR);

        if (now != last)                    /* one second is over */
        {
            last = now;
            left--;

            text[0] = 'T'; text[1] = 'R'; text[2] = 'Y'; text[3] = ' ';
            text[4] = 'A'; text[5] = 'F'; text[6] = 'T'; text[7] = 'E';
            text[8] = 'R'; text[9] = ' ';
            text[10] = (left / 10) + '0';
            text[11] = (left % 10) + '0';
            text[12] = ' ';
            text[13] = 'S'; text[14] = 'E'; text[15] = 'C';
            text[16] = '\0';
            clcd_print(text, LINE2(0));
        }
    }
}

unsigned char password_screen(unsigned char key, unsigned char first)
{
    unsigned char i, same;
    char text[17];

    if (first)
    {
        draw_password_screen();
    }

    if (key == SW5)
    {
        return DASHBOARD_SCREEN;
    }

    if (key == SW1 || key == SW2)
    {
        if (key == SW1)
        {
            entered[pw_index] = '0';
        }
        else
        {
            entered[pw_index] = '1';
        }

        clcd_putch('*', LINE2(6 + pw_index));
        pw_index++;

        if (pw_index == PASSWORD_LEN)
        {
            wait_ms(300);

            same = 1;
            for (i = 0; i < PASSWORD_LEN; i++)
            {
                if (entered[i] != password[i])
                {
                    same = 0;
                }
            }

            if (same)
            {
                attempts = 0;
                clcd_print("PASSWORD CORRECT", LINE1(0));
                clcd_print("                ", LINE2(0));
                wait_ms(1000);
                return MENU_SCREEN;
            }

            /* wrong password */
            attempts++;

            if (attempts >= 3)
            {
                attempts = 0;
                clcd_print("WRONG PASSWORD  ", LINE1(0));
                clcd_print("                ", LINE2(0));
                wait_ms(1000);
                lockout_wait();
            }
            else
            {
                text[0] = 'A'; text[1] = 'T'; text[2] = 'T'; text[3] = 'E';
                text[4] = 'M'; text[5] = 'P'; text[6] = 'T'; text[7] = 'S';
                text[8] = ' ';
                text[9] = 'L'; text[10] = 'E'; text[11] = 'F'; text[12] = 'T';
                text[13] = ':'; text[14] = ' ';
                text[15] = (3 - attempts) + '0';
                text[16] = '\0';

                clcd_print("WRONG PASSWORD  ", LINE1(0));
                clcd_print(text, LINE2(0));
                wait_ms(2000);
            }

            draw_password_screen();
        }
    }

    return PASSWORD_SCREEN;
}

/* ---------------------------------------------------------------- */
/* 3. main menu                                                     */
/*    SW2 = up, SW3 = down, SW4 = select, SW5 = back                */
/* ---------------------------------------------------------------- */

static unsigned char menu_pos = 0;          /* selected item 0 to 4 */
static unsigned char menu_top = 0;          /* item shown on the first line */

static void draw_menu(void)
{
    unsigned char row;

    for (row = 0; row < 2; row++)
    {
        if ((menu_top + row) == menu_pos)
        {
            clcd_putch('*', LINE1(0) + (row * 0x40));
        }
        else
        {
            clcd_putch(' ', LINE1(0) + (row * 0x40));
        }

        clcd_print(menu_item[menu_top + row], LINE1(1) + (row * 0x40));
    }
}

unsigned char menu_screen(unsigned char key, unsigned char first)
{
    if (first)
    {
        menu_pos = 0;
        menu_top = 0;
        draw_menu();
    }

    if (key == SW2 && menu_pos > 0)
    {
        menu_pos--;
        if (menu_pos < menu_top)
        {
            menu_top = menu_pos;
        }
        draw_menu();
    }
    else if (key == SW3 && menu_pos < 4)
    {
        menu_pos++;
        if (menu_pos > menu_top + 1)
        {
            menu_top = menu_pos - 1;
        }
        draw_menu();
    }
    else if (key == SW4)
    {
        /* 0 = view log ... 4 = change password */
        return VIEW_LOG_SCREEN + menu_pos;
    }
    else if (key == SW5)
    {
        return DASHBOARD_SCREEN;
    }

    return MENU_SCREEN;
}

/* ---------------------------------------------------------------- */
/* 4. view log      SW2 = newer, SW3 = older, SW5 = back            */
/* ---------------------------------------------------------------- */

static unsigned char view_index = 0;

static void draw_log(void)
{
    char text[17];

    if (log_count == 0)
    {
        clcd_print("NO LOGS FOUND   ", LINE1(0));
        clcd_print("                ", LINE2(0));
        return;
    }

    clcd_print("# TIME     EV SP", LINE1(0));
    make_log_text(view_index, text);
    clcd_print(text, LINE2(0));
}

unsigned char view_log_screen(unsigned char key, unsigned char first)
{
    if (first)
    {
        view_index = 0;
        draw_log();
    }

    if (key == SW2 && view_index > 0)
    {
        view_index--;
        draw_log();
    }
    else if (key == SW3 && (view_index + 1) < log_count)
    {
        view_index++;
        draw_log();
    }
    else if (key == SW5)
    {
        return MENU_SCREEN;
    }

    return VIEW_LOG_SCREEN;
}

/* ---------------------------------------------------------------- */
/* 5. clear log : clear all logs, then store CL event               */
/* ---------------------------------------------------------------- */

unsigned char clear_log_screen(unsigned char key, unsigned char first)
{
    log_pos = 0;
    log_count = 0;
    save_log_pointers();

    log_event(EV_CL);                       /* record that logs were cleared */

    clcd_print("LOGS CLEARED    ", LINE1(0));
    clcd_print("                ", LINE2(0));
    wait_ms(2000);

    return MENU_SCREEN;
}

/* ---------------------------------------------------------------- */
/* 6. download log : send all logs to Tera Term through UART        */
/* ---------------------------------------------------------------- */

unsigned char download_log_screen(unsigned char key, unsigned char first)
{
    unsigned char i;
    char text[17];

    clcd_print("DOWNLOADING...  ", LINE1(0));
    clcd_print("                ", LINE2(0));

    if (log_count == 0)
    {
        puts("\n\rNo logs found\n\r");
    }
    else
    {
        puts("\n\r--- CAR BLACK BOX LOGS ---\n\r");
        puts("# TIME     EV SP\n\r");

        for (i = 0; i < log_count; i++)
        {
            make_log_text(i, text);
            puts(text);
            puts("\n\r");
        }
    }

    clcd_print("DOWNLOAD DONE   ", LINE1(0));
    wait_ms(2000);

    return MENU_SCREEN;
}

/* ---------------------------------------------------------------- */
/* 7. set time                                                      */
/*    SW1 = next field (HH/MM/SS), SW2 = +, SW3 = -,                */
/*    SW4 = save, SW5 = cancel                                      */
/* ---------------------------------------------------------------- */

static unsigned char set_h, set_m, set_s;
static unsigned char set_field = 0;         /* 0 = HH, 1 = MM, 2 = SS */

static void draw_set_time(void)
{
    unsigned char t[3];
    char text[9];

    if (set_field == 0)
    {
        clcd_print("SET TIME  HH    ", LINE1(0));
    }
    else if (set_field == 1)
    {
        clcd_print("SET TIME  MM    ", LINE1(0));
    }
    else
    {
        clcd_print("SET TIME  SS    ", LINE1(0));
    }

    t[0] = dec_to_bcd(set_h);
    t[1] = dec_to_bcd(set_m);
    t[2] = dec_to_bcd(set_s);
    time_to_text(t, text);

    clcd_print("                ", LINE2(0));
    clcd_print(text, LINE2(4));
}

unsigned char set_time_screen(unsigned char key, unsigned char first)
{
    unsigned char t[3];

    if (first)
    {
        get_time(t);
        set_h = bcd_to_dec(t[0]);
        set_m = bcd_to_dec(t[1]);
        set_s = bcd_to_dec(t[2]);
        set_field = 0;
        draw_set_time();
    }

    if (key == SW1)
    {
        set_field++;
        if (set_field == 3)
        {
            set_field = 0;
        }
        draw_set_time();
    }
    else if (key == SW2)                    /* increment */
    {
        if (set_field == 0)
        {
            set_h++;
            if (set_h == 24) set_h = 0;
        }
        else if (set_field == 1)
        {
            set_m++;
            if (set_m == 60) set_m = 0;
        }
        else
        {
            set_s++;
            if (set_s == 60) set_s = 0;
        }
        draw_set_time();
    }
    else if (key == SW3)                    /* decrement */
    {
        if (set_field == 0)
        {
            if (set_h == 0) set_h = 23; else set_h--;
        }
        else if (set_field == 1)
        {
            if (set_m == 0) set_m = 59; else set_m--;
        }
        else
        {
            if (set_s == 0) set_s = 59; else set_s--;
        }
        draw_set_time();
    }
    else if (key == SW4)                    /* save into the RTC */
    {
        write_ds1307(HOUR_ADDR, dec_to_bcd(set_h));
        write_ds1307(MIN_ADDR, dec_to_bcd(set_m));
        write_ds1307(SEC_ADDR, dec_to_bcd(set_s));

        clcd_print("TIME SAVED      ", LINE1(0));
        clcd_print("                ", LINE2(0));
        wait_ms(1500);
        return MENU_SCREEN;
    }
    else if (key == SW5)
    {
        return MENU_SCREEN;
    }

    return SET_TIME_SCREEN;
}

/* ---------------------------------------------------------------- */
/* 8. change password : enter new one, then enter it again          */
/*    SW1 = 0, SW2 = 1, SW5 = cancel                                */
/* ---------------------------------------------------------------- */

static char new_pwd[PASSWORD_LEN];
static char confirm_pwd[PASSWORD_LEN];
static unsigned char cp_stage = 0;          /* 0 = new, 1 = re-enter */
static unsigned char cp_index = 0;

static void draw_change_password(void)
{
    if (cp_stage == 0)
    {
        clcd_print("ENTER NEW PWD   ", LINE1(0));
    }
    else
    {
        clcd_print("RE-ENTER PWD    ", LINE1(0));
    }

    clcd_print("                ", LINE2(0));
    cp_index = 0;
}

unsigned char change_password_screen(unsigned char key, unsigned char first)
{
    unsigned char i, same;
    char digit;

    if (first)
    {
        cp_stage = 0;
        draw_change_password();
    }

    if (key == SW5)
    {
        return MENU_SCREEN;
    }

    if (key == SW1 || key == SW2)
    {
        if (key == SW1)
        {
            digit = '0';
        }
        else
        {
            digit = '1';
        }

        if (cp_stage == 0)
        {
            new_pwd[cp_index] = digit;
        }
        else
        {
            confirm_pwd[cp_index] = digit;
        }

        clcd_putch('*', LINE2(6 + cp_index));
        cp_index++;

        if (cp_index == PASSWORD_LEN)
        {
            wait_ms(300);

            if (cp_stage == 0)
            {
                cp_stage = 1;               /* ask again */
                draw_change_password();
            }
            else
            {
                same = 1;
                for (i = 0; i < PASSWORD_LEN; i++)
                {
                    if (new_pwd[i] != confirm_pwd[i])
                    {
                        same = 0;
                    }
                }

                if (same)
                {
                    for (i = 0; i < PASSWORD_LEN; i++)
                    {
                        password[i] = new_pwd[i];
                        write_external_eeprom(ADDR_PWD + i, password[i]);
                    }
                    write_external_eeprom(ADDR_PWD_FLAG, 0x55);

                    log_event(EV_CP);       /* record that password was changed */

                    clcd_print("PASSWORD CHANGED", LINE1(0));
                }
                else
                {
                    clcd_print("PWD NOT MATCHED ", LINE1(0));
                }

                clcd_print("                ", LINE2(0));
                wait_ms(2000);
                return MENU_SCREEN;
            }
        }
    }

    return CHANGE_PASSWORD_SCREEN;
} 