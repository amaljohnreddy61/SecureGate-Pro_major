#ifndef __LCD_H__
#define __LCD_H__

#include <lpc214x.h>
#include "types.h"
#include "board_config.h"

/* 8-bit mode, D0-D7 on 8 contiguous pins of one port (board_config.h) */
#if LCD_DATA_PORT == 1
#define LCD_IODIR   IODIR1
#define LCD_IOSET   IOSET1
#define LCD_IOCLR   IOCLR1
#else
#define LCD_IODIR   IODIR0
#define LCD_IOSET   IOSET0
#define LCD_IOCLR   IOCLR0
#endif

#define LCD_D    (0xFFUL << LCD_DATA_BASE_PIN)
#define RS       (1UL << LCD_RS_PIN)           /* P0.12 */
#define EN       (1UL << LCD_EN_PIN)           /* P0.13 */

void LCD_Init(void);
void LCD_Command(u8 cmd);
void LCD_Data(u8 data);
void LCD_String(const char *str);
void LCD_Clear(void);
void LCD_GotoXY(u8 row,u8 col);
void LCD_Integer(s32 num);

/* ---- SecureGate names -> LCD driver ---- */
#define InitLCD()              LCD_Init()
#define StrLCD(s)              LCD_String(s)
#define LCD_SetCursor(r,c)     LCD_GotoXY((r),(c))

#endif
