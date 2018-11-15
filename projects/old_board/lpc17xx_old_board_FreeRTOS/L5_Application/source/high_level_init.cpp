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

#include <stdio.h>           // printf

#include "sys_config.h"     // CPU Clock Configuration
#include "rit_isr.h"        // SD card polling callback is used for RIT
#include "utilities.h"      // printMemoryInfo()

#include "fat/disk/sd.h"        // Initialize SD Card Pins for CS, WP, and CD
#include "fat/disk/spi_flash.h" // Initialize Flash CS pin

#include "rtc.h"             // RTC init
#include "I2C2.hpp"          // I2C1 init
#include "adc0.h"            // ADC0 init
#include "spi1.h"            // SPI-1 init
#include "storage.hpp"       // Mount Flash & SD Storage
#include "io.hpp"            // Board IO

#include "boot_type.h"
#include "fault_registers.h"
#include "c_tlm_comp.h"



typedef void (*voidFuncPtr)(void);
void setupPeriodicCallBack(voidFuncPtr pFunction, unsigned int timeMs);

bool mountStorage(FileSystemObject& drive, const char* pDescStr);
void copyLogFileToSDCard();
bool discoverExternalDevsOnI2C();
bool initializeBoardIO();
void printMemoryInfo();
void printLine() { puts("------------------------------------------"); }


/**
 * Initializes the High Level System such as IO Pins and Drivers
 */
void highLevelInitialize(void)
{
    printLine();

    #if ENABLE_TELEMETRY
    /* Add default telemetry components */
    tlm_component_add("disk");
    tlm_component_add("debug");
    #endif

    /**
     * Initialize the signals used by the SD Card and Flash Memory
     */
    sd_initializeCardSignals(); // Initialize CS signal, WP and CD signals used for SD Card
    flash_InitializeSignals();  // Initialize CS signal

    /**
     * Setup Timer0 so that delay_ms(us) functions will work.
     * vConfigureTimerForRunTimeStats() is used by FreeRTOS run time statistics.
     * If FreeRTOS is used, timer will be setup anyway.
     * If FreeRTOS is not used, call it here to delay_ms(us) functions will work.
     */
    vConfigureTimerForRunTimeStats();

    /**
     * Intentional delay here because this gives us some time to
     * close COM Port at Hyperload and Open it at Hercules
     */
    delay_ms(STARTUP_DELAY_MS);

    /**
     * Print Boot Info
     */
    const unsigned int cpuClock = getCpuClock();
    printf("\nSystem Boot @ %u.%u Mhz\n", cpuClock / (1000 * 1000), cpuClock % (1000 * 1000));
    if(rebootType_watchdogRecover == getRebootCause()) {
        printLine();
        printf("System rebooted after crash.  Relevant info:\n");
        printf("PC: 0x%08X.  LR: 0x%08X.  PSR: 0x%08X\n",
                (unsigned int)FAULT_PC, (unsigned int)FAULT_LR, (unsigned int)FAULT_PSR);
        printLine();
    }

    /**
     * Initialize the Peripherals used in the system
     * I2C2 : Used by LED Display, Acceleration Sensor, Temperature Sensor
     * ADC0 : Used by Light Sensor
     * SPI1 : Used by SD Card & External SPI Flash Memory
     */
    adc0_initialize();
    spi1_Init();
    if (false == I2C2::getInstance().init(100)) {
        puts("I2C #2 : ERROR!  Possible short on SDA or SCL wire!");
    }

    /**
     * Install 10ms periodic callback outside of FreeRTOS since we want to support
     * FATFS even if FreeRTOS is not running.  Need IRQ enabled for this to work.
     *
     * Initialize the SPI Mutex that the DISK IO Layer will use (if FreeRTOS is running),
     * and then try to mount both Flash Storage and SD Card Storage and print their info.
     */
    rit_setup_callback(sd_timerproc, 10);

    /**
     * If Flash is not mounted, it is probably a new board and the flash is not
     * formatted so format it, alert the user, and try to re-mount it
     */
    bool flashMounted = mountStorage(Storage::getFlashDrive(), " Flash ");
    if(!flashMounted)
    {
        printf("FLASH not formatted, formatting now ... ");
        printf("%s\n", FR_OK == Storage::getFlashDrive().format() ? "Done" : "Error");
        flashMounted = mountStorage(Storage::getFlashDrive(), " Flash ");
    }

    /**
     * If SD Card is present, try to copy log file from Flash to SD Card
     */
    bool sdMounted = mountStorage(Storage::getSDDrive(),    "SD Card");
    if(flashMounted && sdMounted)
    {
        copyLogFileToSDCard();
    }

    /**
     * SD Card specifications are 24Mhz maximum
     */
    spi1_SetMaxClockMhz(24);
    printLine();

    #if LOG_BOOT_INFO_TO_FILE
    logBootInfo(__DATE__);
    #endif

    /**
     * Print discovered I2C devices.
     * But not really useful if no external I2C device attached.
     */
    if(discoverExternalDevsOnI2C()) {
        printLine();
    }

    /**
     * Initialize all sensors of this board.
     */
    if(!initializeBoardIO()) {
        printLine();
    }

    /**
     * Print memory used so far.
     * This will show memory usage before main() starts.
     */
    char buff[512];
    printMemoryInfo(buff);
    puts(buff);
    printLine();

    // Display CPU speed in Mhz on LED display
    LD.setNumber(getCpuClock()/(1000*1000));

    puts("Calling your main()");
    printLine();
}

