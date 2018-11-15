#include "LPC17xx.h"
#include "spi_flash.h"
#include "signal_inits.h"



#define FLASH_SECTOR_SIZE       512

/**
 * This union structure is used to access the bytes of an integer
 */
typedef union
{
   /// Full Address that overlaps with the structure below
   unsigned int fullAddr;

   struct {
       unsigned char B07_B00;
       unsigned char B15_B08;
       unsigned char B23_B16;
       unsigned char B31_B24;
   }byte;
}FlashAddressType;

/**
 * OPCODES of this Flash Memory
 */
typedef enum {
    opCode_StatusReg = 0xD7,
    opCode_GetSignature = 0x9F,
    opCode_readContinousMemRead = 0xE8,

    opCode_PageErase = 0x81,
    opCode_progThruBuff1 = 0x82,
    opCode_Buffer1Write = 0x84,
    opCode_buffer1ToMemWithoutBuiltInErase = 0x88
}FlashOpCodeType;

DSTATUS flash_Initialize()
{
    return RES_OK;
}

// Get status of flash
DSTATUS flash_status()
{
    return RES_OK;
}

DRESULT flash_ReadSector(unsigned char* pData, int sectorNum, int sectorCount)
{
    FlashAddressType addr;
    addr.fullAddr = (sectorNum * FLASH_SECTOR_SIZE);
    const int totalBytes = (sectorCount * FLASH_SECTOR_SIZE);
    int i = 0;

    // Assume flash is always ready (since we don't program it)
    board_io_flash_cs();
    {
        spi1_exchange_byte(opCode_readContinousMemRead);
        spi1_exchange_byte(addr.byte.B23_B16);
        spi1_exchange_byte(addr.byte.B15_B08);
        spi1_exchange_byte(addr.byte.B07_B00);
        spi1_exchange_byte(0xff);
        spi1_exchange_byte(0xff);
        spi1_exchange_byte(0xff);
        spi1_exchange_byte(0xff);

        for (i = 0; i < totalBytes; i++) {
            *pData = spi1_exchange_byte(*pData);
            pData++;
        }
    }
    board_io_flash_ds();

    return RES_OK;
}
