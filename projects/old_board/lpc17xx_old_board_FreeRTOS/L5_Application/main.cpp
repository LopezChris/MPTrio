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
 * @brief This is the application entry point.
 * 			FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 * 			@see L0_LowLevel/io_functions.h if you wish to override printf/scanf functions.
 *
 * @note  printf of %f is turned off to save memory, this can be configured at sys_config.h
 */
#include "tasks.hpp"
#include "examples/examples.hpp"



/**
 * The main() creates tasks or "threads".  See the documentation of scheduler_task class at cpp_task.hpp
 * for details.  There is a very simple example towards the beginning of this class declaration.
 *
 * @warning SPI bus usage notes :
 *      - You can read/write files from multiple tasks because it automatically goes through SPI semaphore.
 *      - If you are going to use the SPI Bus in a FreeRTOS task, you need to use the API at L4_IO/fat/spi_sem.h
 */
int main(void)
{
    /**
     * Terminal task provides gateway to interact with the board.
     * Remote task allows you to use remote control to interact with the board.
     * Disable remote task if you are not using it.  Also, it needs ENABLE_TELEMETRY
     * such that it can save remote control codes to non-volatile memory.
     */
    scheduler_add_task(new terminalTask());
    scheduler_add_task(new remoteTask());

    /**
     * Change "#if 0" to "#if 1" to enable examples.
     * Study the example code to build your application.
     *
     * Try one example at a time, but try queue_tx() and queue_rx() together
     * since they send/receive data from each other.
     */
    #if 0
    scheduler_add_task(new example_task());
    scheduler_add_task(new example_alarm());
    scheduler_add_task(new example_logger_qset());

    scheduler_add_task(new queue_tx());
    scheduler_add_task(new queue_rx());
    #endif

    scheduler_start(true);
    return -1;
}
