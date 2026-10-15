#include "admin.h"
#include "board_config.h"
#include "lcd.h"
#include "keypad.h"
#include "eeprom.h"
#include "r305.h"
#include "uart0.h"
#include "interrupt.h"
#include "delay.h"
#include "motor.h"
#include "rtc.h"
#include <string.h>
#include <stdio.h>

s8 admin_password[PASS_LEN] = "1234";

static void Msg(cs8 *l1, cs8 *l2, u32 ms)
{
    LCD_Clear();
    StrLCD(l1);
    if (l2) { LCD_SetCursor(1, 0); StrLCD(l2); }
    delay_ms(ms);
}


static u8 Ask_New_Password(s8 *out)
{
    s8 p1[PASS_LEN], p2[PASS_LEN];

    LCD_Clear(); StrLCD("Enter New Pass:"); LCD_SetCursor(1, 0);
    if (!Get_Input(p1, PASS_LEN, '*')) return 0;
    if (strlen(p1) < MIN_PASS_LEN) { Msg("TOO SHORT!", "MIN 4 DIGITS", 1500); return 0; }

    LCD_Clear(); StrLCD("Confirm Pass:"); LCD_SetCursor(1, 0);
    if (!Get_Input(p2, PASS_LEN, '*')) return 0;

    if (strcmp(p1, p2) != 0) { Msg("MISMATCH! ABORT", "NO CHANGE", 1500); return 0; }
    strcpy(out, p1);
    return 1;
}

void Edit_Password(void)
{
    s8 id[ID_LEN], cur[PASS_LEN], np[PASS_LEN], log_msg[50];
    s32 slot;
    UserRecord user;

    LCD_Clear(); StrLCD("ID (0 = ADMIN):"); LCD_SetCursor(1, 0);
    if (!Get_Input(id, ID_LEN, 0)) return;

    if (strcmp(id, "0") == 0) {
        LCD_Clear(); StrLCD("Enter Curr Pass:"); LCD_SetCursor(1, 0);
        if (!Get_Input(cur, PASS_LEN, '*')) return;
        if (strcmp(cur, admin_password) != 0) { Msg("WRONG PASSWORD!", 0, 1500); return; }
        if (!Ask_New_Password(np)) return;
        strcpy(admin_password, np);
        EEPROM_SaveAdminPass(admin_password);
        Msg("PASS CHANGED!", 0, 1500);
        Log_Event("ADMIN - PASSWORD UPDATED");
        return;
    }

    slot = Find_User(id);
    if (slot == -1) { Msg("USER NOT FOUND", 0, 1500); return; }
    EEPROM_LoadUser((u8)slot, &user);

    LCD_Clear(); StrLCD("Enter Curr Pass:"); LCD_SetCursor(1, 0);
    if (!Get_Input(cur, PASS_LEN, '*')) return;
    if (strcmp(cur, user.password) != 0) { Msg("WRONG PASSWORD!", 0, 1500); return; }
    if (!Ask_New_Password(np)) return;

    strcpy(user.password, np);
    EEPROM_SaveUser((u8)slot, &user);
    Msg("PASS CHANGED!", 0, 1500);
    sprintf(log_msg, "USER %s - PASSWORD UPDATED", id);
    Log_Event(log_msg);
}

/* Two-scan enrolment: image -> CharBuffer1, lift finger, image -> CharBuffer2,
 * merge (RegModel), store at fp_id. 1 = success. */
static u8 Enroll_Fingerprint(u8 fp_id)
{
    u8 c;

    Msg("PLACE FINGER...", 0, 0);
    c = R305_WaitFinger(FP_WAIT_MS);
    if (c != R305_OK || R305_GenerateChar(1) != R305_OK) goto fail;

    Msg("REMOVE FINGER", 0, 0);
    if (R305_WaitFingerRemoved(FP_WAIT_MS) != R305_OK) goto fail;

    Msg("PLACE SAME", "FINGER AGAIN", 0);
    c = R305_WaitFinger(FP_WAIT_MS);
    if (c != R305_OK || R305_GenerateChar(2) != R305_OK) goto fail;

    if (R305_RegModel() != R305_OK) { Msg("FINGERS DIFFER", "TRY AGAIN", 1500); return 0; }
    if (R305_StoreTemplate(1, fp_id) != R305_OK) goto fail;
    return 1;
fail:
    Msg("ENROLL FAILED", 0, 1500);
    return 0;
}

/* Ask for a User ID and load that user. Returns the slot, or -1 if cancelled / not found. */
static s32 Pick_User(UserRecord *user, s8 *id)
{
    s32 slot;

    LCD_Clear(); StrLCD("Enter User ID:"); LCD_SetCursor(1, 0);
    if (!Get_Input(id, ID_LEN, 0)) return -1;
    slot = Find_User(id);
    if (slot == -1) { Msg("USER NOT FOUND", 0, 1500); return -1; }
    EEPROM_LoadUser((u8)slot, user);
    return slot;
}

