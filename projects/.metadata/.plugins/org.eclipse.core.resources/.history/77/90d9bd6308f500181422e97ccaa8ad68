#ifndef TESTLCD_HPP
#define TESTLCD_HPP

#include "tasks.hpp"
#include "utilities.h"
#include "io.hpp" //LD
#include "math.h"//floor
#include "L4_IO/gpio.hpp"//buttons
#include <mp3Test/NOKIA5110.hpp>
#include "L2_Drivers/lpc_pwm.hpp"
#include "uart0_min.h"
#include "printf_lib.h"

/**
 * Simple test for passing strings
 * to a shared queue which will print
 * whatever is passed to it.
 * 
 * This is a good test for when reading from the SD card
 * and just pass the song name from metadata
 */


class postMessage : public scheduler_task{
    public:

    typedef enum{
        sharedLCDQueueID
    }sharedHandleId_t;

    postMessage();
    bool run(void *p);
};




#endif // TESTLCD_HPP