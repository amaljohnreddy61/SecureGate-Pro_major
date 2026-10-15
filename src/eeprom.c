#include "eeprom.h"
#include "i2c.h"
#include "delay.h"
#include <string.h>
#include <stddef.h>

static void i2c_eeprom_bytewrite(u8 slaveAddr, u16 wAddr, u8 dat) {
    i2c_start();
    i2c_write(slaveAddr << 1);
    i2c_write((u8)((wAddr >> 8) & 0xFF));  /* address MSB */
    i2c_write((u8)(wAddr & 0xFF));         /* address LSB */
    i2c_write(dat);                        /* actual data byte */
    i2c_stop();
    delay_ms(10);   /* internal write cycle time */
}

static u8 i2c_eeprom_randomread(u8 slaveAddr, u16 rAddr) {
    u8 dat;
    i2c_start();
    i2c_write(slaveAddr << 1);
    i2c_write((u8)((rAddr >> 8) & 0xFF));  /* address MSB */
    i2c_write((u8)(rAddr & 0xFF));         /* address LSB */
    i2c_restart();
    i2c_write((slaveAddr << 1) | 1);
    dat = i2c_nack();
    i2c_stop();
    return dat;
}

u8 EEPROM_Init(void) {
    u8 i, zero = 0;
    i2c_clear_error();
    I2C0_Init();                                    
    if (!i2c_probe(EEPROM_I2C_ADDR)) return 1;     
    if (i2c_eeprom_randomread(EEPROM_I2C_ADDR, EEPROM_HDR_ADDR) == EEPROM_MAGIC_VAL)
        return i2c_error ? 1 : 0;
   
    for (i = 0; i < MAX_USERS; i++) {
        i2c_eeprom_bytewrite(EEPROM_I2C_ADDR,
            (u16)(EEPROM_USERS_BASE + i * USER_REC_SIZE + offsetof(UserRecord, is_active)), zero);
        i2c_eeprom_bytewrite(EEPROM_I2C_ADDR,
            (u16)(EEPROM_USERS_BASE + i * USER_REC_SIZE + offsetof(UserRecord, is_locked)), zero);
    }
    i2c_eeprom_bytewrite(EEPROM_I2C_ADDR, EEPROM_HDR_ADDR, EEPROM_MAGIC_VAL);
    return i2c_error ? 1 : 0;
}

void EEPROM_SaveUser(u8 index, UserRecord *user) {
    u16 addr = EEPROM_USERS_BASE + (u16)(index * USER_REC_SIZE);
    u8 *ptr = (u8 *)user;
    u32 i;
    for (i = 0; i < sizeof(UserRecord); i++) {
        i2c_eeprom_bytewrite(EEPROM_I2C_ADDR, addr + (u16)i, ptr[i]);
    }
}

void EEPROM_LoadUser(u8 index, UserRecord *user) {
    u16 addr = EEPROM_USERS_BASE + (u16)(index * USER_REC_SIZE);
    u8 *ptr = (u8 *)user;
    u32 i;
    for (i = 0; i < sizeof(UserRecord); i++) {
        ptr[i] = i2c_eeprom_randomread(EEPROM_I2C_ADDR, addr + (u16)i);
    }
}

void EEPROM_SaveAdminPass(cs8 *pass) {
    u32 i;
    for (i = 0; i < PASS_LEN; i++) {
        i2c_eeprom_bytewrite(EEPROM_I2C_ADDR, EEPROM_ADMIN_PASS_ADDR + (u16)i, (u8)pass[i]);
    }
    i2c_eeprom_bytewrite(EEPROM_I2C_ADDR, EEPROM_ADMIN_MAGIC_ADDR, EEPROM_MAGIC_VAL);
}

void EEPROM_LoadAdminPass(s8 *pass_out) {
    u8 magic = i2c_eeprom_randomread(EEPROM_I2C_ADDR, EEPROM_ADMIN_MAGIC_ADDR);
    u32 i;

    if (magic != EEPROM_MAGIC_VAL) {
        /* EEPROM never initialized: fall back to factory default and store it */
        strcpy(pass_out, "1234");
        EEPROM_SaveAdminPass(pass_out);
        return;
    }

    for (i = 0; i < PASS_LEN; i++) {
        pass_out[i] = (s8)i2c_eeprom_randomread(EEPROM_I2C_ADDR, EEPROM_ADMIN_PASS_ADDR + (u16)i);
    }
}

s32 Find_User(cs8 *id) {
    UserRecord user;
    s32 i;
    for (i = 0; i < MAX_USERS; i++) {
        EEPROM_LoadUser((u8)i, &user);
        if (user.is_active && strcmp(user.user_id, id) == 0) {
            return i;
        }
    }
    return -1;
}

u8 Alloc_FP_ID(void) {
    UserRecord user;
    u8 id, i, used;
    for (id = 1; id <= MAX_USERS; id++) {
        used = 0;
        for (i = 0; i < MAX_USERS; i++) {
            EEPROM_LoadUser(i, &user);
            if (user.is_active && user.fp_id == id) { used = 1; break; }
        }
        if (!used) return id;
    }
    return 0;
}