void Edit_Fingerprint(void)
{
    s8 key, id[ID_LEN], log_msg[50];
    UserRecord user;

    LCD_Clear();
    StrLCD("1ENR 2DEL 3ALL");
    LCD_SetCursor(1, 0);
    StrLCD("FINGERPRINT");
    key = (s8)KeyScan();

    if (key == '3') {                                /* delete every fingerprint in the module */
        Msg("DELETE ALL FP?", "# YES  * NO", 0);
        if (KeyScan() != '#') { Msg("CANCELLED", 0, 1500); return; }
        if (R305_EmptyLibrary() == R305_OK) {
            Msg("ALL FP DELETED", "RE-ENROLL USERS", 2000);
            Log_Event("ADMIN - ALL FINGERPRINTS DELETED");
        } else {
            Msg("DELETE FAILED", 0, 1500);
        }
        return;
    }

    if (key != '1' && key != '2') return;
    if (Pick_User(&user, id) == -1) return;          /* the user's own template ID is used */

    if (key == '1') {
        if (Enroll_Fingerprint(user.fp_id)) {
            Msg("FP ENROLLED!", 0, 1500);
            sprintf(log_msg, "USER %s - FINGERPRINT ENROLLED", id);
            Log_Event(log_msg);
        }
    } else {
        if (R305_DeleteTemplate(user.fp_id) == R305_OK) {
            Msg("FP DELETED!", "RE-ENROLL USER", 1500);
            sprintf(log_msg, "USER %s - FINGERPRINT DELETED", id);
            Log_Event(log_msg);
        } else {
            Msg("DELETE FAILED", 0, 1500);
        }
    }
}

void Add_User(void)
{
    UserRecord user;
    s8 id[ID_LEN], pass[PASS_LEN], cpass[PASS_LEN], log_msg[50];
    s32 free_slot = -1, i;
    u8 fp_id;

    for (i = 0; i < MAX_USERS && free_slot == -1; i++) {
        EEPROM_LoadUser((u8)i, &user);
        if (!user.is_active) free_slot = i;
    }
    fp_id = Alloc_FP_ID();
    if (free_slot == -1 || fp_id == 0) { Msg("MEMORY FULL!", 0, 1500); return; }

    LCD_Clear(); StrLCD("Enter New ID:"); LCD_SetCursor(1, 0);
    if (!Get_Input(id, ID_LEN, 0)) return;

    if (Find_User(id) != -1) {
        Msg("USER ID EXISTS", "TRY ANOTHER ID", 2000);
        return;
    }

    LCD_Clear(); StrLCD("Enter Password:"); LCD_SetCursor(1, 0);
    if (!Get_Input(pass, PASS_LEN, '*')) return;
    if (strlen(pass) < MIN_PASS_LEN) { Msg("TOO SHORT!", "MIN 4 DIGITS", 1500); return; }

    LCD_Clear(); StrLCD("Confirm Password"); LCD_SetCursor(1, 0);
    if (!Get_Input(cpass, PASS_LEN, '*')) return;
    if (strcmp(pass, cpass) != 0) { Msg("PASS MISMATCH", 0, 1500); return; }

    if (!Enroll_Fingerprint(fp_id)) return;      /* nothing is saved unless the finger is stored */

    memset(&user, 0, sizeof(user));
    strcpy(user.user_id, id);
    strcpy(user.password, pass);
    user.fp_id = fp_id;
    user.is_active = 1;
    user.is_locked = 0;
    EEPROM_SaveUser((u8)free_slot, &user);

    Msg("USER ADDED!", 0, 1500);
    sprintf(log_msg, "USER %s - NEW USER ADDED", id);        Log_Event(log_msg);
    sprintf(log_msg, "USER %s - FINGERPRINT ENROLLED", id);  Log_Event(log_msg);
}

void Delete_User(void)
{
    s8 id[ID_LEN], log_msg[60];
    u32 confirm_key;
    s32 slot;
    UserRecord user;

    LCD_Clear(); StrLCD("Enter User ID:"); LCD_SetCursor(1, 0);
    if (!Get_Input(id, ID_LEN, 0)) return;

    slot = Find_User(id);
    if (slot == -1) { Msg("USER NOT FOUND", 0, 1500); return; }
    EEPROM_LoadUser((u8)slot, &user);

    Msg("DELETE USER?", "# YES  * NO", 0);
    confirm_key = KeyScan();

    if (confirm_key == '#') {
        R305_DeleteTemplate(user.fp_id);
        user.is_active = 0;
        EEPROM_SaveUser((u8)slot, &user);
        Msg("USER DELETED", "SUCCESSFULLY", 1500);
        sprintf(log_msg, "USER %s: USER RECORD DELETED", id);
        Log_Event(log_msg);
    } else {
        Msg("CANCELLED", 0, 1500);
    }
}

