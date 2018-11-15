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
 * @file rtc.h
 * @ingroup Drivers
 * @brief   This file provides access to the System's Real-time Clock that maintains
 *          time even through power-loss granted that RTC Backup battery is installed.
 */
#ifndef RTC_H
#define RTC_H
#ifdef __cplusplus
extern "C" {
#endif



/**
 * This struct aligns with the LPC Hardware's consolidated RTC registers.
 * The un-named chars are just for padding according to LPC CTIME registers.
 */
typedef struct {
    /*    - Data-           -Padding-  */
    unsigned sec  : 6;      unsigned : 2;
    unsigned min  : 6;      unsigned : 2;
    unsigned hour : 5;      unsigned : 3;
    unsigned dow  : 3;      unsigned : 5;

    unsigned day  : 5;      unsigned : 3;
    unsigned month: 4;      unsigned : 4;
    unsigned year :12;      unsigned : 4;

    unsigned doy  :12;      unsigned : 20;
} RTC ;

/**
 * Enumeration of the RTC.dow
 */
typedef enum {
    dow_sun = 0,
    dow_mon,
    dow_tue,
    dow_wed,
    dow_thu,
    dow_fri,
    dow_sat,
} __attribute__((packed)) DayOfWeekEnum;

/// Initialize the RTC
void rtc_initialize (void);

/// Get the latest time in RTC structure
RTC  rtc_gettime (void);

/// Sets the RTC time
void rtc_settime (const RTC*);

/// Get the RTC time as string in the format: "MM/DD/YYYY, HH:MM:SS"
const char* rtc_getDateTimeString(char dateTimeString[24]);


#ifdef __cplusplus
}
#endif
#endif
