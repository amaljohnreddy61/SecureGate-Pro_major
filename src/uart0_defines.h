//uart0_defines.h
#ifndef __UART0_DEFINES_H
#define __UART0_DEFINES_H

//pin function selection (P0.0 = TXD0, P0.1 = RXD0)
#define UART0_PINSEL_MASK 0x0000000F   //bits[3:0]
#define TXD0_PIN  0x00000001
#define RXD0_PIN  0x00000004

//baud rate at power-up (R305_AutoDetect() changes it later)
#include "board_config.h"
#define UART0_BAUD    ((R305_UART == 0) ? R305_BAUD : LOG_BAUD)
#define UART0_DIVISOR (PCLK_FREQ/(16*UART0_BAUD))

//U0LCR
#define WORD_LEN_8BIT 0x03
#define DLAB_BIT      7

//U0LSR
#define DR_BIT    0
#define TEMT_BIT  6

//access-log ring buffer
#define LOG_HISTORY_SIZE 25
#define LOG_LINE_LEN     96

#endif /* __UART0_DEFINES_H */
