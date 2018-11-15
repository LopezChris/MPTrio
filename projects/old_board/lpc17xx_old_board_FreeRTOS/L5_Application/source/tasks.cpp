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

#include <stdio.h>      // printf
#include <string.h>     // memset() etc.
#include "FreeRTOS.h"
#include "semphr.h"

#include "io_functions.h"       // stdio set IO functions
#include "uart0.hpp"            // Interrupt driven UART0 driver
#include "uart3.hpp"

#include "utilities.h"          // PRINT_EXECUTION_SPEED()
#include "handlers.hpp"         // Command-line handlers

#include "file_logger.hpp"      // Logging
#include "io.hpp"               // For Board IO
#include "watchdog.h"           // For watchdog reboot
#include "tasks.hpp"
#include "c_tlm_var.h"
#include "c_tlm_comp.h"
#include "c_tlm_stream.h"
#include "c_tlm_binary.h"

/**
 * Command type structure.
 * This structure is returned by getCommand() such that the calling function not
 * only knows about the command, but also which interface it came from.
 * For example, maybe UART0 and UART2 can both send commands, so knowing where
 * the command came from (UartIface), we can reply back appropriately.
 */
typedef struct {
    UartDev *out;   ///< Output interface that should be used for command's response
    str *cmdStr;    ///< Pointer to the command's string.
}cmd_type;

/**
 * Gets a line of input with backspace support and stores into str s
 * @param maxLen Maximum chars to store into s before leaving this function.
 */
cmd_type getCommand(void);

terminalTask::terminalTask() :
        scheduler_task("terminal", 1024*4, PRIORITY_HIGH),
        mCommandCount(0), mDiskTlmSize(0), mpBinaryDiskTlm(NULL),
        mCmdProc(18)
{
    /* Nothing to do */
}

bool terminalTask::regTlm(void)
{
    #if ENABLE_TELEMETRY
    return (TLM_REG_VAR(tlm_component_get_by_name("debug"), mCommandCount, tlm_uint) &&
            TLM_REG_VAR(tlm_component_get_by_name("debug"), mDiskTlmSize, tlm_uint));
    #endif
    return true;
}

bool terminalTask::taskEntry()
{
    /* remoteTask() creates shared object in its init(), so we can get it now */
    xSemaphoreHandle learn_sem = getSharedObject("learn_sem");
    CommandProcessor &cp = mCmdProc;

    // System information handlers
    cp.addHandler(taskListHandler, "info",    "Task/CPU Info.  Use 'info 200' to get CPU during 200ms");
    cp.addHandler(memInfoHandler,  "meminfo", "See memory info");
    cp.addHandler(healthHandler,   "health",  "Output system health");
    cp.addHandler(timeHandler,     "time",    "'time' to view time.  'time set MM DD YYYY HH MM SS Wday' to set time");

    // File I/O handlers:
    cp.addHandler(catHandler, "cat",    "Read a file.  Ex: 'cat 0:file.txt' or "
                                        "'cat 0:file.txt -noprint' to test if file can be read");
    cp.addHandler(cpHandler,   "cp",    "Copy files from/to Flash/SD Card.  Ex: 'cp 0:file.txt 1:file.txt'");
    cp.addHandler(dcpHandler,  "dcp",   "Copy all files of a directory to another directory.  Ex: 'dcp 0:src 1:dst'");
    cp.addHandler(lsHandler,   "ls",    "Use 'ls 0:' for Flash, or 'ls 1:' for SD Card");
    cp.addHandler(mkdirHandler,"mkdir", "Create a directory. Ex: 'mkdir test'");
    cp.addHandler(mvHandler,   "mv",    "Rename a file. Ex: 'rm 0:file.txt 0:new.txt'");
    cp.addHandler(rmHandler,   "rm",    "Remove a file. Ex: 'rm 0:file.txt'");

    // Misc. handlers
    cp.addHandler(storageHandler, "storage", "Parameters: 'format sd', 'format flash', 'mount sd', 'mount flash'");
    cp.addHandler(rebootHandler,  "reboot",  "Reboots the system");
    cp.addHandler(logHandler,     "log",     "'log <hello>' to log info.  'log flush' to flush logs");
    cp.addHandler(learnIrHandler, "learn",   "Begin to learn IR codes for numbers 0-9", learn_sem, sizeof(learn_sem));

    #if (ENABLE_TELEMETRY)
    cp.addHandler(telemetryHandler, "telemetry", "Outputs registered telemetry. "
                                                 "'telemetry save' : Saves disk tel. "
                                                 "'telemetry <comp. name> <name> <value>' to set a telemetry variable");
    #endif

    // Initialize Interrupt driven version of getchar & putchar
    Uart0& uart0 = Uart0::getInstance();
    bool success = uart0.init(38400, 32, 512);
    stdio_SetInputCharFunction(uart0.getcharIntrDriven);
    stdio_SetOutputCharFunction(uart0.putcharIntrDriven);

    #if ENABLE_TELEMETRY
    /* Telemetry should be registered at this point, so initialize the binary
     * telemetry space that we periodically check to save data to disk
     */
    tlm_component *disk = tlm_component_get_by_name(DISK_TLM_NAME);
    mDiskTlmSize = tlm_binary_get_size_one(disk);
    mpBinaryDiskTlm = new char[mDiskTlmSize];
    if (success) {
        success = (NULL != mpBinaryDiskTlm);
    }

    /* Now update our copy of disk telemetry */
    tlm_binary_get_one(disk, mpBinaryDiskTlm);
    #endif

    STR_ON_STACK(help, 8);
    help = "help";
    mCmdProc.handleCommand(help, uart0);

    return success;
}

