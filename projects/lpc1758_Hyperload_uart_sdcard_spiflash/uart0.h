#ifndef UART0_H_
#define UART0_H_
#include "LPC17xx.h"
#include "timer0.h"


/// Set to non-zero if you wish to use UART2 as well as UART0
#define UART2   0



void delay_ms(unsigned int ms)
{
    ms += timer0_getMsValue();
    while(timer0_getMsValue() < ms);
}

static void uart_init(unsigned int baudRate)
{
    LPC_SC->PCONP |= (1 << 3);          // Enable power to UART0
    LPC_PINCON->PINSEL0 &= ~(0xF << 4); // Clear values
    LPC_PINCON->PINSEL0 |= (0x5 << 4);  // Set values for UART0 Rx/Tx

    LPC_SC->PCLKSEL0 &= ~(3 << 6);    // Uart Clock = PCLK/4
    LPC_SC->PCLKSEL0 |= (1 << 6);     // Uart Clock = PCLK / 1

    LPC_UART0->LCR = (1 << 7);  // Enable DLAB
    LPC_UART0->DLM = 0;
    LPC_UART0->DLL = (short)((SystemCoreClock) / (16 * baudRate));
    LPC_UART0->LCR = 3;         // 8-bit data

#if UART2
    LPC_SC->PCONP |= (1 << 24);          // Enable power to UART2
    LPC_PINCON->PINSEL4 &= ~(0xF << 16); // Clear values
    LPC_PINCON->PINSEL4 |=  (0xA << 16); // Set values for UART2 Rx/Tx

    // Set UART2 Peripheral Clock divider to 1
    LPC_SC->PCLKSEL1 &= ~(3 << 16);
    LPC_SC->PCLKSEL1 |=  (1 << 16);

    LPC_UART2->LCR = (1 << 7);  // Enable DLAB
    LPC_UART2->DLM = 0;
    LPC_UART2->DLL = (short)((SystemCoreClock) / (16 * baudRate));
    LPC_UART2->LCR = 3;         // 8-bit data
#endif
}

static void uart_deinit(void)
{
    LPC_PINCON->PINSEL0 &= ~(0xF << 4);  // reset values for UART0 Rx/Tx
    LPC_SC->PCLKSEL0 &= ~(3 << 6);       // Uart Clock = PCLK / 4

    LPC_UART0->LCR = (1 << 7);  // Enable DLAB
    LPC_UART0->DLM = 0;
    LPC_UART0->DLL = 0;
    LPC_UART0->LCR = 0;

#if UART2
    LPC_PINCON->PINSEL4 &= ~(0xF << 16); // Clear values
    LPC_SC->PCLKSEL1 &= ~(3 << 16);      // CLK/4

    LPC_UART2->LCR = (1 << 7);  // Enable DLAB
    LPC_UART2->DLM = 0;
    LPC_UART2->DLL = 0;
    LPC_UART2->LCR = 0;

    LPC_SC->PCONP &= ~(1 << 24);          // Enable power to UART0
#endif
}

static void uart_putchar(char out)
{
    while(! (LPC_UART0->LSR & (1 << 6)));
#if UART2
    while(! (LPC_UART2->LSR & (1 << 6)));
#endif

    LPC_UART0->THR = out;
#if UART2
    LPC_UART2->THR = out;
#endif
}

char uart_getchar(int timeout_ms)
{
    timeout_ms += timer0_getMsValue();

    while(1)
    {
        if (LPC_UART0->LSR & (1 << 0)) {
            return LPC_UART0->RBR;
        }
#if UART2
        if (LPC_UART2->LSR & (1 << 0)) {
            return LPC_UART2->RBR;
        }
#endif
        if (timer0_getMsValue() > timeout_ms) {
            return 0xFF;
        }
    }

    return 0xFF;
}
void uart_put(const char* str)
{
    while(*str) {
        uart_putchar(*str);
        str++;
    }
}
void uart_puts(const char* str)
{
    uart_put(str);
    uart_putchar('\n');
}

void uart_flush_rx()
{
    delay_ms(500);
    volatile char b = 0;
    b = LPC_UART0->RBR;
#if UART2
    b = LPC_UART2->RBR;
#endif
    (void)b; /* avoid warning */
}

#endif /* UART0_H_ */
