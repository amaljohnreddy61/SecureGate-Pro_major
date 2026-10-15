#include "rtc.h"

void RTC_Init(void)
{
    if (CCR & 1) return;                /* already running (VBAT kept it alive) */
    PREINT  = (PCLK_FREQ / 32768) - 1;
    PREFRAC = PCLK_FREQ - ((PREINT + 1) * 32768);
    RTC_Set(0, 0, 0);
    RTC_SetDate(1, 1, 2026);
}

void RTC_Set(u8 hh, u8 mm, u8 ss)
{
    CCR = 0x02;                         /* stop + reset the prescaler */
    HOUR = hh; MIN = mm; SEC = ss;
    CCR = 0x01;                         /* run */
}

void RTC_SetDate(u8 dd, u8 mm, u16 yyyy)
{
    CCR = 0x00;                         /* stop (counters keep their value) */
    DOM = dd; MONTH = mm; YEAR = yyyy;
    CCR = 0x01;                         /* run */
}

void RTC_Get(u8 *dd, u8 *mo, u16 *yyyy, u8 *hh, u8 *mi, u8 *ss)
{
    u32 t0, t1, t0b;

    do {                                /* re-read if a second ticked between the two reads */
        t0  = CTIME0;
        t1  = CTIME1;
        t0b = CTIME0;
    } while (t0 != t0b);

    *ss   = (u8)(t0 & 0x3F);
    *mi   = (u8)((t0 >> 8) & 0x3F);
    *hh   = (u8)((t0 >> 16) & 0x1F);
    *dd   = (u8)(t1 & 0x1F);
    *mo   = (u8)((t1 >> 8) & 0x0F);
    *yyyy = (u16)((t1 >> 16) & 0xFFF);
}