bool terminalTask::run(void* p)
{
    printf("LPC: ");

    // If no command (idle), then use this time to save disk telemetry
    cmd_type cmd = getCommand();
    if (!cmd.out) {
        puts("");
        return saveDiskTlm();
    }

    // If the user did not press enter key, getLen() will be greater than 0
    if (cmd.cmdStr->getLen() > 0)
    {
        PRINT_EXECUTION_SPEED()
        {
            ++mCommandCount;
            mCmdProc.handleCommand(*(cmd.cmdStr), *(cmd.out));
        }
    }

    return true;
}

bool terminalTask::saveDiskTlm(void)
{
    #if ENABLE_TELEMETRY
    tlm_component *disk = tlm_component_get_by_name(DISK_TLM_NAME);

    /* Size of telemetry shouldn't change */
    if (mDiskTlmSize != tlm_binary_get_size_one(disk)) {
        return false;
    }

    if (!tlm_binary_compare_one(disk, mpBinaryDiskTlm)) {
        puts("Disk variables changed...");

        FILE *file = fopen(DISK_TLM_NAME, "w");
        if (file) {
            // Only update variables if we could open the file
            tlm_binary_get_one(disk, mpBinaryDiskTlm);

            tlm_stream_one_file(disk, file);
            fclose(file);

            puts("Changes saved to disk...");
            LOG_INFO_SIMPLE("Disk variables saved to disk");
        } else {
            return false;
        }
    }
    #endif

    return true;
}



remoteTask::remoteTask() :
        scheduler_task("remote", 512*3, PRIORITY_HIGH),
        mIrNumber(0)
{
    memset(mNumCodes, 0, sizeof(mNumCodes));
}

bool remoteTask::init(void)
{
    setFrequency(100);

    vSemaphoreCreateBinary(mLearnSem);
    xSemaphoreTake(mLearnSem, 0);

    return addSharedObject("learn_sem", mLearnSem);
}

