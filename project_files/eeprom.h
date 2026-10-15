#ifndef __EEPROM_H
#define __EEPROM_H

#include "types.h"

#define EEPROM_I2C_ADDR      0x50   /* AT24C256, A0-A2 tied to GND */

/* --- Memory map inside the AT24C256 (32 KB, 15-bit word address) --- */
#define USER_REC_SIZE         32
#define EEPROM_HDR_ADDR       0x0000                       /* 1 byte: user-table initialised marker */
#define EEPROM_USERS_BASE     0x0020
#define EEPROM_ADMIN_PASS_ADDR ((u16)(EEPROM_USERS_BASE + MAX_USERS * USER_REC_SIZE))
#define EEPROM_ADMIN_MAGIC_ADDR ((u16)(EEPROM_ADMIN_PASS_ADDR + PASS_LEN))
#define EEPROM_MAGIC_VAL      0xB1                         /* layout marker (unchanged: existing users are kept) */

typedef struct {
    s8 user_id[ID_LEN];
    s8 password[PASS_LEN];
    u8 fp_id;
    u8 is_active;
    u8 is_locked;
} UserRecord;

u8   EEPROM_Init(void);      /* 0 = OK, 1 = EEPROM not answering. Blank / old-layout chip -> empty user table */
void EEPROM_SaveUser(u8 index, UserRecord *user);
void EEPROM_LoadUser(u8 index, UserRecord *user);

void EEPROM_SaveAdminPass(cs8 *pass);
void EEPROM_LoadAdminPass(s8 *pass_out);

s32  Find_User(cs8 *id);
u8   Alloc_FP_ID(void);      /* lowest unused template ID 1..MAX_USERS, 0 if none */

#endif /* __EEPROM_H */
