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
#ifndef SOFT_TIMER_H__
#define SOFT_TIMER_H__

#include "FreeRTOS.h"
#include "task.h"



/**
 * Soft timer class to provide thin layer of a timer.  There is no hard-timer
 * running in the background and this class imply relies on FreeRTOS tick
 * timer to provide timer capability.
 */
class SoftTimer
{
    public:
        SoftTimer();                ///< Constructor
        SoftTimer(portTickType ms); ///< Constructor to construct and set timer value.

        /// @returns true if the timer has expired
        bool expired(void);

        /**
         * Restarts the timer to provide consistent frequency of expired().
         * restart() is better suited to provide same frequency because in case
         * we check for expired() at a later time that runs past our timer 3
         * times, then expired() in combination with restart() will return true
         * 3 times; see example below :
         * @code
         *      timer.reset(10); // Init once
         *
         *      // After reset(), use this as recurring timer :
         *      if (timer.expired()) {
         *          timer.restart();
         *      }
         * @endcode
         */
        void restart(void);

        /**
         * Resets the timer from this point of time.
         * @param ms  The milliseconds at which timer should expire next.
         */
        void reset(portTickType ms);

        /// Stops the timer.
        void stop(void);

        /// @returns true if the timer is set and running
        inline bool isRunning(void) const { return (mIntervalMs > 0); }

    private:
        portTickType mTargetMs;     ///< Expire time with respect to OS tick
        portTickType mIntervalMs;   ///< Timer interval
        portTickType mPrevMs;       ///< previous OS ticks used to determine overflow
        bool mTargetMsOverflow;     ///< Flag if our mTargetMs overflowed
};



#endif
