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
#include "lpc_pwm.hpp"
#include "sys_config.h"



static unsigned int gTcMax = 0;
PWM::PWM(pwmType pwm, unsigned int frequencyHz) :
    mPwm(pwm)
{
    if (0 == gTcMax) {
        gTcMax = (getCpuClock() / frequencyHz);

        LPC_SC->PCONP |= (1 << 6);      // Enable Power
        LPC_SC->PCLKSEL0 &= ~(3 << 12); // Clear PCLK Divider
        LPC_SC->PCLKSEL0 |=  (1 << 12); // Set divider = 1

        // Reset on PWMMR0: the PWMTC will be reset if PWMMR0 matches it.
        LPC_PWM1->MCR |= (1 << 1);

        // Suppose CPU = 48,000,000 (48Mhz)
        // frequency desired = 50   (50Hz)
        // PWM1MR0 will be ~ 960,000 to produce 50Khz PWM
        LPC_PWM1->MR0 = gTcMax;

        LPC_PWM1->TCR = (1 << 0) | (1 << 3); // Enable PWM1
        LPC_PWM1->CTCR &= ~(0xF << 0);
    }

    switch(mPwm)
    {
        // PINSEL, and enable the PWM channel :
        case pwm2:
            LPC_PINCON->PINSEL3 &= ~(3 << 8);
            LPC_PINCON->PINSEL3 |=  (2 << 8);
            LPC_PWM1->PCR |= (1 << 10);
            break;
        case pwm4:
            LPC_PINCON->PINSEL3 &= ~(3 << 14);
            LPC_PINCON->PINSEL3 |=  (2 << 14);
            LPC_PWM1->PCR |= (1 << 12);
            break;
        case pwm5:
            LPC_PINCON->PINSEL3 &= ~(3 << 16);
            LPC_PINCON->PINSEL3 |=  (2 << 16);
            LPC_PWM1->PCR |= (1 << 13);
            break;
        default :
            break;
    }
}

bool PWM::set(float percent)
{
    if(percent > 100) {
        return false;
    }

    // Get the value from the percent
    // If percent = 50, mTcMax = 1000, then value will be 50*1000 / 100 = 500
    const unsigned int valueNeeded = (percent * gTcMax) / 100;

    switch(mPwm)
    {
        case pwm2: LPC_PWM1->MR2 = valueNeeded;  break;
        case pwm4: LPC_PWM1->MR4 = valueNeeded;  break;
        case pwm5: LPC_PWM1->MR5 = valueNeeded;  break;
        default : return false;
    }

    // Enable the latch
    LPC_PWM1->LER |= (1 << mPwm);
    return true;
}
