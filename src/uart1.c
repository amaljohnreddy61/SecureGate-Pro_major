//uart1.c
#include "types.h"
#include "uart1.h"
#include "uart1_defines.h"
#include "delay.h"

void UART1_Init(u32 baud)
{
	u16 divisor = (u16)(PCLK_FREQ / (16 * baud));
	PINSEL0 &= ~UART1_PINSEL_MASK;
	PINSEL0 |= TXD1_PIN | RXD1_PIN;
	U1LCR = (1 << DLAB_BIT) | WORD_LEN_8BIT;
	U1DLL = divisor & 0xFF;
	U1DLM = (divisor >> 8) & 0xFF;
	U1LCR = WORD_LEN_8BIT;
	U1FCR = 0x07;                        /* enable + reset FIFOs */
}

void UART1_Flush(void)
{
	volatile u8 d;
	while (U1LSR & (1 << DR_BIT))
		d = U1RBR;
	(void)d;
}

void UART1_SendChar(u8 ch)
{
	while (!(U1LSR & (1 << THRE_BIT)));
	U1THR = ch;
}

void UART1_TxStr(cs8 *p)
{
	while (*p)
		UART1_SendChar((u8)*p++);
}

u8 UART1_ReadTimeout(u8 *ch, u32 timeout_ms)
{
	u32 n = timeout_ms * 50;             /* 20 us per poll */
	while (n--) {
		if (U1LSR & (1 << DR_BIT)) {
			*ch = U1RBR;
			return 1;
		}
		delay_us(20);
	}
	return 0;
}
