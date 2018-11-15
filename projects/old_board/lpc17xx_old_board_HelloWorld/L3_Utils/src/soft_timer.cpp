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
#include "soft_timer.hpp"


SoftTimer::SoftTimer() :
    mTargetMs(0), mIntervalMs(0), mPrevMs(0), mTargetMsOverflow(false)
{
    /* Nothing to do */
}
SoftTimer::SoftTimer(portTickType ms) :
    mTargetMs(0), mIntervalMs(0), mPrevMs(0), mTargetMsOverflow(false)
{
    reset(ms);
}
bool SoftTimer::expired(void)
{
    /* Cannot expire if timer is not set */
    if (0 == mIntervalMs) {
        return false;
    }

    const portTickType msNow = xTaskGetMsCount();
    const bool osTimerOverflow = (msNow < mPrevMs);
    bool timerExpired = false;

    /**
     * If our target overflowed, we have to wait for the os ticks to
     * also overflow, otherwise there's no way our timer can expire.
     */
    if (mTargetMsOverflow) {
        if (osTimerOverflow) {
            timerExpired = (msNow >= mTargetMs);
            mTargetMsOverflow = false;
        }
    }
    else {
        /**
         * If we did not overflow, but the os tick overflowed, then for sure
         * the timer has expired.
         */
        if (osTimerOverflow) {
            timerExpired = true;
        }
        else {
            timerExpired = (msNow >= mTargetMs);
        }
    }

    mPrevMs = msNow;
    return timerExpired;
}
void SoftTimer::restart(void)
{
    const portTickType nextTargetMs = mIntervalMs + mTargetMs;
    mTargetMsOverflow = (nextTargetMs < mTargetMs);
    mTargetMs = nextTargetMs;
}
void SoftTimer::reset(portTickType ms)
{
    const portTickType msNow = xTaskGetMsCount();
    mIntervalMs = ms;
    mTargetMs = msNow + mIntervalMs;
    mTargetMsOverflow = (mTargetMs < msNow);
}
void SoftTimer::stop(void)
{
    mIntervalMs = 0;
    mTargetMs = 0;
}
