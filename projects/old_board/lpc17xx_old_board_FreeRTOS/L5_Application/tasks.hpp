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
 * @brief Contains FreeRTOS Tasks
 */
#ifndef TASKS_HPP_
#define TASKS_HPP_
#include "cpp_task.hpp"
#include "command_handler.hpp"
#include "str.hpp"
#include "averager.hpp"
#include "memory.h"
#include "file_logger.hpp"
#include "soft_timer.hpp"



/**
 * Terminal task is our UART0 terminal that handles our commands into the board.
 * This also saves and restores the "disk" telemetry.  Disk telemetry variables
 * are automatically saved and restored across power-cycles to help us preserve
 * any non-volatile information.
 */
class terminalTask : public scheduler_task
{
public:
        terminalTask();
        bool regTlm(void);    ///< Registers telemetry
        bool taskEntry(void); ///< Registers commands.
        bool run(void *p);

private:
        bool saveDiskTlm(void);

        uint16_t mCommandCount;     ///< terminal command count
        uint16_t mDiskTlmSize;      ///< Size of disk variables in bytes
        char *mpBinaryDiskTlm;      ///< Binary disk telemetry
        CommandProcessor mCmdProc;  ///< Command processor
};

/**
 * Remote task is the task the monitors the IR remote control signals.
 * It can "learn" remote control codes by typing "learn" into the UART0 terminal.
 * Thereafter, if a user enters a 2-digit number through a remote control, then
 * your function handleUserEntry() is called where you can take an action.
 */
class remoteTask : public scheduler_task
{
public:
        remoteTask();
        bool init(void);
        bool regTlm(void);
        bool taskEntry(void);
        bool run(void *p);

private:
        void handleIrNumber(int num);
        bool getNumberFromCode(uint32_t code, uint32_t& num);

        uint32_t mNumCodes[10];     ///< IR Number codes
        uint32_t mIrNumber;         ///< Current IR number we're decoding
        xSemaphoreHandle mLearnSem; ///< Semaphore to enable IR code learning
        SoftTimer mIrNumTimer;      ///< Timeout for user entry for 1st and 2nd digit
};

#endif /* TASKS_HPP_ */
