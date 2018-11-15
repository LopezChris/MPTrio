#include "sd.h"
#include "disk_defines.h"
#include "signal_inits.h"
#include "../timer0.h"



static inline int sdcard_get_timer_ms()
{
    return timer0_getMsValue();
}
static inline char rcvr_spi()
{
    return spi1_exchange_byte(0xFF);
}
static inline char xmit_spi(char out)
{
    return spi1_exchange_byte(out);
}

#define FCLK_SLOW()     do { LPC_SSP1->CPSR = 100; } while (0)
#define FCLK_FAST()     do { LPC_SSP1->CPSR = 4;   } while (0)
#define SD_SELECT()     board_io_sd_cs()
#define SD_DESELECT()   board_io_sd_ds()



/* Definitions for MMC/SDC command */
#define CMD0            (0x40+0)        /* GO_IDLE_STATE */
#define CMD1            (0x40+1)        /* SEND_OP_COND (MMC) */
#define ACMD41          (0xC0+41)       /* SEND_OP_COND (SDC) */
#define CMD8            (0x40+8)        /* SEND_IF_COND */
#define CMD9            (0x40+9)        /* SEND_CSD */
#define CMD10           (0x40+10)       /* SEND_CID */
#define CMD12           (0x40+12)       /* STOP_TRANSMISSION */
#define ACMD13          (0xC0+13)       /* SD_STATUS (SDC) */
#define CMD16           (0x40+16)       /* SET_BLOCKLEN */
#define CMD17           (0x40+17)       /* READ_SINGLE_BLOCK */
#define CMD18           (0x40+18)       /* READ_MULTIPLE_BLOCK */
#define CMD23           (0x40+23)       /* SET_BLOCK_COUNT (MMC) */
#define ACMD23          (0xC0+23)       /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24           (0x40+24)       /* WRITE_BLOCK */
#define CMD25           (0x40+25)       /* WRITE_MULTIPLE_BLOCK */
#define CMD55           (0x40+55)       /* APP_CMD */
#define CMD58           (0x40+58)       /* READ_OCR */

/* MMC/SDC command */
#define MMC_GET_TYPE        10
#define MMC_GET_CSD         11
#define MMC_GET_CID         12
#define MMC_GET_OCR         13
#define MMC_GET_SDSTAT      14
/* ATA/CF command */
#define ATA_GET_REV         20
#define ATA_GET_MODEL       21
#define ATA_GET_SN          22

/* Card type flags (CardType) */
#define CT_MMC              0x01
#define CT_SD1              0x02
#define CT_SD2              0x04
#define CT_SDC              (CT_SD1|CT_SD2)
#define CT_BLOCK            0x08

BYTE m_card_type; /* Card type flags */



BYTE wait_ready(void)
{
    BYTE res;

    /* Wait for ready in timeout of 500ms */
    UINT timeout = sdcard_get_timer_ms() + 500;
    rcvr_spi();

    do
    {
        res = rcvr_spi();
    } while ((res != 0xFF) && sdcard_get_timer_ms() < timeout);

    return res;
}

BOOL rcvr_datablock(BYTE *buff, /* Data buffer to store received data */
UINT btr /* Byte count (must be multiple of 4) */
)
{
    BYTE token;
    UINT timeout = sdcard_get_timer_ms() + 100;
    do
    { /* Wait for data packet in timeout of 100ms */
        token = rcvr_spi();
    } while ((token == 0xFF) && sdcard_get_timer_ms() < timeout);

    if (token != 0xFE)
        return FALSE; /* If not valid data token, return with error */

    do
    {
        *buff++ = rcvr_spi();
        *buff++ = rcvr_spi();
        *buff++ = rcvr_spi();
        *buff++ = rcvr_spi();
    } while (btr -= 4);

    rcvr_spi(); /* Discard CRC */
    rcvr_spi();

    return TRUE; /* Return with success */
}

