//interrupt.h
#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "types.h"

/* Admin-menu request switch on EINT0 (P0.16). The ISR only raises a flag; main() runs the menu. */
extern volatile u8 admin_menu_request;   /* set by ISR, consumed in main() */
extern volatile u8 in_admin_session;     /* true while Admin_Menu() is running */

void EINT0_Init(void);
void External_ISR(void) __irq;

#endif /* __INTERRUPT_H */
