#ifndef TIMER0_H_
#define TIMER0_H_
#include "LPC17xx.h"



static inline void timer0_init(void)
{
    // Setup timer service
    LPC_TIM0->TCR = 1;  // Enable Timer
    LPC_TIM0->CTCR = 0; // Increment on PCLK

    // Set CPU / 1 PCLK
    LPC_SC->PCLKSEL0 &= ~(3 << 2);
    LPC_SC->PCLKSEL0 |=  (1 << 2);

    // Enable 1ms resolution
    LPC_TIM0->PR = (SystemCoreClock / 1000);
}

static inline void timer0_deinit(void)
{
    // Reset timer service
    LPC_TIM0->TCR = 0;
    LPC_TIM0->CTCR = 0;
    LPC_SC->PCLKSEL0 &= ~(3 << 2);
    LPC_TIM0->PR = 0;
}

static unsigned int timer0_getMsValue(void)
{
    return LPC_TIM0->TC;
}

#endif /* TIMER0_H_ */
