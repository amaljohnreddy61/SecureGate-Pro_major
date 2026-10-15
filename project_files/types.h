//types.h
#ifndef __TYPES_H
#define __TYPES_H

#include <LPC214x.h>

/* Custom Type Definitions */
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;
typedef int            s32;
typedef char           s8;
typedef const char     cs8;

/* ---- System-wide configuration ---- */
#define MAX_USERS        8      /* user records == R305 template slots 1..MAX_USERS */
#define ID_LEN           7      /* up to 6 digits (abstract example: 243562) + '\0' */
#define PASS_LEN         9      /* up to 8 digits + '\0' */
#define MIN_PASS_LEN     4
#define MAX_ATTEMPTS     3
#define LOCKDOWN_SEC     30     /* system lockdown after repeated invalid IDs / admin PINs */
#define PCLK_FREQ        15000000UL   /* CCLK=60MHz (PLL), VPBDIV at reset default -> PCLK=CCLK/4 */

#endif /* __TYPES_H */