void Unlock_User(void)
{
    s8 id[ID_LEN], log_msg[50];
    s32 slot;
    UserRecord user;

    LCD_Clear(); StrLCD("Enter User ID:"); LCD_SetCursor(1, 0);
    if (!Get_Input(id, ID_LEN, 0)) return;

    slot = Find_User(id);
    if (slot == -1) { Msg("USER NOT FOUND", 0, 1500); return; }

    EEPROM_LoadUser((u8)slot, &user);
    if (!user.is_locked) { Msg("NOT LOCKED", 0, 1500); return; }

    user.is_locked = 0;
    EEPROM_SaveUser((u8)slot, &user);
    Msg("ACCOUNT UNLOCKED", 0, 1500);
    sprintf(log_msg, "USER %s - ACCOUNT UNLOCKED", id);
    Log_Event(log_msg);
}

static u8 Days_In_Month(u8 mm, u16 yyyy)
{
    static const u8 dim[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (mm == 2 && ((yyyy % 4 == 0 && yyyy % 100 != 0) || yyyy % 400 == 0)) return 29;
    return dim[mm - 1];
}

#define D2(p) ((u8)(((p)[0] - '0') * 10 + ((p)[1] - '0')))

void Set_Clock(void)
{
    s8 t[ID_LEN], d[ID_LEN], log_msg[50];
    u8 hh, mi, ss, dd, mo;
    u16 yyyy;

    LCD_Clear(); StrLCD("TIME HHMMSS:"); LCD_SetCursor(1, 0);
    if (!Get_Input(t, ID_LEN, 0)) return;
    if (strlen(t) != 6) { Msg("USE 6 DIGITS", 0, 1500); return; }
    hh = D2(t); mi = D2(t + 2); ss = D2(t + 4);
    if (hh > 23 || mi > 59 || ss > 59) { Msg("INVALID TIME", 0, 1500); return; }

    LCD_Clear(); StrLCD("DATE DDMMYY:"); LCD_SetCursor(1, 0);
    if (!Get_Input(d, ID_LEN, 0)) return;
    if (strlen(d) != 6) { Msg("USE 6 DIGITS", 0, 1500); return; }
    dd = D2(d); mo = D2(d + 2); yyyy = (u16)(2000 + D2(d + 4));
    if (mo < 1 || mo > 12 || dd < 1 || dd > Days_In_Month(mo, yyyy)) { Msg("INVALID DATE", 0, 1500); return; }

    RTC_Set(hh, mi, ss);
    RTC_SetDate(dd, mo, yyyy);
    Msg("CLOCK SET", 0, 1000);
    sprintf(log_msg, "ADMIN - CLOCK SET %02u/%02u/%04u %02u:%02u:%02u",
            (unsigned int)dd, (unsigned int)mo, (unsigned int)yyyy,
            (unsigned int)hh, (unsigned int)mi, (unsigned int)ss);
    Log_Event(log_msg);
}

void System_Lockdown(u32 seconds)
{
    u32 s;
    Log_Event("SYSTEM LOCKDOWN - REPEATED UNAUTHORIZED ATTEMPTS");
    Door_Deny();                                  /* reverse = door held closed / locked */
    for (s = seconds; s > 0; s--) {
        LCD_Clear();
        StrLCD("SYSTEM LOCKED");
        LCD_SetCursor(1, 0);
        StrLCD("WAIT ");
        LCD_Integer((s32)s);
        StrLCD("s");
        delay_ms(1000);
    }
    Log_Event("SYSTEM LOCKDOWN ENDED");
}

void Admin_Menu(void)
{
    s8 key;
    s8 pass[PASS_LEN];
    u8 tries = 0, ok = 0;

    in_admin_session = 1;

    while (tries < MAX_ATTEMPTS && !ok) {
        LCD_Clear();
        StrLCD("ADMIN PIN:");
        LCD_SetCursor(1, 0);
        if (!Get_Input(pass, PASS_LEN, '*')) { in_admin_session = 0; return; }
        if (strcmp(pass, admin_password) == 0) {
            ok = 1;
        } else {
            tries++;
            Msg("WRONG PIN!", 0, 1000);
            Log_Event("ADMIN - WRONG PIN");
        }
    }

    if (!ok) {
        Log_Event("ADMIN - ACCESS DENIED");
        System_Lockdown(LOCKDOWN_SEC);
        in_admin_session = 0;
        return;
    }

    Log_Event("ADMIN - LOGIN SUCCESSFUL");

    while (admin_menu_request) {
        LCD_Clear();
        StrLCD("1PW 2FP 3AD 4DL");
        LCD_SetCursor(1, 0);
        StrLCD("5EX AUN BLG CLK");
        key = (s8)KeyScan();

        switch (key) {
            case '1': Edit_Password();     break;
            case '2': Edit_Fingerprint();  break;
            case '3': Add_User();          break;
            case '4': Delete_User();       break;
            case '5': admin_menu_request = 0; break;
            case 'A': Unlock_User();       break;
            case 'B': View_Logs();         break;
            case 'C': Set_Clock();         break;
            default: break;
        }
    }

    Log_Event("ADMIN - LOGOUT");
    in_admin_session = 0;
}
