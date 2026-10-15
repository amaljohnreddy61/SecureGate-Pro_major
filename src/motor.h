#ifndef __MOTOR_H
#define __MOTOR_H

#include "types.h"
#include "board_config.h"

void Motor_Init(void);   /* pins as outputs, motor stopped (call once at start-up) */
void Door_Open(void);    /* forward : access granted               */
u8   Door_Close(void);   /* reverse : 1 = door closed, 0 = limit switch never closed (time-out) */
void Door_Deny(void);    /* reverse pulse: access denied / locked  */

#endif /* __MOTOR_H */
