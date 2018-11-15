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
 * @file utilities.h
 * @ingroup Utilities
 *
 * 04172012 : Added dprintf()
 */
#ifndef UTILITIES_H__
#define UTILITIES_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"



/**
 * Delays in microseconds
 * @param delayMicroSec The delay in microseconds
 */
void delay_us(unsigned int delayMicroSec);

/**
 * Delays in milliseconds
 * @param delayMilliSec The delay in milliseconds.
 */
void delay_ms(unsigned int delayMilliSec);

/**
 * @returns non-zero result if FreeRTOS is running
 */
char isFreeRtosRunning();

/**
 * Prints memory information to stdio
 * The buffer needs to be at least 128 bytes
 */
void printMemoryInfo(char* pBuff);

/**
 * Logs CPU boot information to the Storage
 * "boot.csv" is appended with the time this function is called
 */
void logBootInfo(const char*);


/**
 * Macro that can be used to print the timing/performance of a block
 * Example:
 * @code
 *      PRINT_EXECUTION_SPEED()
 *      {
 *          // ...
 *      }
 *      // At the end, the time taken between this block will be printed
 * @endcode
 */
#define PRINT_EXECUTION_SPEED() for(unsigned int __time=xTaskGetMsCount(); __time!=0; \
                                    printf("   Finished in %u ms\n", (unsigned int)xTaskGetMsCount()-__time),__time=0)



#ifdef __cplusplus
}
#endif
#endif /* UTILITIES_H__ */
