//i2c.h
#ifndef __I2C_H
#define __I2C_H

#include "types.h"

extern u8 i2c_error;            /* 1 = last transfer timed out / not acknowledged */

void I2C0_Init(void);           /* also frees a stuck bus (9 clock pulses + STOP) */
void i2c_clear_error(void);
void i2c_start(void);
void i2c_stop(void);
u8   i2c_write(u8 dat);         /* 1 = acknowledged */
void i2c_restart(void);
u8   i2c_nack(void);
u8   i2c_probe(u8 addr7);       /* 1 = a device answers at addr7 */

#endif /* __I2C_H */
