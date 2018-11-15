#ifndef SIGNAL_INITS_H_
#define SIGNAL_INITS_H_
#include "LPC17xx.h"



#define BIO_FLASH_CS_P0PIN      6   ///< P0.6
#define BIO_SD_CARD_CS_P1PIN    25  ///< P1.25



static inline void board_io_flash_cs(void)  { LPC_GPIO0->FIOCLR = (1 << BIO_FLASH_CS_P0PIN); }
static inline void board_io_flash_ds(void)  { LPC_GPIO0->FIOSET = (1 << BIO_FLASH_CS_P0PIN); }
static inline char board_io_sd_cs(void)     { LPC_GPIO1->FIOCLR = (1 << BIO_SD_CARD_CS_P1PIN); return 1; }
static inline char board_io_sd_ds(void)     { LPC_GPIO1->FIOSET = (1 << BIO_SD_CARD_CS_P1PIN); return 0; }



static inline void spi_cs_init(void)
{
    LPC_SC->PCONP |= (1 << 10);     // SPI1 Power Enable
    LPC_SC->PCLKSEL0 &= ~(3 << 20); // Clear clock Bits
    LPC_SC->PCLKSEL0 |=  (1 << 20); // CLK / 1

    LPC_SSP1->CR0 = 7;          // 8-bit mode
    LPC_SSP1->CR1 = (1 << 1);   // Enable SSP as Master
    LPC_SSP1->CPSR = 4;         // Clock divider

    /* Initialize SPI1 on P0.7, P0.8, and P0.9 */
    LPC_PINCON->PINSEL0 &= ~(0x3F << 14);
    LPC_PINCON->PINSEL0 |=  (0x2A << 14);

    /* Initialize Flash CS*/
    board_io_flash_ds();
    LPC_GPIO0->FIODIR |= (1 << BIO_FLASH_CS_P0PIN);

    /* Initialize SD card CS */
    board_io_sd_ds();
    LPC_GPIO1->FIODIR |= (1 << BIO_SD_CARD_CS_P1PIN);
}
static inline void spi_cs_deinit(void)
{
    // Deinit SPI
    LPC_SC->PCLKSEL0 &= ~(3 << 20);
    LPC_SSP1->CR0 = 0;
    LPC_SSP1->CR1 = 0;
    LPC_SSP1->CPSR = 0;

    // Deinit SPI signals
    LPC_PINCON->PINSEL0 &= ~(0x3F << 14);

    // Deinit CS signals
    LPC_GPIO0->FIODIR &= ~(1 << BIO_FLASH_CS_P0PIN);
    LPC_GPIO1->FIODIR &= ~(1 << BIO_SD_CARD_CS_P1PIN);

    // CS asserts actually set FIODIR value to 0 just like we need it
    board_io_sd_cs();
    board_io_flash_cs();
}

static inline char spi1_exchange_byte(char out)
{
    LPC_SSP1->DR = out;
    while(LPC_SSP1->SR & (1 << 4)); // Wait until SSP is busy
    return LPC_SSP1->DR;
}

#endif /* SIGNAL_INITS_H_ */
