To get started, have a look at L5_Application/main.cpp.  You can change a #define 0 to #define 1
and look at examples on how to develop your own application.


Revision History:

06-05-2013 : 
    Major changes to optimize memory, and add examples.
    1)  Changed rtc_alarm.h API to be more consistent and added example in header file
    2)  Added Examples directory to L5_Application
    3)  Added malloc lock and unlock to make it safe across FreeRTOS tasks.
    4)  Removed unncessary hooks.h file
    5)  Added more documentation at tasks.hpp and added handleUserEntry() for remoteTask class.
    6)  Changed command_handler.hpp by removing output str and adding output interface.
          Changed all commands to directly output the data at tasks.cpp
    7)  Combined uart_base.hpp and uart_iface.hpp
    8)  Added lpc_pwm.hpp that supports 3 PWM channels on SJ One (v1) board.
    9)  Added tokenize() to str class (str.hpp and str.cpp)
    10) Changed uart_dev.cpp printf() to use heap vs. stack memory to do the printf
    11) Added printf_lib.h to do u0_dbg_printf().  See file for more documentation.
    12) Removed handles.h and misc.c
    13) Added spi_sem.h and spi_sem.c
    14) Changed syscalls.c to fix stdout, stdin, and stderr I/O
    15) Enhanced scheduler_task() such that tasks can communicate with each other.
    16) Added SoftTime class that relies on FreeRTOS.
    
04-25-2013
    1) Added newlib hooks for time functions
    2) Fixed infinite loop when no command was sent for 60 seconds.
    
04-20-2013
    1) Fixed gets() bug at uart_iface.hpp
    2) Modified command_handler.hpp
    
04-13-2013
    1) Added rit_isr.h to provide API to setup repetitive callback which also provides
        a free running timer for free.  RIT uses higher priority than FreeRTOS and if
        it sees that FreeRTOS is not running, it will set BASEPRI register to zero to
        ensure that accidental use of FreeRTOS API hasn't set the BASEPRI to non-zero
        therefore blocking our interrupts higher than IP_SYSCALL
    2) Fixed bug at uart_base.cpp for timeout without FreeRTOS.
    3) Changed uart_base.cpp such that last activity time is when a char was received
        such that when a char is sent, it is not considered as any activity.
    4) Fixed bug at uart_iface.cpp for gets()

03-29-2013
    Removed memory configuration options.  Now heap can use almost all of 64Kb memory
    
02-01-2013:
    Added telemetry system.
    Newlib hooks added to use C FILE API, such as fopen() and fclose()
    
10-16-2012:
    Added option to optimize SPI Flash write operation
    Changed UART0 PCLK to CPU Clock instead of CPU/4 to support higher baud rates
    
10-07-2012:
    Fixed critical FreeRTOS Priority bug by adding interruptPriorities.h
    Included RTC Semaphore for FreeRTOS
    DMA for SSP1
    Added C++ Interface inheritance for UART driver
    Added Boot information by L0/bootType.h
    Added bit manipulation by L0/bit_manip.h
    Added watchdog timer L0/watchdog.h
    Added different memory schemes by configuring it at sys_config.h
    Added more functionality to storage.h
    Enabled sleep-mode configuration for temperature sensor.
    
09-16-2012:
    str needed str.printf() fixed when string couldn't be printed due to insufficient capacity.
    FATFS needed fix to get_fattime() due to change to RTC API()