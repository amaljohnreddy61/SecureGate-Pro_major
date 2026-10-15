//uart0.c
#include "types.h"
#include "board_config.h"
#include "uart0.h"
#include "uart0_defines.h"
#include "uart1.h"
#include "r305.h"
#include "lcd.h"
#include "delay.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>

static u32 log_counter = 1;
static s8  log_history[LOG_HISTORY_SIZE][LOG_LINE_LEN];
static u8  log_head  = 0;
static u8  log_count = 0;


static void LOG_TxStr(cs8 *p)
{
	if (r305_uart == 0) UART1_TxStr(p);
	else                U0_TxStr(p);
}

void UART_Init(void)
{
	//P0.0 = TXD0, P0.1 = RXD0
	PINSEL0 &= ~UART0_PINSEL_MASK;
	PINSEL0 |= TXD0_PIN | RXD0_PIN;
	//8N1, DLAB on, set baud, DLAB off
	U0LCR = (1 << DLAB_BIT) | WORD_LEN_8BIT;
	U0DLL = UART0_DIVISOR & 0xFF;
	U0DLM = (UART0_DIVISOR >> 8) & 0xFF;
	U0LCR = WORD_LEN_8BIT;
	U0FCR = 0x07;                        /* enable + reset FIFOs */
}

void U0_Tx(u8 sByte)
{
	U0THR = sByte;
	while (((U0LSR >> TEMT_BIT) & 1) == 0);      /* wait for transmit to complete */
}

void U0_SetBaud(u32 baud)
{
	u16 div = (u16)(PCLK_FREQ / (16 * baud));
	U0LCR = (1 << DLAB_BIT) | WORD_LEN_8BIT;
	U0DLL = div & 0xFF;
	U0DLM = (div >> 8) & 0xFF;
	U0LCR = WORD_LEN_8BIT;
}

void U0_Flush(void)
{
	volatile u8 d;
	while (U0LSR & (1 << DR_BIT))
		d = U0RBR;
	(void)d;
}

u8 U0_ReadTimeout(u8 *ch, u32 timeout_ms)
{
	u32 n = timeout_ms * 50;             /* 20 us per poll */
	while (n--) {
		if (U0LSR & (1 << DR_BIT)) {
			*ch = U0RBR;
			return 1;
		}
		delay_us(20);
	}
	return 0;
}

void U0_TxStr(cs8 *p)
{
	while (*p)
		U0_Tx(*p++);
}

/* Heading of the access log (start-up, and again for View Logs) */
void Log_Banner(void)
{
	LOG_TxStr("\r\n----------------------------------------\r\n"
	          "        SECUREGATE PRO ACCESS LOG\r\n"
	          "----------------------------------------\r\n\r\n");
}

/* "LOG nnn: [DD/MM/YYYY HH:MM:SS] message" */
void Log_Event(cs8 *message)
{
	s8  buf[LOG_LINE_LEN];
	u8  dd, mo, hh, mi, ss;
	u16 yyyy;

	RTC_Get(&dd, &mo, &yyyy, &hh, &mi, &ss);
	sprintf(buf, "LOG %03u: [%02u/%02u/%04u %02u:%02u:%02u] %.56s\r\n",
	        (unsigned int)log_counter++,
	        (unsigned int)dd, (unsigned int)mo, (unsigned int)yyyy,
	        (unsigned int)hh, (unsigned int)mi, (unsigned int)ss, message);
	LOG_TxStr(buf);

	
	strncpy(log_history[log_head], buf, LOG_LINE_LEN - 1);
	log_history[log_head][LOG_LINE_LEN - 1] = '\0';
	log_head = (u8)((log_head + 1) % LOG_HISTORY_SIZE);
	if (log_count < LOG_HISTORY_SIZE)
		log_count++;
}

void View_Logs(void)
{
	u8 i, idx;

	Log_Banner();
	idx = (u8)((log_head + LOG_HISTORY_SIZE - log_count) % LOG_HISTORY_SIZE);
	for (i = 0; i < log_count; i++)
	{
		LOG_TxStr(log_history[idx]);
		idx = (u8)((idx + 1) % LOG_HISTORY_SIZE);
	}
	LOG_TxStr("\r\n------------- END OF LOG ---------------\r\n");

	Log_Event("ADMIN - LOGS VIEWED");

	LCD_Clear();
	StrLCD("LOGS SENT");
	LCD_SetCursor(1, 0);
	StrLCD("VIA UART");
	delay_ms(1500);
}