bool remoteTask::regTlm(void)
{
    bool success = true;
    #if ENABLE_TELEMETRY
        tlm_component *disk = tlm_component_get_by_name(DISK_TLM_NAME);
        if(success) {
            success = TLM_REG_ARR(disk, mNumCodes, tlm_uint);
        }
    #endif
    return success;
}
bool remoteTask::taskEntry(void)
{
    LD.clear();
    return true;
}
bool remoteTask::run(void *p)
{
    uint32_t number = 0;
    STR_ON_STACK(temp, 64);

    if(xSemaphoreTake(mLearnSem, 0))
    {
        puts("IR Codes will be learned.  Press buttons 0-9 on the remote");
        LD.setLeftDigit('-');
        LD.setRightDigit('-');

        for(int i=0; i < 10; i++)
        {
            while(!IS.isIRCodeReceived()) {
                vTaskDelayMs(100);
            }

            unsigned int code = IS.getLastIRCode();
            temp.printf("Learned: #%i = %x", i, code);
            puts(temp());

            mNumCodes[i] = code;
            LD.setNumber(i);
        }

        puts("Learned all numbers!");
        vTaskDelayMs(2000);
    }

    /**
     * If the timer is running, we are expecting 2nd digit to be entered through IR code.
     * If the timeout occurs, we clear the LED display and throw away the IR numbers.
     */
    if (mIrNumTimer.isRunning()) {
        if(IS.isIRCodeReceived() &&
           getNumberFromCode(IS.getLastIRCode(), number))
        {
            mIrNumber += number;
            LD.setRightDigit(number + '0');

            handleIrNumber(number);
            vTaskDelayMs(2000);

            // Discard if any code came in within the delay above
            (void) IS.getLastIRCode();
            mIrNumTimer.stop();
        }
        else if (mIrNumTimer.expired()) {
            mIrNumTimer.stop();
            LD.clear();
        }
    }
    else {
        /**
         * If we got an IR code, we store the left digit, and start the timer to expect
         * the 2nd IR code to be entered.
         */
        if(IS.isIRCodeReceived() && getNumberFromCode(IS.getLastIRCode(), number)) {
            LD.setLeftDigit(number + '0');
            LD.setRightDigit('-');

            mIrNumber = 10 * number; // Left digit

            // If 2nd digit not entered soon enough, go back to this state.
            mIrNumTimer.reset(10 * 1000);
        }
        else {
            /* User can control LED display here if no IR code is being decoded */
        }
    }

    return true;
}

void remoteTask::handleIrNumber(int num)
{
    /* Handle the IR number here for your project */
}

bool remoteTask::getNumberFromCode(uint32_t code, uint32_t& num)
{
    for(int i=0; i<10; i++)
    {
        if(mNumCodes[i] == code)
        {
            num = i;
            return true;
        }
    }

    return false;
}


void handleEchoAndBackspace(str& cmd, char newChar)
{
    /* Echo & backspace logic over stdio */
    // Backspace 1 char @ terminal and erase last char of string
    if ('\b' == newChar) {
        if (cmd.getLen() > 0) {
            putchar('\b');
            putchar(' ');
            cmd.eraseLast(1);
        }
        else {
            // Send "Alert" sound to terminal because we can't backspace
            const char bellSound = '\a';
            newChar =  bellSound;
        }
    }
    else if ('\n' != newChar && '\r' != newChar) {
        cmd += newChar;
    }
    putchar(newChar);
}

cmd_type getCommand(void)
{
    Uart0& u0 = Uart0::getInstance();
    const int maxLen = 128;
    static str uart0_cmd;
    char c = 0;

    /* Initialize command type assuming our command came from
     * uart0 and from uart0_command str
     */
    cmd_type ret;
    ret.cmdStr = &uart0_cmd;
    ret.out = &u0;

    uart0_cmd.clear();
    do
    {
        /* If no command for 60 secs, return NULL back to give the task a chance
         * to do any misc. activities.
         */
        if (u0.getChar(&c, 1000)) {
            ret.out = &u0;
            ret.cmdStr = &uart0_cmd;
            handleEchoAndBackspace(uart0_cmd, c);
        }
        else if (!u0.recentlyActive(60*1000)) {
            u0.resetActivity();
            ret.out = NULL;
            break;
        }

        /**
         * Guard against command length too large
         */
        if (uart0_cmd.getLen() >= maxLen) {
            break;
        }
    } while (c != '\n');

    return ret;
}
