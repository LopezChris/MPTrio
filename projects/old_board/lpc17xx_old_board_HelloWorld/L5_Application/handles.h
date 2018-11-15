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
 * @brief This file should contain all FreeRTOS Handles and data
 *          that shall be shared across all FreeRTOS tasks.
 */
#ifndef HANDLES_H_
#define HANDLES_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"


/**
 * Structure that holds FreeRTOS Queues, Tasks, and Semaphore Handles
 * Do not instantiate this struct, go through getHandles() to get
 * single instance of this structure.
 */
typedef struct
{
    struct {
        xSemaphoreHandle spi;
        xSemaphoreHandle learnIrCodes;
    }Sem;
}HandlesType;

/**
 * Get the instance of System Handles Structure
 */
const HandlesType* getHandles();



#ifdef __cplusplus
}
#endif
#endif /* HANDLES_H_ */
