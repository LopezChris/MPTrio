#include "uart_iface.hpp"
#include "sys_config.h"
#include <stdio.h>
#include <stdarg.h>


bool UartIface::put(const char* pString, unsigned int timeout)
{
    while(*pString) {
        if(!putChar(*pString++, timeout)) {
            return false;
        }
    }

    return true;
}

void UartIface::puts(const char* pBuff, unsigned int timeout)
{
    put(pBuff, timeout);
    put("\r\n", timeout);
}

bool UartIface::gets(char* pBuff, int maxLen, unsigned int timeout)
{
    char c = 0;
    int charCount = 0;
    bool success = false;

    while(getChar(&c, timeout)) {
        if ('\r' != c && '\n' != c) {
            *pBuff++ = c;
        }
        if(++charCount >= maxLen) {
            break;
        }
        if('\n' == c) {
            success = true;
            break;
        }
    }

    // If we didn't get any characters, don't null terminate the string
    if(charCount > 0) {
        *pBuff = '\0';
    }

    return success;
}

int UartIface::printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buff[256] = { 0 };

    /**
     * Try to print into allocated memory, and if the number of characters
     * is greater than capacity, reallocate more memory, and print again ???
     */
#if USE_REDUCED_PRINTF == 0
    int len = vsnprintf(buff, sizeof(buff)-1, format, args);
#else
    int len = vsniprintf(buff, sizeof(buff)-1, format, args);
#endif

    /**
     * output is only written if len is greater than 0 and less
     * than our capacity.  So if it is not written then retry ???
     */
    if(! (len > 0 && len <= (int)sizeof(buff)))
    {
        /* Notify anyone that we can't print ? */
    }
    else {
        this->put(buff);
    }

    va_end(args);
    return len;
}

int UartIface::scanf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int parsed = 0;
    char buff[256] = { 0 };

    if (this->gets(buff, sizeof(buff))) {
        #if USE_REDUCED_PRINTF == 0
        parsed = vsscanf(buff, format, args);
        #else
        parsed = vsiscanf(buff, format, args);
        #endif
    }
    va_end(args);
    return parsed;
}
