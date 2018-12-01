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

#include "scheduler_task.hpp"
#include "soft_timer.hpp"
#include "command_handler.hpp"
#include "wireless.h"
#include "char_dev.hpp"

#include "FreeRTOS.h"
#include "semphr.h"

// For MP3 stuff
#include "ff.h"
#include <vector>
#include <string>

// These go on the shared FreeRTOS queue used to communicate with the UI thread
// These are also returned by mp3PlayerTask::playFile() to indicate which action the
// task's main thread (mp3PlayerTask::run()) should take next.
enum class mp3Command : uint32_t {
    NONE=0,  // never put on queue, returned by mp3PlayerTask::playFile() when no command is received; indicates normal operation
    SKIP=1,
    PREV=2,
    PAUSE=3, // handled internally by playFile
    PLAY=4   // handled internally by playFile
    // Add more of these once we get the basics working
};


/**
 * Contains basic logic for playing MP3 files, receiving control commands
 * on a queue. TODO: Couple with mp3ControllerTask.
 */
// Define an 8K paginator for feeding the codec (yep, I'm being lazy and using a macro)
#define BUFFER_PAGINATION_SIZE 8192
class mp3PlayerTask : public scheduler_task
{
    public:
        mp3PlayerTask(uint8_t priority) :
            scheduler_task("MP3 Player", 2048, priority)
        {
            mp3_buffer = malloc(sizeof(char) * BUFFER_PAGINATION_SIZE);
        }

        ~mp3PlayerTask() {
            free(mp3_buffer);
        }

        // Iterate over MP3 files, playing them in a loop
        // For now, let's focus on this task and not worry about control systems (that task can be
        // developed in parallel for the moment)
        bool run(void *p);

    private:
        void *mp3_buffer;
        // TODO: Next/Prev skipping along with other additional controls will be supported
        // via a queue of commands also stored with scheduler_task::addSharedObject()
        // and accessed with scheduler_task::getSharedObject() from playFile().'

        //// Infrastructure stuff
        mp3Command getNextCommand();

        //// File stuff
        // Gets a list of MP3 files and puts them in the vector (passed by reference)
        static void listMp3Files(std::vector<std::string> &files);
        // Set the current MP3 file being fed to the codec to the file with the name passed
        // Get a list of filenames to pass to this method by calling listMp3Files()
        // Command semantics
    public:
        mp3Command playFile(const std::string &name);

        //// MP3 codec interface
        void initCodec();
        void sineTest();
    private:
        void prepCodecForNewSong();
        // Sends a buffer to the codec and then waits with vTaskDelay until the codec needs more data
        void sendToCodec(void *buffer, uint32_t length);
};

/**
 * Terminal task is our UART0 terminal that handles our commands into the board.
 * This also saves and restores the "disk" telemetry.  Disk telemetry variables
 * are automatically saved and restored across power-cycles to help us preserve
 * any non-volatile information.
 */
class terminalTask : public scheduler_task
{
    public:
        terminalTask(uint8_t priority);     ///< Constructor
        bool regTlm(void);                  ///< Registers telemetry
        bool taskEntry(void);               ///< Registers commands.
        bool run(void *p);                  ///< The main loop

    private:
        // Command channels device and input command str
        typedef struct {
            CharDev *iodev; ///< The IO channel
            str *cmdstr;    ///< The command string
            bool echo;      ///< If input should be echo'd back
        } cmdChan_t;

        VECTOR<cmdChan_t> mCmdIface;   ///< Command interfaces
        CommandProcessor mCmdProc;     ///< Command processor
        uint16_t mCommandCount;        ///< terminal command count
        uint16_t mDiskTlmSize;         ///< Size of disk variables in bytes
        char *mpBinaryDiskTlm;         ///< Binary disk telemetry
        SoftTimer mCmdTimer;           ///< Command timer

        cmdChan_t getCommand(void);
        void addCommandChannel(CharDev *channel, bool echo);
        void handleEchoAndBackspace(cmdChan_t *io, char c);
        bool saveDiskTlm(void);
};

/**
 * Remote task is the task that monitors the IR remote control signals.
 * It can "learn" remote control codes by typing "learn" into the UART0 terminal.
 * Thereafter, if a user enters a 2-digit number through a remote control, then
 * your function handleUserEntry() is called where you can take an action.
 */
class remoteTask : public scheduler_task
{
    public:
        remoteTask(uint8_t priority);   ///< Constructor
        bool init(void);                ///< Inits the task
        bool regTlm(void);              ///< Registers non-volatile variables
        bool taskEntry(void);           ///< One time entry function
        bool run(void *p);              ///< The main loop

    private:
        /** This function is called when a 2-digit number is decoded */
        void handleUserEntry(int num);
        
        /**
         * @param code  The IR code
         * @param num   The matched number 0-9 that mapped the IR code.
         * @returns true if the code has been successfully mapped to the num
         */
        bool getNumberFromCode(uint32_t code, uint32_t& num);

        uint32_t mNumCodes[10];      ///< IR Number codes
        uint32_t mIrNumber;          ///< Current IR number we're decoding
        SemaphoreHandle_t mLearnSem; ///< Semaphore to enable IR code learning
        SoftTimer mIrNumTimer;       ///< Time-out for user entry for 1st and 2nd digit
};

/**
 * Nordic wireless task to participate in the mesh network and handle retry logic
 * such that packets are resent if an ACK has not been received
 */
class wirelessTask : public scheduler_task
{
    public:
        wirelessTask(uint8_t priority) :
            scheduler_task("wireless", 512, priority)
        {
            /* Nothing to init */
        }

        bool run(void *p)
        {
            wireless_service(); ///< This is a non-polling function if FreeRTOS is running.
            return true;
        }
};

/**
 * Periodic callback dispatcher task
 * This task gives the semaphores that end up calling functions at periodic_callbacks.cpp
 */
class periodicSchedulerTask : public scheduler_task
{
    public:
        periodicSchedulerTask(bool kHz=0);
        bool init(void);
        bool regTlm(void);
        bool run(void *p);

    private:
        bool handlePeriodicSemaphore(const uint8_t index, const uint8_t frequency);
        const uint8_t mKHz; // Periodic dispatcher should use 1Khz callback too
};

#endif /* TASKS_HPP_ */
