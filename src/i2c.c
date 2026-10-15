
#include "types.h"
#include "i2c.h"
#include "i2c_defines.h"
#include "delay.h"

u8 i2c_error = 0;                       

#define I2C_TIMEOUT   300000UL          
#define SCL_MASK      (1UL << 2)
#define SDA_MASK      (1UL << 3)
#define I2C_CLR_ALL   0x6C             


static void i2c_bus_recover(void)
{
	u8 i;
	PINSEL0 &= ~I2C0_PINSEL_MASK;       /* P0.2 / P0.3 = GPIO */
	IODIR0  &= ~SDA_MASK;               /* SDA input  */
	IODIR0  |=  SCL_MASK;               /* SCL output */
	IOSET0 = SCL_MASK;
	delay_us(10);
	for (i = 0; i < 9 && !(IOPIN0 & SDA_MASK); i++) {
		IOCLR0 = SCL_MASK; delay_us(10);
		IOSET0 = SCL_MASK; delay_us(10);
	}
	IOCLR0 = SCL_MASK; delay_us(10);    /* STOP: SDA low -> high while SCL high */
	IODIR0 |= SDA_MASK; IOCLR0 = SDA_MASK; delay_us(10);
	IOSET0 = SCL_MASK;  delay_us(10);
	IOSET0 = SDA_MASK;  delay_us(10);
	IODIR0 &= ~(SDA_MASK | SCL_MASK);   /* release both lines */
}

void I2C0_Init(void)
{
	i2c_bus_recover();
	//cfg p0.2,p0.3 as SCL0,SDA0
	PINSEL0 &= ~I2C0_PINSEL_MASK;
	PINSEL0 |= SCL0_PIN_FUNC | SDA0_PIN_FUNC;
	//cfg I2C0 speed
	I2C0SCLH = LOADVAL;
	I2C0SCLL = LOADVAL;
	//I2C0 peripheral enable
	I2C0CONCLR = I2C_CLR_ALL;
	I2C0CONSET = 1 << I2EN_BIT;
}

void i2c_clear_error(void)
{
	i2c_error = 0;
}

static void i2c_fail(void)
{
	i2c_error = 1;
	I2C0_Init();                        /* reset the peripheral and free the bus */
}

static u8 i2c_wait_si(void)
{
	u32 t = I2C_TIMEOUT;
	while (((I2C0CONSET >> SI_BIT) & 1) == 0) {
		if (--t == 0) { i2c_fail(); return 0; }
	}
	return 1;
}

void i2c_start(void)
{
	if (i2c_error) return;
	//issue start condition
	I2C0CONSET = 1 << STA_BIT;
	//wait for SI bit status
	if (!i2c_wait_si()) return;
	//clear start condition
	I2C0CONCLR = 1 << STA_BIT;
}

void i2c_stop(void)
{
	u32 t = I2C_TIMEOUT;
	if (i2c_error) return;
	//issue stop condition
	I2C0CONSET = 1 << STO_BIT;
	//clr SI bit status
	I2C0CONCLR = 1 << SI_BIT;
	//stop will be cleared automatically
	while ((I2C0CONSET >> STO_BIT) & 1) {
		if (--t == 0) { i2c_fail(); return; }
	}
}

/* returns 1 if the slave acknowledged (SLA+W / data / SLA+R), 0 on NACK or time-out */
u8 i2c_write(u8 dat)
{
	u8 st;
	if (i2c_error) return 0;
	//put data into I2C0DAT
	I2C0DAT = dat;
	//clr SI, leave STA/AA alone
	I2C0CONCLR = 1 << SI_BIT;
	//wait for SI bit status
	if (!i2c_wait_si()) return 0;
	st = (u8)I2C0STAT;
	return (u8)(st == 0x18 || st == 0x28 || st == 0x40);
}

void i2c_restart(void)
{
	if (i2c_error) return;
	//re-start condition
	I2C0CONSET = 1 << STA_BIT;
	//clr SI bit
	I2C0CONCLR = 1 << SI_BIT;
	//wait for SI bit status
	if (!i2c_wait_si()) return;
	//clear re-start condition
	I2C0CONCLR = 1 << STA_BIT;
}

u8 i2c_nack(void)
{
	if (i2c_error) return 0xFF;
	//assert not-ack
	I2C0CONCLR = 1 << AA_BIT;
	//clr SI, issue nack
	I2C0CONCLR = 1 << SI_BIT;
	if (!i2c_wait_si()) return 0xFF;
	return I2C0DAT;
}


u8 i2c_probe(u8 addr7)
{
	u8 ok;
	i2c_start();
	ok = i2c_write((u8)(addr7 << 1));
	i2c_stop();
	return (u8)(ok && !i2c_error);
}
