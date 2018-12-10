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
#include "Decoder.hpp"

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

void mp3PlayerTask::sendStringToDisplay(const char *str, size_t len) {

    QueueHandle_t lcd_str_queue = NULL;

    if ((lcd_str_queue = scheduler_task::getSharedObject("lcd_str_queue")) == NULL) {

        uart0_puts("Making queue");
        lcd_str_queue = xQueueCreate(6, sizeof(char *));
        scheduler_task::addSharedObject("lcd_str_queue", lcd_str_queue);
        uart0_puts("Made queue");
    }
    // This would be vulnerable code if we cared about security!
    // Copy string so that we don't have to care when the STL frees the source string
    char *cpy = (char *)calloc(sizeof(char), len+1);
    if (cpy == NULL) {
        uart0_puts("Not enough memory!");
    }
    memcpy(cpy, str, len);
    cpy[len] = '\0'; // Explicit null termination, just in case

    if (xQueueSend(lcd_str_queue, &cpy, 0) != pdPASS) {
        // We couldn't add it to the queue! Avoid a memory leak
        uart0_puts("Freeing memory");
        free(cpy);
    }
    vTaskDelay(100);
}

bool mp3PlayerTask::isSongDone() {
    uart0_puts("FIXME: Ask codec when song is done");
    vTaskDelay(1000 * 60 * 3); // Sleep for 3 minutes
    return true;
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
        sendStringToDisplay(f_name.c_str(), f_name.length());

        UINT bytes_read = 0;
        bool has_read = false;
        FRESULT read_rslt;
        while ((FR_OK == (read_rslt = f_read(&mp3_file, mp3_buffer, BUFFER_PAGINATION_SIZE, &bytes_read))) &&
                bytes_read == BUFFER_PAGINATION_SIZE) {
            has_read = true;
            // Woo! Everything was read into mp3_buffer
            sendToCodec(mp3_buffer, BUFFER_PAGINATION_SIZE);

            // Actually calculated to be 64 for 64kbps songs
            vTaskDelay(30);

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

                    stopSong(&mp3_file); // Forcibly stop song
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
            // TODO: Do we need to do some
            // Uh-oh, something went bad with the read
            // UI TODO: Display error message (maybe use the code from in read_rslt "READ ERR: <CODE>")
            uart0_puts("Error reading from file: error code returned from f_read");

            if (has_read) {
                endSong(); // Attempt to gracefully end song as if we reached the end
            }
        } else {
            // bytes read != file size -- looks like the end!
            sendToCodec(mp3_buffer, bytes_read);
            f_close(&mp3_file);
            endSong(); // Gracefully end song

            return getNextCommand();
        }


    } else {
        // UI TODO: Display error message (maybe use the code from in open_rslt "OPEN ERR: <CODE>")
        uart0_puts("Error opening mp3 file");
    }
    return mp3Command::NONE;
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

    // Adding mutex around this whole thing as I am not sure where spi1 is called
    SemaphoreHandle_t spi_bus_lock = NULL;

    if((spi_bus_lock = scheduler_task::getSharedObject("spi_bus_lock")) == NULL){

            spi_bus_lock = xSemaphoreCreateMutex();
            uart0_puts("Made SPI bus lock\n");
            scheduler_task::addSharedObject("spi_bus_lock", spi_bus_lock);
            uart0_puts("Added SPI bus lock as shared object\n");
        }



    initCodec();
    while (true) {

        for (uint32_t i = 0; i < mp3_files.size(); ++i) {
            uart0_puts(mp3_files[i].c_str());
            sprintf(scratch, "1:%s", mp3_files[i].c_str());
            std::string full_path(scratch);

            // This funtion heavily uses the spi1 bus needs lock.
            // There should be no skipping because of this since this task
            // has critical priority and runLCD task has low priority
            if(xSemaphoreTake(spi_bus_lock, 1000) == pdTRUE){
                mp3Command cmd = playFile(full_path);

                if (cmd == mp3Command::SKIP) {
                    // Do nothing -- this just means we returned early from the song
                } else if (cmd == mp3Command::PREV) {
                    // Go to previous song, wrap around with modulus operator
                    i = (i - 3) % mp3_files.size();
                }
                // Pause and play are handled inside playFile(), and no other commands exist for now

                xSemaphoreGive(spi_bus_lock);
                vTaskDelay(1);
            }
        }
    }
    return true;
}

void mp3PlayerTask::sineTest() {
    Decoder dec;

    dec.hardReset();
    dec.decoderInit();
    dec.setVolume(30, 30);
    dec.sineTest();
}

void mp3PlayerTask::initCodec() {
    Decoder dec;

    dec.decoderInit();
    // TODO: Write me!
    // TODO: Do anything you need to initialize the communication channel to the codec and the codec itself.
}

void mp3PlayerTask::endSong() {
    Decoder dec;

    dec.endSong();
    // TODO: Write me!
    // TODO: Do anything you need to initialize the communication channel to the codec and the codec itself.
}

void mp3PlayerTask::stopSong(FIL *file) {
    Decoder dec;

    dec.stopSong(file);
    // TODO: Write me!
    // TODO: Do anything you need to initialize the communication channel to the codec and the codec itself.
}

void mp3PlayerTask::prepCodecForNewSong() {
    // TODO: Write me!
    Decoder dec;
    dec.initSong();
    // TODO: Reset the codec's state as necessary. After this returns, sendToCodec will send the first chunk of the song.
}

void mp3PlayerTask::sendToCodec(void *buffer, uint32_t length) {
    // TODO: Write me!

    Decoder dec;
    dec.transferData((char *)buffer, length);
}
