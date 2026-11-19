/* 
 * File:   car_black_box.h
 * Author: Alisha Shelke 
 *
 * Created on 17 November, 2026, 5:22 PM
 */

#ifndef CAR_BLACK_BOX_H
#define CAR_BLACK_BOX_H

/* screens */
#define DASHBOARD_SCREEN        0
#define PASSWORD_SCREEN         1
#define MENU_SCREEN             2
#define VIEW_LOG_SCREEN         3
#define CLEAR_LOG_SCREEN        4
#define DOWNLOAD_LOG_SCREEN     5
#define SET_TIME_SCREEN         6
#define CHANGE_PASSWORD_SCREEN  7

/* events (index in the event name table) */
#define EV_ON                   0
#define EV_GN                   1
#define EV_G1                   2
#define EV_G2                   3
#define EV_G3                   4
#define EV_G4                   5
#define EV_GR                   6
#define EV_C                    7
#define EV_CL                   8       /* clear log       */
#define EV_CP                   9       /* change password */

/* every screen returns the next screen, 'first' is 1 when the screen is just opened */
void init_black_box(void);
unsigned char dashboard(unsigned char key, unsigned char first);
unsigned char password_screen(unsigned char key, unsigned char first);
unsigned char menu_screen(unsigned char key, unsigned char first);
unsigned char view_log_screen(unsigned char key, unsigned char first);
unsigned char clear_log_screen(unsigned char key, unsigned char first);
unsigned char download_log_screen(unsigned char key, unsigned char first);
unsigned char set_time_screen(unsigned char key, unsigned char first);
unsigned char change_password_screen(unsigned char key, unsigned char first);

#endif