BYTE send_cmd(BYTE cmd, /* Command byte */
DWORD arg /* Argument */
)
{
    BYTE n, res;

    if (cmd & 0x80)
    { /* ACMD<n> is the command sequense of CMD55-CMD<n> */
        cmd &= 0x7F;
        res = send_cmd(CMD55, 0);
        if (res > 1)
            return res;
    }

    /* Select the card and wait for ready */
    SD_DESELECT();
    SD_SELECT(); // Wait for card ready

    if (wait_ready() != 0xFF)
    {
        return 0xFF;
    }

    /* Send command packet */xmit_spi(cmd);
    /* Start + Command index */
    xmit_spi((BYTE)(arg >> 24));
    /* Argument[31..24] */
    xmit_spi((BYTE)(arg >> 16));
    /* Argument[23..16] */
    xmit_spi((BYTE)(arg >> 8));
    /* Argument[15..8] */
    xmit_spi((BYTE)arg);
    /* Argument[7..0] */
    n = 0x01; /* Dummy CRC + Stop */
    if (cmd == CMD0)
        n = 0x95; /* Valid CRC for CMD0(0) */
    if (cmd == CMD8)
        n = 0x87; /* Valid CRC for CMD8(0x1AA) */
    xmit_spi(n);

    /* Receive command response */
    if (cmd == CMD12)
        rcvr_spi(); /* Skip a stuff byte when stop reading */
    n = 10; /* Wait for a valid response in timeout of 10 attempts */

    do
    {
        res = rcvr_spi();
    } while ((res & 0x80) && --n);

    return res; /* Return with the response value */
}

DSTATUS sd_initialize()
{
    BYTE n, cmd, ty, ocr[4];

    FCLK_SLOW();
    for (n = 10; n; n--) {
        rcvr_spi(); /* 80 dummy clocks */
    }

    SD_SELECT();
    ty = 0;
    if (send_cmd(CMD0, 0) == 1)
    { /* Enter Idle state */

        UINT timeout = sdcard_get_timer_ms() + 1000;
        if (send_cmd(CMD8, 0x1AA) == 1)
        { /* SDHC */
            for (n = 0; n < 4; n++)
                ocr[n] = rcvr_spi(); /* Get trailing return value of R7 resp */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA)
            { /* The card can work at vdd range of 2.7-3.6V */
                while (sdcard_get_timer_ms() < timeout
                        && send_cmd(ACMD41, 1UL << 30))
                    ; /* Wait for leaving idle state (ACMD41 with HCS bit) */

                if (sdcard_get_timer_ms() < timeout && send_cmd(CMD58, 0) == 0)
                { /* Check CCS bit in the OCR */
                    for (n = 0; n < 4; n++)
                        ocr[n] = rcvr_spi();
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        }
        else
        { /* SDSC or MMC */
            if (send_cmd(ACMD41, 0) <= 1)
            {
                ty = CT_SD1;
                cmd = ACMD41; /* SDSC */
            }
            else
            {
                ty = CT_MMC;
                cmd = CMD1; /* MMC */
            }
            while (sdcard_get_timer_ms() < timeout && send_cmd(cmd, 0))
                ; /* Wait for leaving idle state */

            if ((sdcard_get_timer_ms() < timeout) || send_cmd(CMD16, 512) != 0) /* Set R/W block length to 512 */
            {
            }
        }
    }
    m_card_type = ty;
    SD_DESELECT();

    if (ty) {
        FCLK_FAST();
        return RES_OK;
    }

    return RES_NOTRDY;
}

DSTATUS sd_status()
{
    return RES_OK;
}

DRESULT sd_read(BYTE *buff, /* Pointer to the data buffer to store read data */
DWORD sector, /* Start sector number (LBA) */
BYTE count /* Sector count (1..255) */
)
{
    if (!count)
        return RES_PARERR;

    SD_SELECT();
    if (!(m_card_type & CT_BLOCK))
        sector *= 512; /* Convert to byte address if needed */

    if (count == 1)
    { /* Single block read */
        if ((send_cmd(CMD17, sector) == 0) /* READ_SINGLE_BLOCK */
        && rcvr_datablock(buff, 512))
            count = 0;
    }
    else
    { /* Multiple block read */
        if (send_cmd(CMD18, sector) == 0)
        { /* READ_MULTIPLE_BLOCK */
            do
            {
                if (!rcvr_datablock(buff, 512))
                    break;
                buff += 512;
            } while (--count);
            send_cmd(CMD12, 0); /* STOP_TRANSMISSION */
        }
    }
    SD_DESELECT();

    return count ? RES_ERROR : RES_OK;
}
