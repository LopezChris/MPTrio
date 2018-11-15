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


#include "../io.hpp" // All IO Class definitions
#include "LPC17xx.h"
#include "adc0.h"
#include "isr_priorities.h"



/**
 * The following diagram shows bit number corresponding to the LED
 *
 *      0
 *     ---
 *  5 | 6 | 1
 *     ---
 *  4 |   | 2
 *     ---
 *      3   *7
 */
static const char LED_DISPLAY_CHARMAP[255] =
{
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,    // 32 : Space
        0x86, // 33 : !
        0,0,0,0, 0,0,0,0, 0,0,0, // 34-44
        0x40, // 45 : -
        0x80, // 46 : .
        0x00, // 47 : /
        0x3F, // 48 : 0
        0x30, // 49 : 1
        0x5B, // 50 : 2
        0x4F, // 51 : 3
        0x66, // 52 : 4
        0x6D, // 53 : 5
        0x7C, // 54 : 6
        0x07, // 55 : 7
        0x7F, // 56 : 8
        0x67, // 57 : 9
        0,0,0,0,0,0,0, // 58-64
        0x77, // 65 : A
        0x7F, // 66 : B
        0x39, // 67 : C
        0x3F, // 68 : D
        0x79, // 69 : E
        0x71, // 70 : F
        0x3D, // 71 : G
        0x76, // 72 : H
        // TODO: Need rest of the LED Display char-map
};



bool Acceleration_Sensor::init()
{
    const unsigned char activeModeWith100Hz = (1 << 0) | (3 << 3); // Active Mode @ 100Hz

    writeReg(Ctrl_Reg1, activeModeWith100Hz);
    const char whoAmIReg = readReg(WhoAmI);

    return (mWhoAmIExpectedValue == whoAmIReg);
}
short Acceleration_Sensor::getX()
{
    return (short)get16BitRegister(X_MSB) / 16;
}
short Acceleration_Sensor::getY()
{
    return (short)get16BitRegister(Y_MSB) / 16;
}
short Acceleration_Sensor::getZ()
{
    return (short)get16BitRegister(Z_MSB) / 16;
}



/**
 * The design of the IR Sensor is as follows:
 *  Timer1 captures falling edges of CAP1.0 and timestamps are saved when this happens.
 *  Every time capture happens, a 20ms match interrupt is setup/reset.  When no more
 *  signals arrive, the 20ms match interrupt will decode the timestamps and convert
 *  timestamps into binary signal.
 *
 *  Note that this doesn't decode into real data sent through the IR, but nevertheless,
 *  the decoded signal will be unique per button pressed on an IR remote.
 */
static unsigned int LAST_DECODED_IR_SIGNAL = 0;
#define TIMER1_US_PER_TICK  (100)
extern "C"
{
    void TIMER1_IRQHandler()
    {
        const unsigned int captureMask = (1 << 4);
        const unsigned int MR0Mask     = (1 << 0);
        const unsigned int ticksFor20Ms = (20 * 1000) / TIMER1_US_PER_TICK;

        const  unsigned short maxFallingEdgesPerIRFrame = 32;
        static unsigned short signalCount = 0;
        static unsigned int signalArray[maxFallingEdgesPerIRFrame] = {0};

        // Capture interrupt occurred:
        if(LPC_TIM1->IR & captureMask)
        {
            /**
             * Reload Match Register Interrupt to interrupt 20ms after this point of time
             * If another capture interrupt arrives later, this timer is reset again.
             */
            LPC_TIM1->MR0 = (LPC_TIM1->TC + ticksFor20Ms);

            // Just store the timestamp of this signal
            if(signalCount < maxFallingEdgesPerIRFrame) {
                signalArray[signalCount++] = LPC_TIM1->CR0;
            }

            // Clear the Timer Capture interrupt
            LPC_TIM1->IR = captureMask;
        }
        // Timeout Interrupt to decode the signal
        else if(LPC_TIM1->IR & MR0Mask)
        {
            if(signalCount > 1)
            {
                /**
                 * Time to decode the signals at this timeout
                 * Calculate differences of falling edges
                 */
                for(int i = 0; i < signalCount-1; i++) {
                    signalArray[i] = signalArray[i+1] - signalArray[i];
                }

                /**
                 * First falling edge value should indicate binary 0.
                 * So anything higher than 50% of this value is considered binary 1.
                 */
                const unsigned int binary1Threshold = signalArray[1] + (signalArray[1]/2);
                unsigned int decodedSignal = 0;
                for(unsigned short i=0; i < signalCount-1; i++) {
                    if(signalArray[i] > binary1Threshold) {
                        //printf("%i, ", signalArray[i]);
                        decodedSignal |= (1 << i);
                    }
                }
                LAST_DECODED_IR_SIGNAL = decodedSignal;
            }

            // Clear the Match Interrupt and signal count
            signalCount = 0;
            LPC_TIM1->IR = MR0Mask;
        }
        else
        {
            // Log error of unexpected interrupt
        }
    }
}


