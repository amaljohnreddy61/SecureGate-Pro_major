//keypad.h
#ifndef __KEYPAD_H
#define __KEYPAD_H

#include "types.h"

void Init_KPM(void);                                  /* 4x4 keypad: rows P1.16-19, columns P1.20-23 */
u32  KeyScan(void);                                   /* blocks for a key; 0 if the admin switch was pressed */
u8   Get_Input(s8 *buf, u8 max_len, s8 mask_char);    /* digits, '#' = enter, '*' = backspace; 0 = interrupted */

#endif /* __KEYPAD_H */
