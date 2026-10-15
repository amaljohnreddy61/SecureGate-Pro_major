//uart1_defines.h
#ifndef __UART1_DEFINES_H
#define __UART1_DEFINES_H

//pin function selection (P0.8 = TXD1, P0.9 = RXD1)
#define UART1_PINSEL_MASK 0x000F0000   //bits[19:16]
#define TXD1_PIN  0x00010000
#define RXD1_PIN  0x00040000

//U1LCR
#define WORD_LEN_8BIT 0x03
#define DLAB_BIT      7

//U1LSR
#define DR_BIT    0
#define THRE_BIT  5

#endif /* __UART1_DEFINES_H */
