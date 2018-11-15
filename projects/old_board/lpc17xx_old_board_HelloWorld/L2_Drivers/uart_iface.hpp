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
 * @file  uart_iface.hpp
 * @brief C++ interface object for input/output
 */
#ifndef OUTPUT_IFACE_HPP__
#define OUTPUT_IFACE_HPP__



/**
 * Uart Interface
 *
 * Interfaces are helpful to dynamically point this pointer or reference to different objects.
 * Suppose you have two UARTs and you wish to use one or the other depending on some configuration:
 *
 *@code
 *  UART0& uart0 = UART0::getInstance();
 *  UART1& uart1 = UART1::getInstance();
 *  OutputIface& out = (some_condition_here) ? uart0 : uart1;
 *
 *  // Now we can use "out" and call functions like:
 *  out.put("Hello World.  I am the output interface!")
 *@endcode
 */
class UartIface
{
    public:
        /**
         * @returns a character from the input device
         * @param   pInputChar  The pointer to input char to store received character
         * @param   timeout Optional parameter which defaults to maximum value that
         *          will allow you to wait forever for a character to be received
         * @returns true if a character was obtained within the given timeout
         */
        virtual bool getChar(char* pInputChar, unsigned int timeout=0xffffffff) const = 0;

        /**
         * Outputs a char given by @param out
         * @param   timeout Optional parameter which defaults to maximum value that
         *          will allow you to wait forever for a character to be sent
         * @returns true if the output char was successfully written to Queue, or
         *          false if the output queue was full within the given timeout
         */
        virtual bool putChar(char out, unsigned int timeout=0xffffffff) = 0;

        /**
         * @{ Output a null-terminated string
         * puts() will also output a newline char at the end of the string
         */
        bool put(const char* pString, unsigned int timeout=0xffffffff);
        void puts(const char* pBuff, unsigned int timeout=0xffffffff);
        /** @} */

        /**
         * Get a string of input up to maxLen
         * @param pBuff The buffer to store data to
         * @param maxLen The maximum chars to get
         * @param timeout The timeout in ticks to wait
         */
        bool gets(char* pBuff, int maxLen, unsigned int timeout=0xffffffff);

        /**
         * Just like printf, except it will print to this output interface
         * @returns the number of characters printed or -1 if the internal buffer
         *          was too small to perform printf() of all characters
         */
        int printf(const char *format, ...);

        /**
         * Just like scanf, except this will perform scanf after receiving a line
         * of input using the gets() method.
         */
        int scanf(const char *format, ...);

        UartIface() {}
        virtual ~UartIface() {}
};




#endif /* OUTPUT_IFACE_HPP__ */
