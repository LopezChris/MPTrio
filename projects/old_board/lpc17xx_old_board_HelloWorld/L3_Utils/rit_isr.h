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

/**
 * @file
 * @brief Provides api to setup RIT (repetitive interrupt) callback
 */
#ifndef RIT_ISR_H__
#define RIT_ISR_H__
#ifdef __cplusplus
extern "C" {
#endif



/** Void function pointer */
typedef void (*voidFuncRitPtr)(void);

/**
 * Setup Periodic RIT callback.  Once a callback is attached, the watchdog will also be reset
 * everytime the callback is called.
 * @param function  Your function to callback.  You should not use FreeRTOS API in this callback.
 * @param timeMs    The frequency of callback in milliseconds.
 *
 * ********************
 *   @WARNING
 * ********************
 *  The RIT must be enabled by calling this function because the RIT ISR also helps us set
 *  BASEPRI to 0 in case FreeRTOS has modified it and FreeRTOS is NOT running.
 *  See more notes at the source code of this file.
 */
void rit_setup_callback(voidFuncRitPtr funcPtrNoFreeRtosApi, unsigned int timeMs);

/**
 * If RIT timer is enabled, we get a free running timer for free.
 * @returns the timer value in ms.  Resolution of this timer depends
 *          on timeMs parameter of rit_setup_callback()
 */
unsigned int rit_get_timer_value_ms(void);



#ifdef __cplusplus
}
#endif
#endif
