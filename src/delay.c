#include "delay.h"


void delay_ms(u32 ms)
{
	u32 i;
	volatile u32 j;
	for (i = 0; i < ms; i++)
		for (j = 0; j < 6000; j++);
}

void delay_us(u32 us)
{
	u32 i;
	volatile u32 j;
	for (i = 0; i < us; i++)
		for (j = 0; j < 6; j++);
}
