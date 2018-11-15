/**
 * @file  spi_flash.h
 * @brief SPI Flash Driver complaint with FAT-FS System
 * @ingroup Board IO
 */
#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_
#ifdef __cplusplus
 extern "C" {
#endif

#include "diskioStructs.h" // Used in flash_ioctl()



/**
 * Initializes the Flash Memory
 * This will also check to ensure Flash Page Size is 256 bytes, and if not
 * it will send the command to set it to 256 bytes which is a permanent
 * operation.  The only caveat is that the Flash needs to be power cycled
 * if/when this once in a life operation is performed.
 */
 DSTATUS flash_Initialize();

 // Get status of flash
 DSTATUS flash_status();

/**
 * Reads a sector from the Flash Memory
 * @param pData The pointer to the data to save the read
 * @param sectorNum     The sector number to read with each sector being 512 bytes
 * @param sectorCount   The number of sectors to read
 */
DRESULT flash_ReadSector(unsigned char* pData, int sectorNum, int sectorCount);



#ifdef __cplusplus
 }
#endif
#endif /* SPI_FLASH_H_ */
