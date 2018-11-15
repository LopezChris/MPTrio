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
#include <stdlib.h>
#include "handles.h"

const HandlesType* getHandles()
{
    static HandlesType* handles = 0;

    if(0 == handles) {
        handles = (HandlesType*)malloc(sizeof(HandlesType));

        handles->Sem.spi = xSemaphoreCreateMutex();
        vSemaphoreCreateBinary(handles->Sem.learnIrCodes);
    }

    return handles;
}

#if 0
const char* getMMYYExtension(const char* pFilename)
{
    static char filename[16] = {0};

    // Short filenames have 12 character limit not including null
    char *p = &filename[0];
    for(int i=0; i<7 && pFilename[i] != '\0'; i++) {
        *p++ = pFilename[i];
    }

    RTC time = rtc_gettime();
    time.year -= 2000;

    //*p++ = '.';
    *p++ = (time.month / 10) + '0';
    *p++ = (time.month % 10) + '0';
    *p++ = (time.year / 10) + '0';
    *p++ = (time.year % 10) + '0';
    *p++ = '\0';

    return filename;
}
#endif
