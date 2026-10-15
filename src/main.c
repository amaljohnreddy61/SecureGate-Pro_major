#include "types.h"
#include "board_config.h"
#include "delay.h"
#include "lcd.h"
#include "keypad.h"
#include "uart0.h"
#include "uart1.h"
#include "i2c.h"
#include "eeprom.h"
#include "r305.h"
#include "motor.h"
#include "interrupt.h"
#include "admin.h"
#include "rtc.h"
#include <string.h>
#include <stdio.h>

static void Msg2(cs8 *l1, cs8 *l2, u32 ms)
{
    LCD_Clear();
    StrLCD(l1);
    if (l2) { LCD_SetCursor(1, 0); StrLCD(l2); }
    delay_ms(ms);
}

static u8 eeprom_ok = 0;

/* 1 = EEPROM answered and the admin PIN was loaded; 0 = EEPROM not responding (never hangs) */
static u8 Eeprom_Start(void)
{
    if (EEPROM_Init() != 0) return 0;
    EEPROM_LoadAdminPass(admin_password);
    return (u8)!i2c_error;
}

static void Log_User(cs8 *id, cs8 *text)
{
    s8 msg[48];
    sprintf(msg, "USER %s - %s", id, text);
    Log_Event(msg);
}

s32 main(void) {
    s8 input_id[ID_LEN], input_pass[PASS_LEN];
    s32 user_slot, attempts;
    u16 matched_fp_id;
    u8 invalid_ids = 0;                 /* consecutive unknown IDs  */
    u8 fp_fail[MAX_USERS];              /* consecutive fingerprint mismatches per user slot */
    u8 c, scan_ok;
    UserRecord current_user;
    u8 r305_found;
    s8 info[48];

    memset(fp_fail, 0, sizeof(fp_fail));

    /* ---- Initialize all hardware modules ---- */
    Motor_Init();                       /* first: motor stopped before anything else can move it */
    InitLCD();
    Init_KPM();
    UART_Init();                        /* both UARTs up; R305_AutoDetect() sets the final baud rates */
    UART1_Init(LOG_BAUD);
    I2C0_Init();
    RTC_Init();
    EINT0_Init();

    Msg2("SECUREGATE PRO", "INITIALIZING...", 500);

    r305_found = R305_AutoDetect();     /* finds which UART / baud the R305 answers on */
    if (r305_found) {
        sprintf(info, "R305 UART%u %u", (unsigned int)r305_uart, (unsigned int)r305_baud);
        Msg2(info, "FOUND", 1500);
    } else {
        Msg2("R305 NOT FOUND", "CHECK WIRING", 2000);
    }

    eeprom_ok = Eeprom_Start();         /* blank chip -> empty user table; never blocks */
    if (!eeprom_ok) Msg2("EEPROM ERROR", "CHECK I2C WIRING", 2000);

    Log_Banner();                       /* heading first, as in the reference log */
    Log_Event("SYSTEM STARTED");
    if (r305_found) {
        sprintf(info, "R305 DETECTED ON UART%u AT %u BAUD", (unsigned int)r305_uart, (unsigned int)r305_baud);
        Log_Event(info);
        if (!r305_pwd_ok)
            Log_Event("NOTE - R305 PASSWORD NOT VERIFIED (FINE IF UNCHANGED)");
    } else {
        Log_Event("WARNING - R305 NOT RESPONDING (UART0/1, 9600/57600)");
    }
    if (!eeprom_ok) Log_Event("WARNING - EEPROM NOT RESPONDING (CHECK I2C)");
    Log_Event("SYSTEM INITIALIZED SUCCESSFULLY");

    while (1) {
        /* EEPROM missing or lost: keep retrying, show why, never freeze on an old screen */
        if (!eeprom_ok || i2c_error) {
            eeprom_ok = Eeprom_Start();
            if (!eeprom_ok) {
                Msg2("EEPROM ERROR", "CHECK I2C WIRING", 1500);
                if (admin_menu_request) { EXTINT = 0x01; admin_menu_request = 0; }
                continue;
            }
            Log_Event("EEPROM OK");
        }

        if (admin_menu_request) {
            Admin_Menu();
            delay_ms(300);              /* let the switch bounce die out */
            EXTINT = 0x01;
            admin_menu_request = 0;
            continue;
        }

        LCD_Clear();
        StrLCD("SECUREGATE PRO");
        LCD_SetCursor(1, 0);
        StrLCD("ENTER ID: ");
        if (!Get_Input(input_id, ID_LEN, 0) || admin_menu_request) continue;

        /* ---- Stage 1: User ID ---- */
        user_slot = Find_User(input_id);
        if (user_slot == -1) {
            Msg2("INVALID ID!", 0, 0);
            Log_User(input_id, "INVALID USER ID");
            Log_User(input_id, "ACCESS DENIED");
            Door_Deny();
            if (++invalid_ids >= MAX_ATTEMPTS) {
                invalid_ids = 0;
                System_Lockdown(LOCKDOWN_SEC);
            }
            continue;
        }
        invalid_ids = 0;

        EEPROM_LoadUser((u8)user_slot, &current_user);

        if (current_user.is_locked) {
            Msg2("ACCOUNT LOCKED!", "SEE ADMIN", 0);
            Log_User(input_id, "ACCOUNT LOCKED");
            Log_User(input_id, "ACCESS DENIED");
            Door_Deny();
            continue;
        }
        if (fp_fail[user_slot] >= MAX_ATTEMPTS) fp_fail[user_slot] = 0;   /* stale after an admin unlock */

        Log_User(input_id, "USER ID VERIFIED");

        /* ---- Stage 2: Password ---- */
        attempts = 0;
        c = 0;                                      /* c = password accepted */
        while (attempts < MAX_ATTEMPTS) {
            LCD_Clear();
            StrLCD("ENTER PASS:");
            LCD_SetCursor(1, 0);
            if (!Get_Input(input_pass, PASS_LEN, '*') || admin_menu_request) break;

            if (strcmp(input_pass, current_user.password) == 0) {
                Log_User(input_id, "PASSWORD VERIFIED");
                c = 1;
                break;
            }
            attempts++;
            Msg2("WRONG PASSWORD!", 0, 1000);
            Log_User(input_id, "WRONG PASSWORD");
        }

        if (admin_menu_request) continue;

        if (!c) {                                   /* 3 wrong passwords -> lock the account */
            current_user.is_locked = 1;
            EEPROM_SaveUser((u8)user_slot, &current_user);
            Msg2("ACCOUNT LOCKED!", 0, 0);
            Log_User(input_id, "ACCOUNT LOCKED");
            Log_User(input_id, "ACCESS DENIED");
            Door_Deny();
            continue;
        }

        /* ---- Stage 3: Fingerprint ---- */
        Msg2("PLACE FINGER...", 0, 0);
        scan_ok = 0;
        c = R305_WaitFinger(FP_WAIT_MS);
        if (c == R305_OK && R305_GenerateChar(1) == R305_OK) scan_ok = 1;

        if (!scan_ok) {
            Msg2(c == R305_NO_FINGER ? "NO FINGER" : "SCAN ERROR!", 0, 1500);
            Log_User(input_id, c == R305_NO_FINGER ? "FINGERPRINT TIMEOUT" : "FINGERPRINT SCAN ERROR");
            Log_User(input_id, "ACCESS DENIED");
            Door_Deny();
            continue;
        }

        if (R305_SearchFinger(1, &matched_fp_id) == 0 && matched_fp_id == current_user.fp_id) {
            fp_fail[user_slot] = 0;
            Log_User(input_id, "FINGERPRINT VERIFIED");
            Log_User(input_id, "ACCESS GRANTED");

            Msg2("ACCESS GRANTED", "OPENING DOOR...", 0);
            Door_Open();
            Log_User(input_id, "DOOR OPENED");

            delay_ms(DOOR_HOLD_MS);

            Msg2("CLOSING DOOR...", 0, 0);
            if (Door_Close()) {
                Log_User(input_id, "DOOR CLOSED");
            } else {
                Msg2("DOOR NOT CLOSED", "CHECK DOOR", 2000);
                Log_User(input_id, "WARNING - DOOR NOT CLOSED");
            }
        } else {
            Msg2("BIOMETRIC FAIL!", 0, 0);
            Log_User(input_id, "FINGERPRINT MISMATCH");
            Log_User(input_id, "ACCESS DENIED");
            Door_Deny();
            if (++fp_fail[user_slot] >= MAX_ATTEMPTS) {   /* repeated impostor attempts */
                current_user.is_locked = 1;
                EEPROM_SaveUser((u8)user_slot, &current_user);
                Log_User(input_id, "ACCOUNT LOCKED");
            }
        }
    }
}
