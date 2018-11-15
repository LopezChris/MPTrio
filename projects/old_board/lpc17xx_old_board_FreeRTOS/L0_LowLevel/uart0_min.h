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
 * @brief This file provides minimal UART0 implementation
 * 			The getchar and putchar functions provide polling version of data I/O
 */
#ifndef UART0_MIN_H_
#define UART0_MIN_H_
#ifdef __cplusplus
extern "C" {
#endif



/**
 * Initializes UART0 of LPC17xx
 * @param baudRate			The baud rate to set
 * @param peripheralClock 	The peripheral clock of the system which dictates the baud rate dividers
 */
void uart0Init(unsigned int baudRate, unsigned int peripheralClock);

/**
 * @param unused Unused parameter to comply with the printfFuncPtr @see printfFuncPtr
 * @returns character received over UART0
 */
char uart0Getchar(char unused);

/**
 * @param out	The character to output over UART0
 * @returns		Zero to comply with the printfFuncPtr @see printfFuncPtr
 */
char uart0Putchar(char out);

/**
 * outputs a string using uart0Putchar()
 * @param pStr	The pointer to null terminated string
 */
void uart0Puts(const char* pStr);



#ifdef __cplusplus
}
#endif
#endif /* UART0_MIN_H_ */
