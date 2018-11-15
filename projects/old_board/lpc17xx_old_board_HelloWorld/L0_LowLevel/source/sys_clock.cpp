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

#include "sys_config.h"
#include "LPC17xx.h"

/// Configures the System clock based on the parameters given in sys_config.h
void configureSystemClock();

/// See function declaration below for documentation.  This function is used by configureSystemClock()
bool calculatePLLParameters(const unsigned int desiredCpuSpeedKhz,
							const unsigned int inputFreqKhz, int* pPLL_M, int* pPLL_N, int* pCPU_D);

/**
 * Feed is used to make PLL register contents take effect
 */
static void pll_feed()
{
    LPC_SC->PLL0FEED = 0xAA;
    LPC_SC->PLL0FEED = 0x55;
}

/**
 * Disables PLL0 (in case bootloader has it enabled)
 * and defaults to using internal oscillator
 */
static void disablePllUseInternal4Mhz()
{
    LPC_SC->PLL0CON &= ~(1 << 1);
    pll_feed();

    // Disable PLL0
    LPC_SC->PLL0CON &= ~(1 << 0);
    pll_feed();

    LPC_SC->CLKSRCSEL = 0;
    LPC_SC->PLL0CFG = 0;    // M=1, N=1 (disregarded anyway after PLL is disabled)
    pll_feed();
    LPC_SC->CCLKCFG = 0;    // Divider = 1 when PLL is not used
}

/**
 * Configures System clock based on
 */
void configureSystemClock()
{
	typedef union
	{
		unsigned int raw;
		struct
		{
			unsigned int msel : 16;
			unsigned int nsel : 8;
			unsigned int reserved :8;
		}__attribute__ ((packed));
	} PLL0ConfigType;
	PLL0ConfigType PLL0ConfigValue = { 0 };

	int m = 0;
	int n = 0;
	int d = 0;
	unsigned int cpuClockKhz = DESIRED_CPU_CLOCK / 1000;
#if (0 == CLOCK_SOURCE)
	unsigned int PLLInputClockKhz = INTERNAL_CLOCK / 1000;
#elif (1 == CLOCK_SOURCE)
	unsigned int PLLInputClockKhz = EXTERNAL_CLOCK / 1000;
#elif (2 == CLOCK_SOURCE)
	unsigned int PLLInputClockKhz = RTC_CLOCK / 1000;
#else
#error "Clock source must be 1,2, or 3"
#endif

	// If we cannot calculate desired CPU clock, then default to a safe value
	if(!calculatePLLParameters(cpuClockKhz, PLLInputClockKhz, &m, &n, &d)) {
	    cpuClockKhz = FALLBACK_CPU_CLOCK / 1000;
	    calculatePLLParameters(cpuClockKhz, PLLInputClockKhz, &m, &n, &d);
	}
	PLL0ConfigValue.msel = m;
	PLL0ConfigValue.nsel = n;

	// Disconnect PLL0 (in case bootloader uses PLL0)
	// Choose internal oscillator going forward (for now)
	disablePllUseInternal4Mhz();

	// Enable main oscillator if needed :
#if (1 == CLOCK_SOURCE)
	LPC_SC->SCS = (1 << 5); // Main Oscillator is enabled
	while ((LPC_SC->SCS & (1 << 6)) == 0)
		; // Wait for main oscillator to be ready
#endif

	LPC_SC->PCLKSEL0 = 0; // Peripheral Clock Selection set initially @ CLK/4
	LPC_SC->PCLKSEL1 = 0; // Peripheral Clock Selection set initially @ CLK/4

	// Select the clock source and if the clock source is the desired clock, then
	// do not use PLL at all and simply return!
	LPC_SC->CLKSRCSEL = CLOCK_SOURCE;

	if(DESIRED_CPU_CLOCK == PLLInputClockKhz*1000) {
	    return;
	}
	else {
	    LPC_SC->CCLKCFG  = d; // Setup Clock Divider
	    LPC_SC->PLL0CFG = PLL0ConfigValue.raw; // Set values of PLL Multiplier and divider
	    pll_feed();

	    LPC_SC->PLL0CON = 0x01; // Enable PLL0
	    pll_feed();
	    while (!(LPC_SC->PLL0STAT & (1 << 26)))
	        ; // Wait for PLL0 to lock

	    LPC_SC->PLL0CON = 0x03; // PLL0 Enable & Connect
	    pll_feed();

	    // Finally, wait for PLLC0_STAT & PLLE0_STAT
	    while (!(LPC_SC->PLL0STAT & ((1 << 25) | (1 << 24))))
	        ;
	}
}

