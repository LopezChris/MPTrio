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

#include <stdio.h>              // printf()
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"               // vTaskList()

#include "command_handler.hpp"  // CMD_HANDLER_FUNC()
#include "rtc.h"                // Set and Get System Time
#include "boot_type.h"          // getBootTime();

#include "utilities.h"          // printMemoryInfo()
#include "storage.hpp"          // Get Storage Device instances
#include "file_logger.hpp"      // Logger class
#include "watchdog.h"           // rebootHandler()

#include "uart0.hpp"
#include "io.hpp"
#include "c_tlm_stream.h"
#include "c_tlm_var.h"



CMD_HANDLER_FUNC(taskListHandler)
{
    const int delayInMs = (int)cmdParams;  // cast parameter str to integer

    if(delayInMs > 0) {
        vTaskResetRunTimeStats();
        vTaskDelayMs(delayInMs);
    }

    // Warning: We need roughly 50 chars of space per task
    signed char buffer[512] = { 0 };
    vTaskList(buffer);

    output.putline("   Name     S Pr  Stack  # CPU  Ticks");
    output.putline((char*)buffer);
}

CMD_HANDLER_FUNC(memInfoHandler)
{
    char buffer[512] = { 0 };
    printMemoryInfo(buffer);
    output.putline(buffer);
}

CMD_HANDLER_FUNC(healthHandler)
{
    Uart0 &u0 = Uart0::getInstance();

    unsigned int total = 0;
    unsigned int available = 0;
    Storage::getFlashDrive().getDriveInfo(&total, &available);

    float floatTemp = TS.getFarenheit();
    int floatSig1 = (int) floatTemp;
    int floatDec1 = ((floatTemp - floatSig1) * 10);
    RTC bt = getBootTime();

    char dateTime[24] = { 0 };
    rtc_getDateTimeString(dateTime);

    output.printf("Flash: %u/%u\n"
                    "Temp : %u.%u\n"
                    "Light: %u\n"
                    "Time     : %s\n"
                    "Boot Time: %u/%u/%u,%02u:%02u:%02u\n"
                    "Uart0 Watermarks: %u/%u (rx/tx)\n",
                    available, total,
                    floatSig1, floatDec1,
                    LS.getRawValue(),
                    dateTime,
                    bt.month, bt.day, bt.year, bt.hour, bt.min, bt.sec,
                    u0.getRxQueueWatermark(), u0.getTxQueueWatermark()
    );
}

CMD_HANDLER_FUNC(timeHandler)
{
    RTC time;

    /**
     * If cmdParam contains "set" with six spaces, we can parse the time
     * Example: set 11 30 2014 8 25 0 1
     */
    if(cmdParams.beginsWith("set"))
    {
        if(cmdParams.countOf(" ") != 7) {
            output.putline("Need time in terms of MM DD YYYY HH MM SS Wday");
            return;
        }

        unsigned m, d, y, hr, mn, sc, w;
        if( 7 != cmdParams.scanf("%*s %u %u %u %u %u %u %u", &m, &d, &y, &hr, &mn, &sc, &w))
        {
            output.putline("Parse failure");
            return;
        }

        time.month = m; time.day = d; time.year = y;
        time.hour = hr, time.min = mn; time.sec = sc;
        time.dow = w;
        rtc_settime(&time);
    }

    time = rtc_gettime();
    output.printf("%02u/%02u/%u  --  %02u:%02u:%02u (%u)\n",
                    time.month, time.day, time.year,
                    time.hour, time.min, time.sec, time.dow);
}

CMD_HANDLER_FUNC(logHandler)
{
    if (cmdParams == "flush") {
        FileLogger::getInstance().flush();
        output.putline("Log(s) have been flushed");
    }
    else {
        LOG_INFO(cmdParams());
        output.printf("Logged: |%s|\n", cmdParams());
    }
}

CMD_HANDLER_FUNC(cpHandler)
{
    char *srcFile = NULL;
    char *dstFile = NULL;
    if(2 != cmdParams.tokenize(" ", 2, &srcFile, &dstFile)) {
        output.putline("Error, Try: 'cp <src file name> <dst file name>'");
        return;
    }

    unsigned int readTimeMs = 0;
    unsigned int writeTimeMs = 0;
    unsigned int bytesTransferred = 0;
    FRESULT copyStatus = Storage::copy(srcFile, dstFile,
                                       &readTimeMs, &writeTimeMs, &bytesTransferred);

    if(FR_OK != copyStatus) {
        output.printf("Error %u copying |%s| -> |%s|\n", copyStatus, srcFile, dstFile);
    }
    else {
        output.printf("Finished!  Read: %u Kb/sec, Write: %u Kb/sec\n",
                      bytesTransferred/(0 == readTimeMs  ? 1 : readTimeMs),
                      bytesTransferred/(0 == writeTimeMs ? 1 : writeTimeMs));
    }
}

