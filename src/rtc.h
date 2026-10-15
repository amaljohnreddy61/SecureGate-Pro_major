#ifndef __RTC_H
#define __RTC_H

#include "types.h"

void RTC_Init(void);                                  /* starts the clock (01/01/2026 00:00:00) if it is not running yet */
void RTC_Set(u8 hh, u8 mm, u8 ss);                    /* time of day, date is kept */
void RTC_SetDate(u8 dd, u8 mm, u16 yyyy);             /* date, time of day is kept */
void RTC_Get(u8 *dd, u8 *mo, u16 *yyyy, u8 *hh, u8 *mi, u8 *ss);

#endif /* __RTC_H */
