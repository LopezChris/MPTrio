/**
 * This is the absolute bare minimum program for LPC17xx
 * Use this project if you really want to build your project from scratch.
 */

#include "LPC17xx.h"
#include "system_LPC17xx.h"

void delay()
{
	volatile int i = 10 * 500000;
	while (--i);
}

void uartInit(unsigned int baudRate)
{
	LPC_SC->PCONP |= (1 << 3);			// Enable power to UART0
	LPC_PINCON->PINSEL0 &= ~(0xF << 4); // Clear values
	LPC_PINCON->PINSEL0 |= (0x5 << 4);  // Set values for UART0 Rx/Tx

	LPC_SC->PCLKSEL0 &= ~(3 << 6);	// Uart Clock = PCLK/4

	LPC_UART0->LCR = (1 << 7);	// Enable DLAB
	LPC_UART0->DLM = 0;
	LPC_UART0->DLL = (SystemCoreClock / 4) / (16 * baudRate);
	LPC_UART0->LCR = 3;			// 8-bit data
}

void uart0Putchar(char out)
{
	while(! (LPC_UART0->LSR & (1 << 6)));
	LPC_UART0->THR = out;
}

void uart0Puts(char* str)
{
	while(*str) {
		uart0Putchar(*str++);
	}
}

int main(void)
{
	SystemInit();
	uartInit(38400);

	while (1)
	{
		uart0Putchar('-');
		delay();
	}

	return 0;
}
