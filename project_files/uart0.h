//uart0.h
#ifndef __UART0_H
#define __UART0_H

#include "types.h"

void UART_Init(void);
void U0_Tx(u8 sByte);
void U0_SetBaud(u32 baud);
void U0_Flush(void);
u8   U0_ReadTimeout(u8 *ch, u32 timeout_ms);   /* 1 = byte read, 0 = timed out */
void U0_TxStr(cs8 *p);
void Log_Banner(void);                         /* "SECUREGATE PRO ACCESS LOG" heading */
void Log_Event(cs8 *message);
void View_Logs(void);

#endif /* __UART0_H */
