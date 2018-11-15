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
 * @brief API at this file can be used to get Memory Information of the System
 */
#ifndef MEMORY_H_
#define MEMORY_H_
#ifdef __cplusplus
extern "C" {
#endif



/**
 * This is the memory structure that is returned from getMemoryInfo()
 * Heap memory obtains pool of memory from System, so the memory pool
 * not obtained by the Heap is listed by the systemAvailable variable.
 */
typedef struct
{
    unsigned globalUsed;        ///< Global Memory allocated
    unsigned heapUsed;          ///< Memory granted by Heap (malloc, new etc.)
    unsigned heapAvailable;     ///< Memory available at Heap
    unsigned systemAvailable;   ///< Memory available to Heap
}MemoryInfoType;

/**
 * Gets System Memory information
 * The information includes Global Memory usage, and dynamic memory usage.
 * @returns MemoryInfoType structure
 */
MemoryInfoType getMemoryInfo();



#ifdef __cplusplus
}
#endif
#endif /* MEMORY_H_ */