CMD_HANDLER_FUNC(catHandler)
{
    // If -print was present, we will print to console
    bool printToScreen = !cmdParams.erase("-noprint");
    cmdParams.trimStart(" ");
    cmdParams.trimEnd(" ");

    FIL file;
    if(FR_OK != f_open(&file, cmdParams(), FA_OPEN_EXISTING | FA_READ))
    {
        output.printf("Failed to open: %s\n", cmdParams());
    }
    else
    {
        // Extra char for null terminator
        char buffer[1024 + 1] = { 0 };
        unsigned int bytesRead = 0;
        unsigned int totalBytesRead = 0;

        unsigned int startTime = xTaskGetMsCount();
        while(FR_OK == f_read(&file, buffer, sizeof(buffer)-1, &bytesRead) && bytesRead > 0)
        {
            totalBytesRead += bytesRead;
            buffer[bytesRead] = '\0';

            if(printToScreen) {
                output.put(buffer);
            }
        }
        f_close(&file);

        if(!printToScreen) {
            unsigned int timeTaken = xTaskGetMsCount() - startTime;
            // Divide by zero check
            if(0 == timeTaken) {
                timeTaken = 1;
            }
            output.printf("Read %u bytes @ %u Kb/sec", totalBytesRead, totalBytesRead/timeTaken);
        }
        output.putline("");
    }
}

CMD_HANDLER_FUNC(lsHandler)
{
    DIR Dir;
    FILINFO Finfo;
    FATFS *fs;
    FRESULT returnCode = FR_OK;

    unsigned int fileBytesTotal, numFiles, numDirs;
    fileBytesTotal = numFiles = numDirs = 0;
    #if _USE_LFN
        char Lfname[_MAX_LFN];
    #endif

    char *dirPath = NULL;
    char *lsOffsetStr = NULL;
    int lsOffset = 0;
    if (2 == cmdParams.tokenize(" ", 2, &dirPath, &lsOffsetStr)) {
        lsOffset = atoi(lsOffsetStr);
    }

    if (NULL == dirPath) {
        dirPath = (char*)"0:";
    }

    if (FR_OK != (returnCode = f_opendir(&Dir, dirPath))) {
        output.printf("Invalid directory: |%s|\n", dirPath);
        return;
    }

    // Offset the listing
    while(lsOffset-- > 0) {
        #if _USE_LFN
            Finfo.lfname = Lfname;
            Finfo.lfsize = sizeof(Lfname);
        #endif
        if (FR_OK != f_readdir(&Dir, &Finfo)) {
            break;
        }
    }

    output.printf("Directory listing of: %s\n\n", dirPath);
    for (;;)
    {
        #if _USE_LFN
            Finfo.lfname = Lfname;
            Finfo.lfsize = sizeof(Lfname);
        #endif

        char returnCode = f_readdir(&Dir, &Finfo);
        if ( (FR_OK != returnCode) || !Finfo.fname[0]) {
            break;
        }

        if (Finfo.fattrib & AM_DIR){
            numDirs++;
        }
        else{
            numFiles++;
            fileBytesTotal += Finfo.fsize;
        }
        output.printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %10lu %13s",
                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
                (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,
                Finfo.fsize, &(Finfo.fname[0]));

        // LFN names tend to increase memory requirements for output str, enable with caution
        #if (_USE_LFN)
        output.put(" - ");
        output.put(Lfname);
        #endif
        output.putline("");
    }
    output.printf("\n%4u File(s), %10u bytes total\n%4u Dir(s)", numFiles, fileBytesTotal, numDirs);

    if (f_getfree(dirPath, (DWORD*) &fileBytesTotal, &fs) == FR_OK)
    {
        output.printf(", %10uK bytes free\n", fileBytesTotal * fs->csize / 2);
    }
}

CMD_HANDLER_FUNC(mkdirHandler)
{
    output.printf("Create directory '%s' : %s\n",
                   cmdParams(), (FR_OK == f_mkdir(cmdParams())) ? "OK" : "ERROR");
}

CMD_HANDLER_FUNC(rmHandler)
{
    output.printf("Delete '%s' : %s\n",
                  cmdParams(), (FR_OK == f_unlink(cmdParams())) ? "OK" : "ERROR");
}

