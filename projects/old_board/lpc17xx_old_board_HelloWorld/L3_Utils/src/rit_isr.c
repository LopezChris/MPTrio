/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */
#include "rit_isr.h"
#include "LPC17xx.h"
#include "sys_config.h" /** getCpuClock() */
#include "watchdog.h"   /** Feed the watchdog while we're running RIT */

#include "FreeRTOS.h"
#include "task.h"

static voidFuncRitPtr rit_callback = 0;               /**< RIT Callback function pointer */
static unsigned int rit_timer_tick_ms = 0;            /**< Milliseconds per callback */
static volatile unsigned int rit_timer_value_ms = 0;  /**< Running milliseconds value */



void RIT_IRQHandler()
{
    rit_timer_value_ms += rit_timer_tick_ms;

    watchdog_Feed();
    rit_callback();

    // Clear Interrupt Flag
    LPC_RIT->RICTRL |= 1;

    /**
     * XXX This is a hack.
     * FreeRTOS api resets our base priority register and our interrupts get locked out.
     * This is small hack to set the base priority to zero if FreeRTOS is not running.
     * @see more notes at isr_piorirites.h
     */
    if (taskSCHEDULER_RUNNING != xTaskGetSchedulerState()) {
        __set_BASEPRI(0);
    }
}


unsigned int rit_get_timer_value_ms(void)
{
    return rit_timer_value_ms;
}

void rit_setup_callback(voidFuncRitPtr function, unsigned int timeMs)
{
    if (0 == function) {
        return;
    }
    // Divide by zero guard
    if(0 == timeMs) {
        timeMs = 1;
    }
    rit_timer_tick_ms = timeMs;

    // Power up first otherwise writing to RIT will give us Hard Fault
    LPC_SC->PCONP |= (1 << 16);

    // Enable CLK/1 to simplify RICOMPVAL calculation below
    LPC_SC->PCLKSEL1 &= ~(3 << 26);
    LPC_SC->PCLKSEL1 |=  (1 << 26);

    LPC_RIT->RICTRL = 0;
    LPC_RIT->RICOUNTER = 0;
    LPC_RIT->RIMASK = 0;
    LPC_RIT->RICOMPVAL = getCpuClock() / (1000 / timeMs);

    // Clear timer upon match, and enable timer
    LPC_RIT->RICTRL = (1 << 0) | (1 << 1) | (1 << 3);

    // Enable System Interrupt and connect the callback
    rit_callback = function;
    NVIC_EnableIRQ(RIT_IRQn);
}