/**
 * IR Sensor is attached to P1.18 - CAP1.0, so it needs TIMER1 to capture the times on P1.18
 */
bool IR_Sensor::init()
{
    // Enable Timer1 that is used to capture CAP1.0 signal
    LPC_SC->PCONP |= (1 << 1);      // Enable Power to Timer1
    LPC_SC->PCLKSEL0 &= ~(3 << 4);  // Clear Clock divider for Timer1
    LPC_SC->PCLKSEL0 |=  (1 << 4);  // Timer1 clock = CPU Clock / 1

    LPC_TIM1->TCR = 1;              // Enable Timer
    LPC_TIM1->CTCR = 0;             // Increment on PCLK
    LPC_TIM1->PR = getCpuClock() / (1000*1000/TIMER1_US_PER_TICK);

    LPC_TIM1->CCR &= ~(7 << 0);            // Clear Bits 2:1:0
    LPC_TIM1->CCR |=  (1 << 2) | (1 << 1); // Enable Falling Edge capture0 with interrupt

    // Enable MR0 interrupt
    LPC_TIM1->MR0 = 0;
    LPC_TIM1->MCR |= (1 << 0);

    // Select P1.18 as CAP1.0 by setting bits 5:4 to 0b11
    LPC_PINCON->PINSEL3 |= (3 << 4);

    // Finally, enable interrupt of Timer1 to interrupt upon falling edge capture
    NVIC_EnableIRQ(TIMER1_IRQn);

    return true;
}
bool IR_Sensor::isIRCodeReceived()
{
    return (0 != LAST_DECODED_IR_SIGNAL);
}
unsigned int IR_Sensor::getLastIRCode()
{
    unsigned int signal = LAST_DECODED_IR_SIGNAL;
    LAST_DECODED_IR_SIGNAL = 0;
    return signal;
}



bool LED_Display::init()
{
    bool devicePresent = checkDeviceResponse();
    if(devicePresent)
    {
        const unsigned char cfgAsOutput = 0x00;
        writeReg(cfgPort0, cfgAsOutput);
        writeReg(cfgPort1, cfgAsOutput);

        setLeftDigit('.');
        setRightDigit('.');
    }

    return devicePresent;
}
void LED_Display::clear()
{
    setLeftDigit(' ');
    setRightDigit(' ');
}
void LED_Display::setNumber(char num)
{
    num %= 100;
    setRightDigit(num%10 + '0');

    if(num < 10)
        setLeftDigit(' ');
    else
        setLeftDigit (num/10 + '0');
}
void LED_Display::setLeftDigit(char alpha)
{
    mNumAtDisplay[0] = alpha;
    writeReg(outputPort1, LED_DISPLAY_CHARMAP[(unsigned)alpha]);
}
void LED_Display::setRightDigit(char alpha)
{
    mNumAtDisplay[1] = alpha;
    writeReg(outputPort0, LED_DISPLAY_CHARMAP[(unsigned)alpha]);
}



