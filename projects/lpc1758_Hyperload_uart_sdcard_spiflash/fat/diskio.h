#ifndef DISK_IO_H__
#define DISK_IO_H__
#include "spi_flash.h"
#include "sd.h"
#include "diskioStructs.h"
#include "disk_defines.h"



static inline DSTATUS disk_initialize(BYTE drv)
{
    return (0 == drv) ? flash_Initialize() : sd_initialize();
}
static inline DSTATUS disk_status(BYTE drv)
{
    return (0 == drv) ? flash_status() : sd_status();
}

static inline  DRESULT disk_read (BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
    return (0 == drv) ? flash_ReadSector(buff, sector, count) : sd_read(buff, sector, count);
}

/* Write functions not needed */
#if 0
static inline DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, BYTE count)
{

}

static inline DRESULT disk_ioctl(BYTE drv, BYTE ctrl,void *buff)
{

}
#endif



#endif
