#ifndef __ADMIN_H
#define __ADMIN_H

#include "types.h"

/* Current admin PIN, held in RAM, mirrored to EEPROM on change. */
extern s8 admin_password[PASS_LEN];

void Admin_Menu(void);
void Edit_Password(void);
void Edit_Fingerprint(void);         /* enrol / delete by User ID, or delete all */
void Add_User(void);
void Delete_User(void);
void Unlock_User(void);
void Set_Clock(void);                /* time (HHMMSS) and date (DDMMYY) for the log stamps */
void System_Lockdown(u32 seconds);   /* intrusion response: door reverse + timed lock-out */

#endif /* __ADMIN_H */
