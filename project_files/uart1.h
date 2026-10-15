//uart1.h
#ifndef __UART1_H
#define __UART1_H

#include "types.h"

void UART1_Init(u32 baud);               /* also routes P0.8/P0.9 to TXD1/RXD1 */
void UART1_Flush(void);
void UART1_SendChar(u8 ch);
void UART1_TxStr(cs8 *p);
u8   UART1_ReadTimeout(u8 *ch, u32 timeout_ms);   /* 1 = byte read, 0 = timed out */

#endif /* __UART1_H */
