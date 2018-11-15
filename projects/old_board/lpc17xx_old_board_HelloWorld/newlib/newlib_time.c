#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "rtc.h"

static void rtcRead (struct tm *theTime, unsigned int *milliseconds)
{
    RTC t = rtc_gettime();
    theTime->tm_sec   = t.sec;
    theTime->tm_min   = t.min;
    theTime->tm_hour  = t.hour;
    theTime->tm_mday  = t.day;
    theTime->tm_mon   = t.month - 1;
    theTime->tm_year  = t.year - 1900;
    theTime->tm_wday  = t.dow;
    theTime->tm_yday  = t.doy - 1;
    theTime->tm_isdst = 0;

    // TO DO Also calculate milliseconds somehow ?
}

static void rtcWrite (struct tm *theTime)
{
    RTC t = { 0 };
    t.sec = theTime->tm_sec;
    t.min = theTime->tm_min;
    t.hour= theTime->tm_hour;
    t.day = theTime->tm_mday;
    t.month = theTime->tm_mon;
    t.year = theTime->tm_year + 1900;
    t.dow = theTime->tm_wday;
    t.doy = theTime->tm_yday;
    rtc_settime(&t);
}

static time_t rtcGetEpochSeconds (unsigned int *milliseconds)
{
    struct tm tm;
    rtcRead (&tm, milliseconds);
    return mktime (&tm);
}

int _gettimeofday (struct timeval *tp, void *tzp)
{
    if (tp)
    {
        unsigned int milliseconds = 0;
        tp->tv_sec = rtcGetEpochSeconds (&milliseconds);
        tp->tv_usec = milliseconds * 1000;
    }
    return 0;
}

int _settimeofday (struct timeval *tp, void *tzp)
{
    if (tp)
    {
        struct tm * timeinfo = localtime(&(tp->tv_sec));
        rtcWrite(timeinfo);
    }
}
