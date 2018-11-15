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


#include <stdio.h>              // setvbuf & printf

#include "sys_config.h"          // CPU Clock Configuration
#include "LPC17xx.h"            // LPC MemoryMap
#include "uart0_min.h"          // Uart0 init
#include "io_functions.h"       // Uart0 IO connect to stdio
#include "isr_priorities.h"
#include "watchdog.h"           // Enable watchdog early
#include "boot_type.h"          // RebootType structure
#include "fault_registers.h"



extern "C" void syscallsInit(void);
extern void configureSystemClock();

void configureFlashAccelerator(unsigned int clock)
{
    const unsigned int clockMhz = clock / (1000 * 1000);

    // Flash accelerator parameters depends on CPU clock to optimize program code read (CPU instructions)
    switch(clockMhz)
    {
        case  0 ... 20  : LPC_SC->FLASHCFG = (0 << 12) | 0x03A; break;
        case 21 ... 40  : LPC_SC->FLASHCFG = (1 << 12) | 0x03A; break;
        case 41 ... 60  : LPC_SC->FLASHCFG = (2 << 12) | 0x03A; break;
        case 61 ... 80  : LPC_SC->FLASHCFG = (3 << 12) | 0x03A; break;
        case 81 ... 100 : LPC_SC->FLASHCFG = (4 << 12) | 0x03A; break;
        default:
            LPC_SC->FLASHCFG = (5 << 12) | 0x03A; /* works for all clock settings */
            break;
    }
}

void initializeDefaultInterruptPriority()
{
    NVIC_SetPriority(UART3_IRQn,        IP_Default);
    NVIC_SetPriority(WDT_IRQn,          IP_Default);
    NVIC_SetPriority(TIMER0_IRQn,       IP_Default);
    NVIC_SetPriority(TIMER1_IRQn,       IP_Default);
    NVIC_SetPriority(TIMER2_IRQn,       IP_Default);
    NVIC_SetPriority(TIMER3_IRQn,       IP_Default);
    NVIC_SetPriority(UART0_IRQn,    IP_Uart);
    NVIC_SetPriority(UART1_IRQn,    IP_Uart);
    NVIC_SetPriority(UART2_IRQn,    IP_Uart);
    NVIC_SetPriority(UART3_IRQn,    IP_Uart);
    NVIC_SetPriority(PWM1_IRQn,         IP_Default);
    NVIC_SetPriority(I2C0_IRQn,     IP_I2C);
    NVIC_SetPriority(I2C1_IRQn,     IP_I2C);
    NVIC_SetPriority(I2C2_IRQn,     IP_I2C);
    NVIC_SetPriority(SPI_IRQn,          IP_Default);
    NVIC_SetPriority(SSP0_IRQn,         IP_Default);
    NVIC_SetPriority(SSP1_IRQn,         IP_Default);
    NVIC_SetPriority(PLL0_IRQn,         IP_Default);
    NVIC_SetPriority(RTC_IRQn,          IP_Default);
    NVIC_SetPriority(EINT0_IRQn,        IP_Default);
    NVIC_SetPriority(EINT1_IRQn,        IP_Default);
    NVIC_SetPriority(EINT2_IRQn,        IP_Default);
    NVIC_SetPriority(EINT3_IRQn,        IP_Default);
    NVIC_SetPriority(ADC_IRQn,          IP_Default);
    NVIC_SetPriority(BOD_IRQn,          IP_Default);
    NVIC_SetPriority(USB_IRQn,          IP_Default);
    NVIC_SetPriority(CAN_IRQn,          IP_Default);
    NVIC_SetPriority(DMA_IRQn,          IP_Default);
    NVIC_SetPriority(I2S_IRQn,          IP_Default);
    NVIC_SetPriority(ENET_IRQn,         IP_Default);
    NVIC_SetPriority(RIT_IRQn,      IP_RIT);
    NVIC_SetPriority(MCPWM_IRQn,        IP_Default);
    NVIC_SetPriority(QEI_IRQn,          IP_Default);
    NVIC_SetPriority(PLL1_IRQn,         IP_Default);
    NVIC_SetPriority(USBActivity_IRQn,  IP_Default);
    NVIC_SetPriority(CANActivity_IRQn,  IP_Default);
}

static RebootType mRebootType = rebootType_unknown; // Store reboot type for anyone to query
RebootType getRebootCause()
{
    return mRebootType;
}
static RTC mRtcBootTime = {0};
RTC getBootTime()
{
    return mRtcBootTime;
}

static void printBootInfo(void)
{
    if (LPC_SC->RSID & 1) {
        uart0Puts("System Power-on: Booting ...");
        mRebootType = rebootType_powerOn;
        LPC_SC->RSID |= 1; // Clear POR bit
    }
    else if(LPC_SC->RSID & 2) {
        uart0Puts("System Reset: Booting ...");
        mRebootType = rebootType_reset;
        LPC_SC->RSID |= 2; // Clear RESET bit
    }
    else if(LPC_SC->RSID & 4) {
        if(FAULT_EXISTS == 0xDEADBEEF) {
            mRebootType = rebootType_watchdogRecover;
            uart0Puts("System Crash: Re-booting ...");
        }
        else {
            mRebootType = rebootType_watchdog;
            uart0Puts("Watchdog System Reset: Booting ...");
            LPC_SC->RSID |= 4; // Clear WDTR bit
        }
        FAULT_EXISTS = 0;
    }
    else {
        mRebootType = rebootType_unknown;
    }
}

/**
 * Initializes the minimal system including CPU Clock, UART, and Flash accelerator
 * Be careful of the order of operations!!!
 */
void lowLevelInitialize(void)
{
    rtc_initialize();
    mRtcBootTime = rtc_gettime();

    /**
     * Configure System Clock based on desired clock rate @ sys_config.h
     * Setup default interrupt priorities that will work with FreeRTOS
     */
    configureSystemClock();
    configureFlashAccelerator(getCpuClock());
    initializeDefaultInterruptPriority();
    __enable_irq();

    // Setup UART with minimum I/O functions
    uart0Init(UART0_DEFAULT_RATE_BPS, (getCpuClock() / 4));
    stdio_SetOutputCharFunction(uart0Putchar);
    stdio_SetInputCharFunction(uart0Getchar);

    /**
     *  If buffers are set to 0, so printf/scanf will behave correctly!
     *  If not set, then printf/scanf will have weird buffering/flushing effects
     */
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stdin,  0, _IONBF, 0);

    /**
     * Set the semaphore for the malloc lock.
     * Initialize newlib fopen() fread() calls support
     */
    syscallsInit();

    /**
     * Enable watchdog to allow us to recover in case of:
     *  - We attempt to run an application and it's not there
     *  - Application we ran crashes
     */
    watchdog_Enable();
    printBootInfo();
}

