/**
 * @file	: main.c
 * @purpose	: Chan's FatFs on LPC17xx demo-application
 * @version	: 0.1
 * @date	: July 2010
 * @author	: Martin Thomas
 *----------------------------------------------------------------------------
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * the author assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. The author
 * reserves the right to make changes in the software without
 * notification. The author also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#define VERSION_STRING "0.0.2"

#include "lpc17xx.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_rtc.h"
#include "rtc_cal.h"

#include "comm.h"
#include "monitor.h"
#include "ff_test_term.h"
#include "diskio.h"

extern void usb_test1(void);

/************************** PRIVATE VARIABLES *************************/

// LED1 on Olimex LPC1766-STK connected to P1.25 (low = on)
#define LED1_PORT  1
#define LED1_PIN   25
#define LED1_MASK (1 << LED1_PIN)

volatile uint32_t SysTickCnt;      /* SysTick Counter                    */

/************************** PRIVATE FUNCTIONS *************************/

static void init_in_main(void)
{
	RTC_TIME_Type initial_time = {
		0, 55, 11, // SS:MM:HH
		15, 0, 0,  // DOM, DOW, DOY
		7, 2010    // MONTH, YEAR
	};

	/* LED(s) */
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum   = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode   = 0;
	PinCfg.Pinnum    = LED1_PIN;
	PinCfg.Portnum   = LED1_PORT;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(1, LED1_MASK, 1);

	/* RTC */
	rtc_cal_init(&initial_time);
}

static inline void LED1_on(void)
{
	FIO_ClearValue(LED1_PORT, LED1_MASK);
}

static inline void LED1_off(void)
{
	FIO_SetValue(LED1_PORT, LED1_MASK);
}

/* SysTick Interrupt Handler (1ms) */
void SysTick_Handler (void)
{
	static uint32_t flip, led_prescale;
	static uint32_t div10;

	if (++led_prescale > 500) {
		led_prescale = 0;
		if (flip) { LED1_on();  }
		else      { LED1_off(); }
		flip = !flip;
	}

	if (++div10 >= 10) {
		div10 = 0;
		disk_timerproc(); /* Disk timer function (100Hz) */
	}

	ff_test_term_timer_callback();

	SysTickCnt++;
}


/*********************************************************************//**
 * @brief	Main sub-routine
 **********************************************************************/
int c_entry(void)
{
	FlagStatus exitflag;
	char key;

	// DeInit NVIC and SCBNVIC
	NVIC_DeInit();
	NVIC_SCBDeInit();

	/* Configure the NVIC Preemption Priority Bits:
	 * two (2) bits of preemption priority, six (6) bits of sub-priority.
	 * Since the Number of Bits used for Priority Levels is five (5), so the
	 * actual bit number of sub-priority is three (3)
	 */
	NVIC_SetPriorityGrouping(0x05);

	//  Set Vector table offset value
#if (__RAM_MODE__==1)
	NVIC_SetVTOR(0x10000000);
#else
	NVIC_SetVTOR(0x00000000);
#endif

	init_in_main();
	LED1_on();

	comm_init();

	comm_puts("Hello from the ChaN FatFs Demo on LPC1700\r\n");
	comm_puts("Version " VERSION_STRING ", Martin Thomas 7/2010\r\n");
#ifdef DEBUG
	comm_puts("DEBUG build\r\n");
#endif

	xfunc_out = xcomm_put;
	xfunc_in  = xcomm_get;
	xprintf("xprintf is working\n");


	SysTick_Config(SystemCoreClock/1000 - 1); /* Generate interrupt each 1 ms   */

#if 0
	usb_test1();
#endif

	// reset exit flag
	// exitflag = RESET;
	exitflag = SET;

	/* Read some data from the buffer */
	while (exitflag == RESET)
	{

		if ( comm_test() ) {
			key = comm_get();
			if (key == 27) {
				/* ESC key, set exit flag */
				comm_puts("Exiting...\r\n");
				exitflag = SET;
			} else {
				/* Echo it back */
				comm_put(key);
			}
		}
	}

	// wait for current transmission complete - THR must be empty
	while ( comm_txbusy() ) { ; }

	ff_test_term();

	/* Loop forever */
	while (1)
		;
	return 1;
}

/* With ARM and GHS toolsets, the entry point is main() - this will
 allow the linker to generate wrapper code to setup stacks, allocate
 heap area, and initialize and copy code and data segments. For GNU
 toolsets, the entry point is through __start() in the crt0_gnu.asm
 file, and that startup code will setup stacks and data */
int main(void)
{
	return c_entry();
}

void startup_delay(void)
{
	for (volatile unsigned long i = 0; i < 500000; i++) { ; }
}

#ifdef DEBUG
/*******************************************************************************
 * @brief		Reports the name of the source file and the source line number
 * 				where the CHECK_PARAM error has occurred.
 * @param[in]	file Pointer to the source file name
 * @param[in]    line assert_param error line source number
 * @return		None
 *******************************************************************************/
void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

#if 1
	xprintf("!! check failed in file %s, line %ld", file, line);
#else
	/* mthomas: avoid unused waring: */
	(void) file;
	(void) line;

	/* Infinite loop */
	while (1)
		;
#endif
}
#endif
