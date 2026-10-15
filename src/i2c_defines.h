//i2c_defines.h
#ifndef __I2C_DEFINES_H
#define __I2C_DEFINES_H
//defines for pin function selection (P0.2=SCL0,P0.3=SDA0)
#define I2C0_PINSEL_MASK 0x000000F0   //bits[7:4]: P0.2 and P0.3 function fields
#define SCL0_PIN_FUNC    0x00000010
#define SDA0_PIN_FUNC    0x00000040

//defines for I2C0 speed configuration
#define I2C_SPEED  100000    //Hz, standard mode
#define LOADVAL    ((PCLK_FREQ/I2C_SPEED)/2)

//bit defines for I2C0CONSET/I2C0CONCLR sfr
#define   AA_BIT 2
#define   SI_BIT 3
#define  STO_BIT 4
#define  STA_BIT 5
#define I2EN_BIT 6

#endif /* __I2C_DEFINES_H */
