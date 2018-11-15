#ifndef DISIODEFINES_H_
#define DISIODEFINES_H_

#include "spi1.h"
#include "LPC17xx.h"


/**
 * SD Card Signal Mapping:
 *
 * Write Protect (WP)   : P0.6
 * Card Detect   (CD)   : P0.29
 * Chip Select   (CS)   : P0.22
 */
#define SPI_SD_CS 			(1 << 22)
#define SELECT()        	LPC_GPIO0->FIOCLR = SPI_SD_CS
#define	DESELECT()      	LPC_GPIO0->FIOSET = SPI_SD_CS
#define SET_CSPIN_OUT() 	LPC_PINCON->PINSEL1 &= ~(3 << 12);  \
							LPC_GPIO0->FIODIR |= SPI_SD_CS;     \
							DESELECT()

#define SOCKPORT        	LPC_GPIO0->FIOPIN      /* Socket contact port	*/
#define SOCKWP				(1 << 6)
#define SOCKINS				(1 << 29)
#define CARD_SIGNAL_INIT()	LPC_PINCON->PINSEL0 &= ~(3 << 12);  \
							LPC_PINCON->PINSEL1 &= ~(3 << 26);  \
							LPC_GPIO0->FIODIR &= ~SOCKWP;       \
							LPC_GPIO0->FIODIR &= ~SOCKINS;

#define	FCLK_SLOW()			spi1_SetMaxClockMhz(1)  /* Set slow SPI clock (100k-400k) */
#define	FCLK_FAST()			spi1_SetMaxClockMhz(24) /* Set fast SPI clock (depends on the CSD) */

#define xmit_spi(dat)		spi1_ExchangeByte(dat)
#define rcvr_spi()			spi1_ExchangeByte(0xff)


#endif /* DISIODEFINES_H_ */