/**
 * LED Mapping:
 *      1 : P1.0
 *      2 : P1.1
 *      3 : P1.4
 *      4 : P1.8
 *      5 : P1.9
 *      6 : P1.10
 *      7 : P1.14
 *      8 : P1.15
 */
bool LED::init()
{
    LPC_GPIO1->FIODIR |= mLedPinsBitMask;
    return true;
}
void LED::on(int ledNum)
{
    mLedValue |= (1 << (ledNum-1));
    setAll(mLedValue);
}
void LED::off(int ledNum)
{
    mLedValue &= ~(1 << (ledNum-1));
    setAll(mLedValue);
}
void LED::setAll(char value)
{
    mLedValue = value;
    LPC_GPIO1->FIOCLR = mLedPinsBitMask;

    if(value & (1 << 0))    LPC_GPIO1->FIOSET = (1 << 0);
    if(value & (1 << 1))    LPC_GPIO1->FIOSET = (1 << 1);
    if(value & (1 << 2))    LPC_GPIO1->FIOSET = (1 << 4);
    if(value & (1 << 3))    LPC_GPIO1->FIOSET = (1 << 8);
    if(value & (1 << 4))    LPC_GPIO1->FIOSET = (1 << 9);
    if(value & (1 << 5))    LPC_GPIO1->FIOSET = (1 << 10);
    if(value & (1 << 6))    LPC_GPIO1->FIOSET = (1 << 14);
    if(value & (1 << 7))    LPC_GPIO1->FIOSET = (1 << 15);
}



bool Light_Sensor::init()
{
    // Light Sensor is on P0.25, select this as ADC0.2
    LPC_PINCON->PINSEL1 &= ~(3 << 18);
    LPC_PINCON->PINSEL1 |=  (1 << 18);

    adc0_initialize();

    return true;
}
unsigned short Light_Sensor::getRawValue()
{
    return adc0_getReading(mAdcChannelOfSensor);
}
unsigned short Light_Sensor::getPercentValue()
{
    const unsigned int maxAdcValue = 4096;
    const unsigned int rawValue = adc0_getReading(mAdcChannelOfSensor);
    return (rawValue * 100) / maxAdcValue;
}



/**
 * Switch Mapping:
 * P2.0 : P2.1 : P2.2 : P2.3 : P2.4 : P2.5 : P2.6 : P2.7
 */
bool Switches::init()
{
    LPC_GPIO2->FIODIR0 = 0x00;
    return true;
}
unsigned char Switches::getSwitchValues()
{
    return LPC_GPIO2->FIOPIN0;
}
bool Switches::getSwitch(int num)
{
    return getSwitchValues() & (1 << (num-1));
}



bool TemperatureSensor::init()
{
    const unsigned char temperatureCfgRegPtr = 0x01;
    const unsigned char oneShotShutdownMode = 0x81;
    const unsigned char expectedBitsThatAreNotZero = 0x60; // R1:R0 bits are 1s at startup

    // Enter one-shot shutdown mode.
    writeReg(temperatureCfgRegPtr, oneShotShutdownMode);

    const unsigned char cfgRegByte0 = readReg(temperatureCfgRegPtr);
    return (0 != (cfgRegByte0 & expectedBitsThatAreNotZero));
}
float TemperatureSensor::getCelsius()
{
    // Get signed 16-bit data of temperature register pointer
    const unsigned char temperatureRegsiterPtr = 0x00;
    signed short temperature = get16BitRegister(temperatureRegsiterPtr);

    // Trigger next conversion:
    const unsigned char temperatureCfgRegPtr = 0x01;
    const unsigned char oneShotShutdownMode = 0x81;
    writeReg(temperatureCfgRegPtr, oneShotShutdownMode);

    // Temperature data is in bits 15:3 which contains signed 16-bit data
    temperature /= 16;

    // Each bit is of 0.0625 degree per bit resolution
    return (0.0625F * temperature);
}

float TemperatureSensor::getFarenheit()
{
    return (getCelsius() * 9.0F / 5) + 32;
}