CMD_HANDLER_FUNC(mvHandler)
{
    char *srcFile = NULL;
    char *dstFile = NULL;
    if(2 != cmdParams.tokenize(" ", 2, &srcFile, &dstFile)) {
        output.putline("Error, Try: 'mv <src file name> <dst file name>'");
    }
    else {
        output.printf("Move '%s' -> '%s' : %s\n",
                      srcFile, dstFile,
                      (FR_OK == f_rename(srcFile, dstFile))  ? "OK" : "ERROR");
    }
}

CMD_HANDLER_FUNC(dcpHandler)
{
    DIR Dir;
    FILINFO Finfo;
    FRESULT returnCode = FR_OK;
    #if _USE_LFN
        char Lfname[_MAX_LFN];
    #endif

    char *srcDir = NULL;
    char *dstDir = NULL;
    if(2 != cmdParams.tokenize(" ", 2, &srcDir, &dstDir)) {
        output.putline("ERROR: Give me source and destination directory separated by a space");
        return;
    }

    /* Check destination directory */
    if (FR_OK != (returnCode = f_opendir(&Dir, dstDir))) {
        output.printf("Invalid destination directory: |%s|\n", dstDir);
        return;
    }
    /* Check source directory */
    if (FR_OK != (returnCode = f_opendir(&Dir, srcDir))) {
        output.printf("Invalid source directory: |%s|\n", srcDir);
        return;
    }

    STR_ON_STACK(src, 256);
    STR_ON_STACK(dst, 256);
    for (;;)
    {
        #if _USE_LFN
            Finfo.lfname = Lfname;
            Finfo.lfsize = sizeof(Lfname);
        #endif

        /* If no more files */
        if ( (FR_OK != f_readdir(&Dir, &Finfo)) || !Finfo.fname[0]) {
            break;
        }

        /* If not a directory */
        if (!(Finfo.fattrib & AM_DIR))
        {
            src.printf("%s/%s", srcDir, Finfo.fname);
            dst.printf("%s/%s", dstDir, Finfo.fname);

            output.printf("Copy %s -> %s : %u Bytes : %s\n",
                           src(), dst(), Finfo.fsize,
                           (FR_OK == Storage::copy(src(), dst())) ? "OK" : "ERROR");
        }
    }
}

CMD_HANDLER_FUNC(storageHandler)
{
    if(cmdParams == "format sd") {
        output.putline((FR_OK == Storage::getSDDrive().format()) ? "Format OK" : "Format ERROR");
    }
    else if(cmdParams == "format flash") {
        output.putline((FR_OK == Storage::getFlashDrive().format()) ? "Format OK" : "Format ERROR");
    }
    else if(cmdParams == "mount sd") {
        output.putline(Storage::getSDDrive().mount() ? "SD Card mounted" : "Error mounting SD Card");
    }
    else if(cmdParams == "mount flash") {
        output.putline(Storage::getFlashDrive().mount() ? "Flash mounted" : "Error mounting Flash Memory");
    }
    else {
        output.putline("Parameters are 'format sd', 'format flash', 'mount sd' and 'mount flash'");
    }
}

CMD_HANDLER_FUNC(rebootHandler)
{
    output.putline("Rebooting System");

    // Flush out everything we can
    FileLogger::getInstance().flush();

    vTaskDelayMs(2000);
    watchdog_RebootSystem();
}

#if (ENABLE_TELEMETRY)
CMD_HANDLER_FUNC(telemetryHandler)
{
    if(cmdParams.getLen() == 0) {
        tlm_stream_all_file(stdout);
    }
    else if(cmdParams == "save") {
        FILE *fd = fopen(DISK_TLM_NAME, "w");
        tlm_stream_one_file(tlm_component_get_by_name(DISK_TLM_NAME), fd);
        fclose(fd);
        output.putline("Telemetry was saved to disk");
    }
    else {
        char *compName = NULL;
        char *varName = NULL;
        char *varVal = NULL;
        if (3 != cmdParams.tokenize(" ", 3, &compName, &varName, &varVal)) {
            output.putline("ERROR: See 'help telemetry'");
        }
        else {
            if (tlm_variable_set_value(compName, varName, varVal)) {
                output.printf("%s:%s set to %s\n", compName, varName, varVal);
            } else {
                output.printf("Failed to set %s:%s to %s\n", compName, varName, varVal);
            }
        }
    }
}
#endif

CMD_HANDLER_FUNC(learnIrHandler)
{
    xSemaphoreHandle learn_sem = pDataParam;
    xSemaphoreGive(learn_sem);
    output.putline("Learning mode enabled");
}
