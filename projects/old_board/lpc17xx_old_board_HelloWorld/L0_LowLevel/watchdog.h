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
 * @ingroup Drivers
 */
#ifndef WATCHDOG_H__
#define WATCHDOG_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "LPC17xx.h"


/**
 * Watchdog timeout in milliseconds
 * Value cannot be greater than 1,000,000 which is too large of a value
 * to set for a useful watchdog timer anyway.
 */
#define WATCHDOG_TIMEOUT_MS     (10 * 1000) // <-- 10 second watchdog


/**
 * Watchdog feed resets the watchdog timeout.
 * Periodic feeds should keep the system running, if the  watchdog
 * feeds stop, system will reset after watchdog timeout.
 */
inline void watchdog_Feed()
{
    LPC_WDT->WDFEED = 0xAA;
    LPC_WDT->WDFEED = 0x55;
}

/**
 * Resets the system immediately.
 */
inline void watchdog_RebootSystem()
{
    /**
     * From the datasheet:
     * After writing 0xAA to WDFEED, access to any Watchdog register other than writing
     * 0x55 to WDFEED causes an immediate reset/interrupt when the Watchdog is enabled.
     */
    LPC_WDT->WDFEED = 0xAA;
    LPC_WDT->WDMOD = 0;
}

/**
 * Enables watchdog with reset mode
 * Value of timeout is defined at this file.
 */
inline void watchdog_Enable()
{
    /**
     * 1 / (4Mhz/4) = Watchdog clock by default (WCLK)
     * Therefore watchdog clock = 1Mhz = 1uS
     * Each value of WDTC means 1uS
     */
    LPC_WDT->WDTC = WATCHDOG_TIMEOUT_MS * 1000;

    /**
     * Enable Watchdog and Watchdog reset enable
     * Once enabled, this cannot be cleared
     */
    LPC_WDT->WDMOD = 3;
    watchdog_Feed();
}


#ifdef __cplusplus
}
#endif
#endif /* WATCHDOG_H__ */
