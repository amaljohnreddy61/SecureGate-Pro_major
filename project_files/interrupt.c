#include "interrupt.h"

volatile u8 admin_menu_request = 0;
volatile u8 in_admin_session    = 0;

void External_ISR(void) __irq {
    admin_menu_request = 1;              /* ISR only raises the flag, as the abstract requires */
    EXTINT = 0x01;                       /* clear EINT0 flag */
    VICVectAddr = 0;
}

void EINT0_Init(void) {
    PINSEL1 &= ~(3UL << 0);
    PINSEL1 |= (1UL << 0);               /* P0.16 = EINT0 */
    EXTMODE  |= 0x01;                    /* edge sensitive */
    EXTPOLAR &= ~0x01;                   /* falling edge */
    EXTINT    = 0x01;                    /* clear stale flag before enabling */
    VICIntSelect &= ~(1UL << 14);
    VICVectAddr1 = (u32)External_ISR;
    VICVectCntl1 = (1UL << 5) | 14;
    VICIntEnable |= (1UL << 14);
}
