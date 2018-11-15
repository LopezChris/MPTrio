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
 * @ingroup BoardIO
 */
#ifndef LPC_GPIO_H__
#define LPC_GPIO_H__

#include "LPC17xx.h"



/**
 * Enumeration of GPIO IDs
 * @warning Do not change the value(s) of the enumerations
 */
typedef enum {
    P1_19 = 19,
    P1_20 = 20,
    P1_22 = 22,
    P1_23 = 23,

    P1_24 = 24,
    P1_25 = 25,
    P1_26 = 26,
    P1_27 = 27,

    P1_30 = 30, /**< Labeled as AD0.4 */
    P1_31 = 31, /**< Labeled as AD0.5 */
} LPC1758_GPIO;



/**
 * GPIO class for SJ-One Board or any LPC17xx CPU
 * The pins on the SJ-One Board can be used as General Purpose Input/Output
 * pins which can be attached to for example and LED or a switch externally.
 *
 * Please see the pin labels on the board and then use this class to
 * manipulate the pin(s).  See the next examples :
 *
 * Output pin example :
 * @code
 *      GPIO myPin(P1_19);   // Control P1.19
 *      myPin.setAsOutput(); // Use the pin as output pin
 *      myPin.setHigh();     // Pin will now be at 3.3v
 *      myPin.setLow();      // Pin will now be at 0.0v
 * @endcode
 *
 * Input pin example :
 * @code
 *      GPIO myPin(P1_20);   // Control P1.20
 *      myPin.setAsInput();  // Use the pin as output pin
 *      bool value = myPin.read(); // Read value of the pin
 * @endcode
 */
class GPIO
{
    public:
        /// Constructor to choose the pin
        GPIO(LPC1758_GPIO gpioId) : mId(gpioId)
        {
            // Configure the PIN as GPIO
            LPC_PINCON->PINSEL3 &= ~(3 << ((mId - 16)*2) );
        }

        inline void setAsInput(void);     ///< Sets pin as input pin
        inline void setAsOutput(void);    ///< Sets pin as output pin

        inline bool read(void) const;     ///< Reads logical value of the pin
        inline void setHigh(void);        ///< Sets the pin to logical HIGH (3.3v)
        inline void setLow(void);         ///< Sets the pin to logical LOW (0.0v)
        inline void set(bool on);         ///< Set high/low based on boolean value

        inline void enablePullUp();          ///< Enables pull-up resistor
        inline void enablePullDown();        ///< Enables pull-down resistor
        inline void disablePullUpPullDown(); ///< Disables pull-up/down resistor
        inline void enableOpenDrainMode(bool openDrain); ///< Enables open drain mode

    protected:
    private:
        LPC1758_GPIO mId; ///< The pin ID
};



/* Code of the GPIO class : */
inline void GPIO::setAsInput(void)  { LPC_GPIO1->FIODIR &= ~(1 << mId); }
inline void GPIO::setAsOutput(void) { LPC_GPIO1->FIODIR |=  (1 << mId); }
inline bool GPIO::read(void) const  { return !!(LPC_GPIO1->FIOPIN & (1 << mId)); }
inline void GPIO::setHigh(void)     { LPC_GPIO1->FIOSET = (1 << mId); }
inline void GPIO::setLow(void)      { LPC_GPIO1->FIOCLR = (1 << mId); }
inline void GPIO::set(bool on)      { (on) ? setHigh() : setLow(); }

inline void GPIO::enablePullUp()   { LPC_PINCON->PINMODE3 &= ~(3 << ((mId - 16)*2) ); }
inline void GPIO::enablePullDown() { LPC_PINCON->PINMODE3 |=  (3 << ((mId - 16)*2) ); }
inline void GPIO::disablePullUpPullDown()
{
    enablePullUp();
    LPC_PINCON->PINMODE3 |= (2 << ((mId - 16)*2) );
}
inline void GPIO::enableOpenDrainMode(bool openDrain)
{
    if(openDrain)
        LPC_PINCON->PINMODE_OD1 |= (1 << mId);
    else
        LPC_PINCON->PINMODE_OD1 &= ~(1 << mId);
}


#endif /* GPIO_H__ */

