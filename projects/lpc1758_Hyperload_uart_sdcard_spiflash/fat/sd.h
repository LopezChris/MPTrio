#ifndef _SDIO
#ifdef __cplusplus
extern "C" {
#endif
#include "diskioStructs.h"



DSTATUS sd_initialize();    ///< Initializes the SD Card for SPI Mode, called automatically by disk_initialize()
DSTATUS sd_status();        ///< Returns status of card (if its been initialized or not)
DRESULT sd_read (BYTE *buff, DWORD sector, BYTE count);	        ///< Reads a sector from the SD Card



#ifdef __cplusplus
}
#endif
#define _SDIO
#endif