/**
 * Calculates the PLL Parameters needed to set the desired CPU Speed
 * @note The parameters are in KiloHertz to avoid overflow of unsigned int (32-bit)
 * @param desiredCpuSpeedKhz  The desired CPU speed in KiloHertz
 * @param inputFreqKhz        The input clock to the PLL in KiloHertz
 * @param pPLL_M    The PLL "M" parameter that multiplies PLL input
 * @param pPLL_N    The PLL "N" Parameter that divides PLL output
 * @param pCPU_D    The CPU divider post the PLL output to feed to the CPU and peripheral clock
 * @post pPLL_M, pPLL_N, and pCPU_D will be set to the values that match or closely match desiredCpuSpeedKhz
 */
bool calculatePLLParameters(const unsigned int desiredCpuSpeedKhz,
		const unsigned int inputFreqKhz, int* pPLL_M, int* pPLL_N, int* pCPU_D)
{
	*pPLL_M = 0;
	*pPLL_N = 0;
	*pCPU_D = 0;

	/**
	 * This simple algorithm (not really optimized in any way), will scroll through
	 * possible values of M, N, and D (See PLL Chapter), to find these values based
	 * on user desired CPU Speed.
	 *
	 * Tests show values match faster with m decrementing than m incrementing
	 */
	for (int m = 511; m >= 6; m--)
	{
		for (int n = 0; n < 32; n++)
		{
			const unsigned int FccoMinKhz = 275 * 1000;
			const unsigned int FccoMaxKhz = 550 * 1000;
			const unsigned int FccoKhz = (2 * (m + 1) * inputFreqKhz) / (n + 1);

			// If Fcco is in range, then check if we can generate desired CPU clock out of this
			if (FccoKhz >= FccoMinKhz && FccoKhz <= FccoMaxKhz)
			{
			    // User Manual, Page 55/840 : Value of 0 and 1 is not allowed
				for (int cpudiv = 3; cpudiv < 256; cpudiv++)
				{
					const unsigned int cpuClockKhz = FccoKhz / (cpudiv + 1);

					/**
					 * Save the parameters if at least we got a speed that is acceptable by CPU
					 * Since cpudiv counts up, we will always get the slowest speed possible
					 */
					const unsigned int maxCpuSpeedKhz = 100 * 1000;
					if (cpuClockKhz <= maxCpuSpeedKhz)
					{
						*pPLL_M = m;
						*pPLL_N = n;
						*pCPU_D = cpudiv;

						// If we absolutely got the speed we wanted, return from this function
						if (cpuClockKhz == desiredCpuSpeedKhz)
						{
						    // Break all the loops and return
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

unsigned int getCpuClock()
{
	unsigned clock = 0;

	/* Determine clock frequency according to clock register values             */
	if (((LPC_SC->PLL0STAT >> 24) & 3) == 3)
	{ /* If PLL0 enabled and connected */
	    switch (LPC_SC->CLKSRCSEL & 0x03)
	    {
	        case 0: /* Int. RC oscillator => PLL0    */
	        case 3: /* Reserved, default to Int. RC  */
	            clock = (INTERNAL_CLOCK
	                    * ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))
	                    / (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)
	                    / ((LPC_SC->CCLKCFG & 0xFF) + 1));
	            break;
	        case 1: /* Main oscillator => PLL0       */
	            clock = (EXTERNAL_CLOCK
	                    * ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))
	                    / (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)
	                    / ((LPC_SC->CCLKCFG & 0xFF) + 1));
	            break;
	        case 2: /* RTC oscillator => PLL0        */
	            clock = (RTC_CLOCK
	                    * ((2 * ((LPC_SC->PLL0STAT & 0x7FFF) + 1)))
	                    / (((LPC_SC->PLL0STAT >> 16) & 0xFF) + 1)
	                    / ((LPC_SC->CCLKCFG & 0xFF) + 1));
	            break;
	    }
	}
	else
	{
	    switch (LPC_SC->CLKSRCSEL & 0x03)
	    {
	        case 0: /* Int. RC oscillator => PLL0    */
	        case 3: /* Reserved, default to Int. RC  */
	            clock = INTERNAL_CLOCK / ((LPC_SC->CCLKCFG & 0xFF) + 1);
	            break;
	        case 1: /* Main oscillator => PLL0       */
	            clock = EXTERNAL_CLOCK / ((LPC_SC->CCLKCFG & 0xFF) + 1);
	            break;
	        case 2: /* RTC oscillator => PLL0        */
	            clock = RTC_CLOCK / ((LPC_SC->CCLKCFG & 0xFF) + 1);
	            break;
	    }
	}

	return clock;
}
