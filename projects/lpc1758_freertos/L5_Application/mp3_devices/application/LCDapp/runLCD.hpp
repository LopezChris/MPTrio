#ifndef RUNLCD_HPP_
#define RUNLCD_HPP_

#include "tasks.hpp"
#include "utilities.h"
#include "io.hpp" //LD
#include "math.h"//floor
#include "L4_IO/gpio.hpp"//buttons
#include <mp3_devices/NOKIA5110.hpp>
#include "L2_Drivers/lpc_pwm.hpp"
#include "uart0_min.h"
#include "printf_lib.h"


/**
 * This is a simple test for writing to
 * LCD screen. To successfully test just have the SD
 * card pass a string as needed and the
 * simple app will just take the string convert to pixels
 * and set the appropriate pixels.
 *
 * TODO: Create roll-over functionality to
 * LCD driver so that if playlist > 6 items
 * the contents being read will return to the top
 * 
 * TODO: Limit the size of strings to 14 chars long; otherwise, the
 * letters are overwritten.
 */


class receiveMessage : public scheduler_task{
    public:

        enum{
            BLACK=1,
            WHITE=0
        };

        typedef enum{
            sharedLCDQueueID
        }sharedHandleId_t;

        receiveMessage();
        bool run(void *p);

    private:
        // Nothing for now
};



#endif
