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
 * @brief	This file provides:
 *              - API to connect stdio printf/scanf to your I/O functions
 *
 * Version: 06192012    Initial
 */

#ifndef IO_FUNCS_H__
#define IO_FUNCS_H__
#ifdef __cplusplus
extern "C" {
#endif

/// Function pointer of a function returning a char and taking a char as parameter
typedef char(*ioFuncPtr)(char);


/**
 * Sets the function used to output a char by printf() or stdio output functions
 * @param func	The function pointer to use to output a char
 */
void stdio_SetOutputCharFunction(ioFuncPtr func);

/**
 * Sets the function used to input a char by scanf() or stdio input functions
 * @param func	The function pointer to use to get a char
 */
void stdio_SetInputCharFunction(ioFuncPtr func);



#ifdef __cplusplus
}
#endif
#endif /* IO_FUNCS_H__ */
