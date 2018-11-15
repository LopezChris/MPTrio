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
 * @brief Interrupt Priorities used in the project
 */
#ifndef INTR_PRIORITIES_H__
#define INTR_PRIORITIES_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "core_cm3.h"



/**
 * Interrupt Priorities
 *  0 - Highest
 * 31 - Lowest
 *
 * Interrupts used in this project must NOT use higher priority than IP_SYSCALL, so
 * if IP_SYSCALL is set to 5, then interrupt priority must be higher number than 5
 * in order to set it to lower priority.
 *
 * FromISR() FreeRTOS API cannot be called from interrupts that have higher priority
 * than IP_SYSCALL, which means that:
 *  - YOUR ISR must have numerically equal to or HIGHER value than IP_SYSCALL
 *
 * The interrupts that use higher priority than IP_SYSCALL MUST NOT USE FREERTOS API.
 */
typedef enum
{
    /**
     * @{ FreeRTOS Interrupt Priorities
     * Be careful changing these because interrupt priorities set below
     * must fall in between these two priorities.
     */
        IP_KERNEL = 31,
        IP_SYSCALL = 5,
    /** @} */

    /**
     * @{ Interrupt Priorities
     * These are used at to set default priorities before main() is called.
     * All interrupts use default priority unless otherwise stated in this enum.
     *
     * If you don't want interrupts to nest, set them to the same priority as IP_DEFAULT
     *
     * @warning
     * RIT is special because it allows us to deal with FreeRTOS API in case FreeRTOS
     * is not running.  If RIT is NOT below SYSCALL, then all interrupts will get locked out
     * because FreeRTOS API may set BASEPRI register to IP_SYSCALL blocking all interrupts
     * below it.  We set RIT to higher priority than FreeRTOS, and we make sure we do
     * not use FreeRTOS API in RIT interrupt.  The RIT will then "recover" base priority
     * and set it to zero if it sees that FreeRTOS is not running.
     */
        IP_Default = 20,
        IP_Uart = IP_Default,
        IP_I2C  = IP_Default,
        IP_RIT  = IP_SYSCALL - 1,
    /** @} */
}IntrPrioritiesType;

#ifdef __cplusplus
}
#endif
#endif
