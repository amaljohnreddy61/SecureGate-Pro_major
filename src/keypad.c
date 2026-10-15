/* keypad.c */

#include "types.h"
#include "keypad.h"
#include "delay.h"
#include "interrupt.h"
#include "lcd.h"

#define R1 (1<<16)
#define R2 (1<<17)
#define R3 (1<<18)
#define R4 (1<<19)

#define C1 (1<<20)
#define C2 (1<<21)
#define C3 (1<<22)
#define C4 (1<<23)

static const char KeyMap[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

static void Keypad_Init(void)
{
    IODIR1 |= (R1|R2|R3|R4);

    IODIR1 &= ~(C1|C2|C3|C4);

    IOSET1 = R1|R2|R3|R4;
}

#define KEY_DEBOUNCE_MS 20

/* Returns 1 for a genuine key press, 0 if it was only noise.
 * Waits for the key to be released and lets the contact settle again. */
static u8 Key_Confirm(u32 col)
{
    delay_ms(KEY_DEBOUNCE_MS);          /* let the press bounce settle */

    if(IOPIN1 & col)                    /* released again -> noise */
        return 0;

    while(!(IOPIN1 & col));             /* wait for release */

    delay_ms(KEY_DEBOUNCE_MS);          /* let the release bounce settle */
    return 1;
}

/* Scans the keypad once. Returns the key, or 0 if none was pressed. */
static u8 Keypad_Poll(void)
{
    u8 row;

    {
        for(row=0; row<4; row++)
        {
            IOSET1 = R1|R2|R3|R4;

            switch(row)
            {
                case 0: IOCLR1 = R1; break;
                case 1: IOCLR1 = R2; break;
                case 2: IOCLR1 = R3; break;
                case 3: IOCLR1 = R4; break;
            }

            delay_ms(1);

            if(!(IOPIN1 & C1))
            {
                if(Key_Confirm(C1))
                    return KeyMap[row][0];
            }

            if(!(IOPIN1 & C2))
            {
                if(Key_Confirm(C2))
                    return KeyMap[row][1];
            }

            if(!(IOPIN1 & C3))
            {
                if(Key_Confirm(C3))
                    return KeyMap[row][2];
            }

            if(!(IOPIN1 & C4))
            {
                if(Key_Confirm(C4))
                    return KeyMap[row][3];
            }
        }
    }
    return 0;   /* no key pressed in this pass */
}

void Init_KPM(void)
{
    Keypad_Init();
}

u32 KeyScan(void)
{
    u8 key;

    for(;;)
    {
        /* bail out if the admin switch was pressed */
        if (admin_menu_request && !in_admin_session)
            return 0;

        key = Keypad_Poll();
        if (key)
            return key;
    }
}

/* Digits only. '#' = enter (ignored on an empty entry), '*' = backspace.
 * Typing stops at max_len-1 digits; the entry only finishes when '#' is pressed.
 * Returns 0 only if the admin switch interrupted the entry. */
u8 Get_Input(s8 *buf, u8 max_len, s8 mask_char)
{
    u8 idx = 0;
    u8 key;

    buf[0] = '\0';
    for (;;)
    {
        key = (u8)KeyScan();
        if (key == 0 || (admin_menu_request && !in_admin_session))
        {
            buf[0] = '\0';
            return 0;
        }
        if (key == '#')
        {
            if (idx) break;
            continue;
        }
        if (key == '*')
        {
            if (idx)
            {
                idx--;
                LCD_Command(0x10);          /* cursor left  */
                LCD_Data(' ');
                LCD_Command(0x10);
            }
            continue;
        }
        if (key >= '0' && key <= '9' && idx < max_len - 1)
        {
            buf[idx++] = key;
            LCD_Data(mask_char ? mask_char : key);
        }
    }
    buf[idx] = '\0';
    return 1;
}
