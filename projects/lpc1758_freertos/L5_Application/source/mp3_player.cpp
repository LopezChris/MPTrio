/*
 * mp3_player.cpp
 *
 *  Created on: Oct 12, 2018
 *      Author: nroth
 */
#include <uart0_min.h>
#include <ff.h>
#include <string.h>
#include <vector>
#include <string>
#include <utility>
#include <stdio.h>

#include "tasks.hpp"

void mp3PlayerTask::listMp3Files(std::vector<std::string> &files) {
    // This code is based on the "ls" terminal command implementation
    DIR Dir;
    FILINFO Finfo;
    FRESULT returnCode = FR_OK;

    unsigned int fileBytesTotal = 0, numFiles = 0, numDirs = 0;
    #if _USE_LFN
        char Lfname[_MAX_LFN];
    #endif

    const char *dirPath = "1:";
    if (FR_OK != (returnCode = f_opendir(&Dir, dirPath))) {
        // Error: return empty vector
        return;
    }

    // Enumerate files
    for (;;)
    {
        #if _USE_LFN
            Finfo.lfname = Lfname;
            Finfo.lfsize = sizeof(Lfname);
        #endif

        returnCode = f_readdir(&Dir, &Finfo);
        if ( (FR_OK != returnCode) || !Finfo.fname[0]) {
            uart0_puts("done with files...");
            break;
        }

        if (Finfo.fattrib & AM_DIR){
            numDirs++;
        }
        else{
            numFiles++;
            fileBytesTotal += Finfo.fsize;
        }

        if ((Finfo.fattrib & AM_HID) == 0 && (Finfo.fattrib & AM_ARC) != 0) {
            // Consider all non-hidden files in the top directory for our list
            size_t name_len = strnlen(Lfname, _MAX_LFN);
            if (name_len > 4 && (
                    !(strncmp(Lfname + (name_len-4), ".mp3", 4)) || (!(strncmp(Lfname + (name_len-4), ".MP3", 4))))) {
                // This is an MP3 file
                std::string name(Lfname);
                files.emplace_back(name);
            }
        }
        // Note: Finfo.fsize
    }
    return;
}

mp3Command mp3PlayerTask::playFile(const std::string &f_name) {
    FIL mp3_file;
    FRESULT open_rslt;
    bool paused = false;
    uart0_puts("About to open MP3");
    uart0_puts(f_name.c_str());
    if (FR_OK == (open_rslt = f_open(&mp3_file, (TCHAR *)f_name.c_str(), FA_READ))) {
        // Assuming for now that we have the space, let's just read the entire thing
        // into RAM
        // Using FreeRTOS heap 3, so compiler-provided malloc() and free() should work
        // if Preet set everything up correctly
        uart0_puts("File opened");
        prepCodecForNewSong();

        UINT bytes_read = 0;
        FRESULT read_rslt;
        while ((FR_OK == (read_rslt = f_read(&mp3_file, mp3_buffer, BUFFER_PAGINATION_SIZE, &bytes_read))) &&
                bytes_read == BUFFER_PAGINATION_SIZE) {
            // Woo! Everything was read into mp3_buffer
            sendToCodec(mp3_buffer, BUFFER_PAGINATION_SIZE);

            // Check for and react to commands while playing the song
            do {
                mp3Command cmd = getNextCommand();
                if (cmd == mp3Command::PAUSE) {
                    paused = true;
                    // While paused, if we change songs the intention is obviously
                    // to play that song so we can ignore "paused" state outside of this method
                } else if (cmd == mp3Command::PLAY) {
                    paused = false;
                } else if (cmd != mp3Command::NONE) {
                    // All other commands are large-scale control operations that would
                    // involve leaving the song (true for now -- if we ever implement scan forward and
                    // scan backward, we can add that functionality as a case above)
                    f_close(&mp3_file);
                    return cmd;
                }
                if (paused) {
                    // Avoid tight loop
                    vTaskDelay(100);
                }
            } while (paused);
        }
        if (read_rslt != FR_OK) {
            // Uh-oh, something went bad with the read
            // UI TODO: Display error message (maybe use the code from in read_rslt "READ ERR: <CODE>")
            uart0_puts("Error reading from file: error code returned from f_read");
        } else {
            // bytes read != file size -- looks like the end!
            sendToCodec(mp3_buffer, bytes_read);
            f_close(&mp3_file);
            return getNextCommand();
        }

        free(mp3_buffer);
    } else {
        // UI TODO: Display error message (maybe use the code from in open_rslt "OPEN ERR: <CODE>")
        uart0_puts("Error opening mp3 file");
    }
}

mp3Command mp3PlayerTask::getNextCommand() {
    QueueHandle_t task_queue = scheduler_task::getSharedObject("mp3_cmd_queue");
    if (task_queue != NULL) {
        mp3Command received = mp3Command::NONE;
        if (pdTRUE == xQueueReceive(task_queue, &received, sizeof(mp3Command))) {
            // We received a command! Return it!
            return received;
        }
    }
    return mp3Command::NONE;
}

bool mp3PlayerTask::run(void *p) {

    std::vector<std::string> mp3_files;
    mp3PlayerTask::listMp3Files(mp3_files);
    char scratch[100];

    initCodec();
    while (true) {

        for (uint32_t i = 0; i < mp3_files.size(); ++i) {
            uart0_puts(mp3_files[i].c_str());
            sprintf(scratch, "1:%s", mp3_files[i].c_str());
            std::string full_path(scratch);
            mp3Command cmd = playFile(full_path);
            if (cmd == mp3Command::SKIP) {
                // Do nothing -- this just means we returned early from the song
            } else if (cmd == mp3Command::PREV) {
                // Go to previous song, wrap around with modulus operator
                i = (i - 2) % mp3_files.size();
            }
            // Pause and play are handled inside playFile(), and no other commands exist for now
        }
        vTaskDelay(100);
    }
    return true;
}

void mp3PlayerTask::initCodec() {
    // TODO: Write me!
    // TODO: Do anything you need to initialize the communication channel to the codec and the codec itself.
}

void mp3PlayerTask::prepCodecForNewSong() {
    // TODO: Write me!
    // TODO: Reset the codec's state as necessary. After this returns, sendToCodec will send the first chunk of the song.
}

void mp3PlayerTask::sendToCodec(void *buffer, uint32_t length) {
    // TODO: Write me!

    uart0_puts("TODO: Send MP3 chunk to MP3 codec and wait for it to need more");
    // TODO: Send to codec device and poll with vTaskDelay until it is ready for another, then return.
}