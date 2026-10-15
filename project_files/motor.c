#include "motor.h"
#include "delay.h"

static void motor_on(u8 forward)
{
#if MOTOR_EN_ENABLED
    IOSET0 = MOTOR_EN;
#endif
    if (forward) { IOSET0 = MOTOR_IN1; IOCLR0 = MOTOR_IN2; }
    else         { IOCLR0 = MOTOR_IN1; IOSET0 = MOTOR_IN2; }
}

static void motor_off(void)
{
    IOCLR0 = MOTOR_IN1 | MOTOR_IN2;
#if MOTOR_EN_ENABLED
    IOCLR0 = MOTOR_EN;
#endif
}

void Motor_Init(void)
{
    motor_off();                                   /* drive low before the pins become outputs */
    IODIR0 |= MOTOR_IN1 | MOTOR_IN2;
#if MOTOR_EN_ENABLED
    IODIR0 |= MOTOR_EN;
#endif
#if DOOR_SWITCH_ENABLED
    IODIR0 &= ~DOOR_SW;                            /* input */
#endif
    motor_off();
}

void Door_Open(void)
{
    motor_on(1);
    delay_ms(DOOR_OPEN_MS);
    motor_off();
}

#if DOOR_SWITCH_ENABLED
static u8 door_is_closed(void) { return (IOPIN0 & DOOR_SW) == 0; }
#endif

u8 Door_Close(void)
{
#if DOOR_SWITCH_ENABLED
    u32 t;
    u8 closed = 0;

    motor_on(0);
    for (t = 0; t < DOOR_CLOSE_MS && !closed; t += 10) {
        delay_ms(10);
        if (door_is_closed()) {                    /* debounce: must still be closed 20 ms later */
            delay_ms(20);
            t += 20;
            closed = door_is_closed();
        }
    }
    motor_off();
    return closed;
#else
    motor_on(0);
    delay_ms(DOOR_CLOSE_MS);
    motor_off();
    return 1;
#endif
}

void Door_Deny(void)
{
    motor_on(0);
    delay_ms(DOOR_DENY_MS);
    motor_off();
}
