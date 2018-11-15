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
 * @brief Provides reason of reboot cause
 */
#ifndef REBOOT_CAUSE_H__
#define REBOOT_CAUSE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "rtc.h"



/**
 * Enumeration of the reboot type
 */
typedef enum {
    rebootType_unknown,
    rebootType_powerOn,
    rebootType_reset,
    rebootType_watchdog,
    rebootType_watchdogRecover,
    rebootType_brownOut
}RebootType;

/**
 * @returns the reboot reason as detected during system startup
 */
RebootType getRebootCause();

/**
 * @returns the boot-time recorded in the system
 */
RTC getBootTime();



#ifdef __cplusplus
}
#endif
#endif
