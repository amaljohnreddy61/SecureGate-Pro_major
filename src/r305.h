//r305.h
#ifndef __R305_H
#define __R305_H

#include "types.h"
#include "r305_defines.h"

extern u8  r305_uart;      /* UART the R305 is on (0/1); the PC log uses the other */
extern u32 r305_baud;      /* baud rate found / used for the R305 */
extern u32 r305_pass;      /* handshake password that was accepted (or the configured one) */
extern u8  r305_pwd_ok;    /* 1 = module accepted the handshake password */

u8   R305_AutoDetect(void);                      /* 1 = module answered on some UART/baud, 0 = fallback */

u8   R305_CollectImage(void);
u8   R305_GenerateChar(u8 buffer_id);
u8   R305_RegModel(void);
u8   R305_StoreTemplate(u8 buffer_id, u16 page_id);
s32  R305_SearchFinger(u8 buffer_id, u16 *matched_id);
u8   R305_DeleteTemplate(u16 page_id);
u8   R305_EmptyLibrary(void);                    /* delete every stored fingerprint */


u8   R305_WaitFinger(u32 timeout_ms);
u8   R305_WaitFingerRemoved(u32 timeout_ms);

#endif /* __R305_H */