bool mountStorage(FileSystemObject& drive, const char* pDescStr)
{
    unsigned int totalKb = 0;
    unsigned int availKb = 0;
    bool mounted = (drive.mount() &&  FR_OK == drive.getDriveInfo(&totalKb, &availKb));

    if(mounted)
    {
        if(totalKb < 1000)
            printf("%s: OK -- Capacity %-5uKB, Available: %-5uKB\n", pDescStr, totalKb, availKb);
        else
            printf("%s: OK -- Capacity %-5uMB, Available: %-5uMB\n", pDescStr, totalKb/1024, availKb/1024);
    }
    else {
        printf("%s: Error or not present.\n", pDescStr);
    }

    return mounted;
}

void copyLogFileToSDCard()
{
    // Open destination file with same filename but store into drive '1' for SD Card
    char dstFileName[] = LOGGER_FILE_NAME;
    dstFileName[0] = '1';

    printf("Copying %s to SD Card ... ", (char*)LOGGER_FILE_NAME);

    // Copy log file from Flash to SD Card:
    FRESULT copyStatus = Storage::copy(LOGGER_FILE_NAME, dstFileName);

    // Delete logger file from FLASH (if you want to)
    #if DEL_UPON_LOGFILE_MOVE_TO_SD
    if(FR_OK == copyStatus) {
        f_unlink(LOGGER_FILE_NAME);
    }
    #endif

    printf("%s\n", (FR_OK == copyStatus) ? "Done!" : (FR_NO_FILE == copyStatus) ? "Not found" : "ERROR!");
}

bool discoverExternalDevsOnI2C()
{
    bool found = false;

    for(unsigned int id = 2; id <= 254; id += 2)
    {
        if(I2C2::getInstance().isDevicePresent(id))
        {
            if(id != Address_AccelerationSensor &&
               id != Address_LED_Display &&
               id != Address_TemperatureSensor)
            {
                printf("Found External I2C Device: 0x%02X\n", id);
                found = true;
            }
        }
    }

    return found;
}

bool initializeBoardIO()
{
    bool success = true;

    if(!AS.init()) { puts("Acceleration Sensor Error"); success = false; }
    if(!IS.init()) { puts("Infrared Sensor Error"); success = false; }
    if(!LS.init()) { puts("Light Sensor Error"); success = false; }
    if(!TS.init()) { puts("Temperature Sensor Error"); success = false; }

    if(!LD.init()) { puts("7-Segment Display Error"); success = false; }
    if(!LE.init()) { puts("LED Error"); success = false; }
    if(!SW.init()) { puts("Switches Error"); success = false; }

    return success;
}
