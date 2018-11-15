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
 * @brief This file provides the configurable paramters for the CPU Clock
 * 			Note that the PLL is always enabled, and so the desired CPU_CLOCK
 * 			should be chosen based on system needs
 */
#ifndef SYSCONFIG_H_
#define SYSCONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif


#define STARTUP_DELAY_MS                2000    /* Startup delay in milli-seconds */
#define DEL_UPON_LOGFILE_MOVE_TO_SD     0       /* Delete log file from flash after moving to SD Card */
#define LOG_BOOT_INFO_TO_FILE           0       /* Log a boot message to "boot.csv" file upon every boot */
#define LOGGER_FILE_NAME                "log"   /* Logger file name */
#define DISK_TLM_NAME                   "disk"  /* Filename to save "disk" telemetry variables */
#define MAX_FILES_OPENED                3       /* Maximum files that can be opened at once */
#define ENABLE_TELEMETRY                0       /* Enable telemetry system. C_FILE_IO forced enabled if enabled */
#define ENABLE_C_FILE_IO                0       /* Allow stdio fopen() fclose() to redirect to ff.h */



/**
 * @returns actual System clock as calculated from PLL and Oscillator selection
 * @note The DESIRED_CPU_CLOCK macro defines "Desired" CPU clock, and doesn't guarantee
 *          this clock rate.  This function returns actual CPU clock of the system.
 */
unsigned int getCpuClock();


/**
 * Select the clock source:
 * - 0: Internal Clock: 4Mhz  1% Tolerance
 * - 1: External Clock: 12Mhz
 * - 2: RTC Clock     : 32.768Khz
 */
#define CLOCK_SOURCE		0


#define INTERNAL_CLOCK		(4  * 1000 * 1000UL)    ///< Do not change, this is the same on all LPC17XX
#define EXTERNAL_CLOCK      (12 * 1000 * 1000UL)    ///< Change according to your board configuration
#define RTC_CLOCK           (32768UL)               ///< Change according to your board configuration

#define DESIRED_CPU_CLOCK	(48 * 1000 * 1000UL)        ///< Define the CPU speed you desire, must be between 1-100Mhz
#define FALLBACK_CPU_CLOCK  (24 * 1000 * 1000UL)        ///< Do not change.  This is the fallback CPU speed if DESIRED_CPU_CLOCK cannot be attained
#define PERIPHERAL_CLOCK	(DESIRED_CPU_CLOCK / 4UL)   ///< Peripheral clock is setup by default to be CPU Clock divided by 4
#define TIMER0_US_PER_TICK  (10)                        ///< Microseconds per tick for Timer0 that is used by FreeRTOS



/**
 * Set printf & sprintf options - Do a clean build after changing this option
 *  - 0 : Full printf from stdio.h --> Slowest : Uses more code size + RAM
 *  - 1 : iprintf from stdio.h     --> Faster  : -16K Reduced  code size but without floating point support
 *  - 2 : Custom printf            --> Fastest : -21K Smallest code size but not complaint with all printf options
 */
#define USE_REDUCED_PRINTF        0     ///< TODO Configure your printf version here
#define UART0_DEFAULT_RATE_BPS    38400 ///< UART0 is configured at this BPS by Startup Code - before main()



/**
 * Valid years for RTC.
 * If RTC year is not found to be in between these,
 *  RTC will reset to 1/1/2000 00:00:00
 */
#define RTC_VALID_RANGE_YEARS   {2012, 2020}



/**
 * SPI Flash memory is very slow at writing data in general, but can be improved :
 * If this is set to 1, flash pages are erased when a file is deleted such that these pages
 * can be written faster at a later time.
 * If this is set to 0, flash write may take longer, but erasing files will be MUCH quicker.
 *
 * Here is a comparison :
 *  Set to 1 : 700K file written @ 150k/sec, but erased in 25 seconds!!
 *  Set to 0 : 700K file written @  25k/sec, but erased in 50 milliseconds
 *
 * @note _USE_ERASE @ ffconf.h must also be set to 1 to use this feature.
 */
#define SPI_FLASH_OPTIMIZE_WRITE    0



/**
 * Telemetry needs C file I/O so force enable it
 */
#if (ENABLE_TELEMETRY)
#undef ENABLE_C_FILE_IO
#define ENABLE_C_FILE_IO 1
#endif

#ifdef __cplusplus
}
#endif
#endif /* SYSCONFIG_H_ */
