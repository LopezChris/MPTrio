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


#include <stdio.h>
#include "rtc.h"
#include "LPC17xx.h"
#include "sys_config.h"  // RTC_VALID_RANGE_YEARS



void rtc_initialize (void)
{
    LPC_SC->PCONP |= (1 << 9);  // Enable RTC Power
    LPC_RTC->CCR = 1;           // Enable RTC

    const uint16_t years[] = RTC_VALID_RANGE_YEARS;
    RTC time = rtc_gettime();

    /* Check for invalid time */
    if(time.year < years[0] || time.year > years[1] ||
       time.min >= 60 || time.sec >= 60 || time.hour >= 24 ||
       time.doy > 365 || time.month > 12 || time.day > 31)
    {
        time.day = 1;
        time.month = 1;
        time.year = 2000;

        time.dow = 0;
        time.hour = time.min = time.sec = 0;

        rtc_settime(&time);
    }
}

RTC rtc_gettime (void)
{
    // Read the struct from LPC Memory Map
    return  *(RTC*) (&LPC_RTC->CTIME0) ;
}

void rtc_settime (const RTC *rtc)
{
	/*
	 * Update RTC registers :
	 * Note: Cannot write to consolidated registers since they are read-only
	 */
    LPC_RTC->SEC   = rtc->sec;
	LPC_RTC->MIN   = rtc->min;
	LPC_RTC->HOUR  = rtc->hour;
	LPC_RTC->DOW   = rtc->dow;
	LPC_RTC->DOM   = rtc->day;
	LPC_RTC->MONTH = rtc->month;
	LPC_RTC->YEAR  = rtc->year;
	LPC_RTC->DOY   = rtc->doy;
}

const char* rtc_getDateTimeString(char dateTimeStr[24])
{
    RTC time = rtc_gettime();

    siprintf(dateTimeStr,
                    "%u/%u/%u,%02u:%02u:%02u",
                    time.month, time.day, time.year,
                    time.hour, time.min, time.sec
            );

    return dateTimeStr;
}
