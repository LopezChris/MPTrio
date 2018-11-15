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
#include "fat/ff.h"
#include "fat/diskio.h"
#include "fat/signal_inits.h"

#include "IAP.h"
#include "LPC17xx.h"
#include "std_lib.h"
#include "system_LPC17xx.h"
#include "uart0.h"



/**
 * This hyperload bootloader is built with special settings.
 * You cannot use any C library function like memset() or memcpy()
 * This is done to minimize the code-size.
 *
 * There are three ways to program :
 *      1) Use Hyperload
 *      2) Save a file to SPI Flash, then use code at "prog_handlers.cpp"
 *          which will write special registers that tell us which file we
 *          need to program when we boot up
 *      3) When special buttons are pressed and a pre-defined filename is
 *          present, we will flash this file to the CPU
 *
 *  Method 2 and 3 require binary file (not hex file)
 */

#define UCT                     500                         ///< UART Char timeout
#define BUFFER_SIZE		        4096                        ///< Must be 256, 512, 1024 or 4096.  256 works better if you want to use xbee

#define TOTAL_FLASH_MEMORY      (512 * 1024)                ///< Flash memory space
#define TOTAL_SECTORS	        30                          ///< 0-29
#define USER_APP_SECT_NUM       (64/4)                      ///< First 64K (all sectors of 4k) reserved by this bootloader
#define USER_APP_START_ADDR     (USER_APP_SECT_NUM * 4096)  ///< Start address of user's application
#define USER_LAST_SECTOR        (TOTAL_SECTORS-1)           ///< Last sector number of flash memory

#define META_DATA_SECTOR        (USER_APP_SECT_NUM-1)       ///< The sector that we save the meta-data to
#define META_DATA_SECTOR_ADDR   (META_DATA_SECTOR * 4096)   ///<  Address of meta data

#define HYPERLOAD_OK				'!'		///< Hyperload return character for SUCCESS
#define HYPERLOAD_CHECKSUM_ERROR	'@'		///< Hyperload return character for receive checksum error
#define HYPERLOAD_VERIFY_ERROR		'^'		///< Hyperload return character for flash verify checksum error
#define HYPERLOAD_EXITING_TO_APP	'*'		///< Hyperload return character when bootloader quits to start application code

/// If this is non-zero, then we can support file program from SPI flash or SD Card
#define SUPPORT_FILE_PROGRAM        1

/**
 * @{ Defines that triggers file program upon button-press
 * We will try to flash a binary file if it is present on SPI flash or SD card
 */
#define FILE_FLASH_PORT_PIN         LPC_GPIO1->FIOPIN       ///< Input register of buttons
#define FILE_FLASH_SW_MASK          ((1 << 9) | (1 << 10))  ///< Pins of buttons
#define FILE_FLASH_FILENAME1        "0:prog.bin"            ///< 0: for SPI FLASH
#define FILE_FLASH_FILENAME2        "1:prog.bin"            ///< 1: for SD Card
/** @} */

/** @{ Special registers
 * These should be consistent with the bootloader such that it can see
 * our data, and program the flash memory from spi flash.
 */
#define SP_REG_IMG_SIG          ((uint32_t)0xDEADD00D)     ///< SP_REG_IMG_PROG is set to this value to program flash
#define SP_REG_IMG_PROG         LPC_RTC->GPREG0 ///< Image program "command" is stored here
#define SP_REG_IMG_FILENAME     LPC_RTC->GPREG1 ///< Null terminated image name up to 12 chars (we use GPREG2 and GPREG3 too)
/** @} */



/// Gets Sector number from a given address
/// @param address Flash Address 0-512K
/// @returns 0-27 for sector number.  -1 if address out of range.
long iap_get_sector(unsigned int address);

/// Gets BUFFER_SIZE # of bytes and writes data at pageNumber*BUFFER_SIZE
/// @param pageNumber  This is block number given that each block is of size BUFFER_SIZE
/// @returns HYPERLOAD_OK or HYPERLOAD_CHECKSUM_ERROR HYPERLOAD_VERIFY_ERROR
unsigned char write_flash_page_from_uart(unsigned int blockNumber);

/// Write a file from file system to cpu flash
int write_flash_from_file(const char *filename);

/// Write the flash file from UART
void write_flash_from_uart(void);

/// Execute application placed after the bootloader
void execute_user_app();

/// Write metadata about the programming info
void write_program_counter(int programModified);

/// Returns the checksum of the entire flash with exception of this bootloader's area
static inline uint32_t get_user_app_checksum(void)
{
    uint32_t checksum = 0;
    uint32_t *pFlash = 0;

    for (pFlash = (uint32_t*) USER_APP_START_ADDR;
        (uint32_t)pFlash < TOTAL_FLASH_MEMORY;
        pFlash++)
    {
        checksum += *pFlash;
    }

    return checksum;
}

typedef struct {
    /*    - Data-           -Padding-  */
    uint32_t sec  : 6;      uint32_t : 2;
    uint32_t min  : 6;      uint32_t : 2;
    uint32_t hour : 5;      uint32_t : 3;
    uint32_t dow  : 3;      uint32_t : 5;

    uint32_t day  : 5;      uint32_t : 3;
    uint32_t month: 4;      uint32_t : 4;
    uint32_t year :12;      uint32_t : 4;

    uint32_t doy  :12;      uint32_t : 20;
} __attribute__((packed)) rtc_t ;

/**
 * This structure must be 256, 512, 1024 or 4096 bytes
 */
typedef struct {
    uint32_t prog_counter;         ///< 1: 4 bytes
    uint32_t prog_modify_counter;  ///< 2: 4 bytes
    uint32_t prog_size_max;        ///< 3: 4 bytes
    uint32_t prog_size_min;        ///< 4: 4 bytes
    uint32_t prog_time_ms;         ///< 5: 4 bytes
    rtc_t last_programming_time;   ///< 12 bytes

    uint8_t padding[256 - sizeof(rtc_t) - (5 * sizeof(uint32_t))];
} metadata_t;

static uint32_t gProgrammedSize = 0;
static uint32_t gProgtimeMs = 0;

int main(void)
{
	system_init();
	timer0_init();
	uart_init(38400);

	uart_getchar(10);	// Flushes UART and also provides "free" delay
	uart_putchar(0xff); // Send 0xFF for Hyperload to sync with our baud rate

#if SUPPORT_FILE_PROGRAM
	/* Test the conditions that trigger special programming instructions */
	const uint32_t flashDueToSpecialInstruction = !!(SP_REG_IMG_SIG == SP_REG_IMG_PROG);
	const uint32_t flashDueToButtonPresses = FILE_FLASH_SW_MASK == (FILE_FLASH_PORT_PIN & FILE_FLASH_SW_MASK);

	if (flashDueToSpecialInstruction || flashDueToButtonPresses)
	{
	    spi_cs_init();

	    /* Try to mount both SD card and SPI flash so we can open file
	     * from both file-systems
	     */
        FATFS spiFlash;
	    FATFS sdCard;
        f_mount(0, &spiFlash);
        f_mount(1, &sdCard);

	    uart_put("\nTry to flash from '");
	    if (flashDueToSpecialInstruction)
	    {
	        SP_REG_IMG_PROG = 0;
	        char *filename = (char *)&(SP_REG_IMG_FILENAME);
	        uart_put(filename);
	        uart_puts("' (register instructions).");


            const uint32_t prevChecksum = get_user_app_checksum();
	        if (write_flash_from_file(filename)) {
	            write_program_counter(prevChecksum !=  get_user_app_checksum());
	        }
	    }
	    else if (flashDueToButtonPresses)
	    {
	        uart_put(FILE_FLASH_FILENAME1);
	        uart_put("' OR '");
	        uart_put(FILE_FLASH_FILENAME2);
	        uart_puts("' (triggered by buttons)");

	        const uint32_t prevChecksum = get_user_app_checksum();
	        if (write_flash_from_file(FILE_FLASH_FILENAME1)) {
	            write_program_counter(prevChecksum !=  get_user_app_checksum());
	        }
	        else if (write_flash_from_file(FILE_FLASH_FILENAME2)) {
	            write_program_counter(prevChecksum !=  get_user_app_checksum());
	        }
	    }

	    spi_cs_deinit();
	}
	// Hyperload will send 0x55 to notify that it is alive!
	else
#endif
	if (0x55 == uart_getchar(500))
	{
	    const uint32_t prevChecksum = get_user_app_checksum();

	    gProgtimeMs = timer0_getMsValue();
		write_flash_from_uart();
		gProgtimeMs = timer0_getMsValue() - gProgtimeMs;

		write_program_counter((prevChecksum != get_user_app_checksum()));
	}

	// Delay a bit for any previous char to go out, then boot application
	delay_ms(5);
	execute_user_app();

	while(1);
	return 0;
}

unsigned char write_flash_page_from_uart(unsigned int blockNumber)
{
	int i = 0;
	unsigned char CheckSum = 0;
	unsigned char PageBuffer[BUFFER_SIZE];
	uint32_t error[4] = {0};

	// Get FLASH data
	for (i = 0; i < BUFFER_SIZE; i++) {
		PageBuffer[i] = uart_getchar(UCT);
		CheckSum += PageBuffer[i];
	}

	// Compare checksum
	if (CheckSum != uart_getchar(UCT)) {
	    uart_flush_rx();
		return HYPERLOAD_CHECKSUM_ERROR;
	}

	// Write FLASH:
	long address = (blockNumber * BUFFER_SIZE) + USER_APP_START_ADDR;
	long sectorNumber = iap_get_sector(address);

#if 0
	/* 12/2013 : Hyperload now modified to send data from binary file flash start
	 * rather than from address zero, so we dont' need to do this.
	 */
	/**
	 * Do not write any sectors until the user's application sectors
	 */
	if(sectorNumber < USER_APP_SECT_NUM) {
		return HYPERLOAD_OK;
	}
#endif

	// Make sure we don't over-write this secondary bootloader
	if (sectorNumber != -1)
	{
		u32IAP_PrepareSectors(sectorNumber, sectorNumber);
		gProgrammedSize += BUFFER_SIZE;

		error[0] = error[1] = 0;
		u32IAP_CopyRAMToFlash(address, (uint32_t)PageBuffer, BUFFER_SIZE);
		if (IAP_STA_CMD_SUCCESS == u32IAP_Compare(address, (uint32_t) PageBuffer, BUFFER_SIZE, error)) {
			return HYPERLOAD_OK;
		}
		else {
		    return HYPERLOAD_VERIFY_ERROR;
		}
	}

	uart_flush_rx();
	return HYPERLOAD_VERIFY_ERROR;
}

long iap_get_sector(unsigned int address)
{
	//const unsigned int USER_FIRST_SECTOR = 1;
	long i = 0;

	const unsigned int iap_sector_addr[] = {
			0x00000000,

			0x00001000, 0x00002000, 0x00003000, 0x00004000, 0x00005000,
			0x00006000, 0x00007000, 0x00008000, 0x00009000, 0x0000A000,
			0x0000B000, 0x0000C000, 0x0000D000, 0x0000E000, 0x0000F000,

			0x00010000, 0x00018000, 0x00020000, 0x00028000,
			0x00030000, 0x00038000, 0x00040000, 0x00048000,
			0x00050000, 0x00058000, 0x00060000, 0x00068000,
			0x00070000, 0x00078000
	};

	for (i = 0; i <= USER_LAST_SECTOR; i++)
	{
		if(address >= iap_sector_addr[i] && address < iap_sector_addr[i+1])
		{
			return i;
		}
	}

	return -1;
}

int write_flash_from_file(const char *filename)
{
    FIL file;
    int status = 0;

    if (FR_OK != f_open(&file, filename, FA_OPEN_EXISTING | FA_READ))
    {
        uart_put("No file: ");
        uart_puts(filename);
    }
    else
    {
        /* Get the start and end sector we will need to program */
        int cpuFlashAddr = USER_APP_START_ADDR;
        int startSector = iap_get_sector(cpuFlashAddr);
        int endSector = iap_get_sector(cpuFlashAddr + file.fsize);

        /* Erase all CPU flash sections we will need */
        if (IAP_STA_CMD_SUCCESS != u32IAP_PrepareSectors(startSector, endSector)) {
            uart_puts("Error preparing sectors");
            f_close(&file);
        }
        else if (IAP_STA_CMD_SUCCESS != u32IAP_EraseSectors(startSector, endSector)) {
            uart_puts("Error erasing sectors");
            f_close(&file);
        }
        else {
            /* We have erased flash, so count this as a programming process */
            status = 1;

            const int maxSizeWeCanProgram = 4096;
            char buffer[maxSizeWeCanProgram];
            unsigned int bytesRead = 0;

            gProgtimeMs = timer0_getMsValue();
            while (FR_OK == f_read(&file, buffer, sizeof(buffer), &bytesRead) &&
                    bytesRead > 0)
            {
                gProgrammedSize += bytesRead;

                int sectorNumOfAddr = iap_get_sector(cpuFlashAddr);
                u32IAP_PrepareSectors(sectorNumOfAddr, sectorNumOfAddr);
                u32IAP_CopyRAMToFlash(cpuFlashAddr, (uint32_t)buffer, sizeof(buffer));

                uint32_t error[4] = {0};
                if (IAP_STA_CMD_SUCCESS !=
                        u32IAP_Compare(cpuFlashAddr, (uint32_t)buffer, sizeof(buffer), error))
                {
                    uart_puts("Error verifying sectors");
                    break;
                }

                cpuFlashAddr += sizeof(buffer);
            }
            gProgtimeMs = timer0_getMsValue() - gProgtimeMs;

            f_close(&file);
            uart_put("File ");
            uart_put(filename);
            uart_puts(" successfully programmed!");
        }
    }

    return status;
}

void write_flash_from_uart(void)
{
    // Notify Hyperload that we're alive too!
    uart_putchar(0xAA);

    char baudRateControlWord = uart_getchar(500); // Get new baud rate control word
    uart_putchar(baudRateControlWord);            // Echo it back to verify

    /**
     * Erase all sectors
     * This will also let Hyperload switch to new baud rate
     */
    u32IAP_PrepareSectors(USER_APP_SECT_NUM, USER_LAST_SECTOR);
    u32IAP_EraseSectors(USER_APP_SECT_NUM, USER_LAST_SECTOR);

    /**
     * Hyperload Frequency should be set to 48,000,000 for this to work correctly
     * It calculates the baud rate by: 48Mhz/(16*BAUD) - 1 = CW (control word)
     * So we solve for BAUD:  BAUD = (48/(CW + 1))/16
     */
    long baudRate = (48000000UL/(baudRateControlWord+1))/16;
    uart_init(baudRate);

    // Send our CPU information along with data parameters:
    // Name:Blocksize:(Bootsize/2):FlashSize
#if(256 == BUFFER_SIZE)
    uart_puts("$LPC1758:256:32768:512");
#endif
#if(512 == BUFFER_SIZE)
    uart_puts("$LPC1758:512:32768:512");
#endif
#if(1024 == BUFFER_SIZE)
    uart_puts("$LPC1758:1024:32768:512");
#endif
#if(4096 == BUFFER_SIZE)
    uart_puts("$LPC1758:4096:32768:512");
#endif

    uart_putchar(HYPERLOAD_OK); // Now start programming...
    while(1)
    {
        int blockNumber = (((int) uart_getchar(UCT) & 0xff) << 8) | (uart_getchar(UCT) & 0xff);

        if(0xffff == blockNumber) {
            break;
        }
        else {
            uart_putchar(write_flash_page_from_uart(blockNumber));
        }
    }

    uart_putchar(HYPERLOAD_EXITING_TO_APP);
}

void __set_PSP(uint32_t topOfProcStack) __attribute__( ( naked ) );
void __set_PSP(uint32_t topOfProcStack)
{
  __ASM volatile ("MSR psp, %0\n\t"
                  "BX  lr     \n\t" : : "r" (topOfProcStack) );
}
void __set_MSP(uint32_t topOfMainStack) __attribute__( ( naked ) );
void __set_MSP(uint32_t topOfMainStack)
{
  __ASM volatile ("MSR msp, %0\n\t"
                  "BX  lr     \n\t" : : "r" (topOfMainStack) );
}

void execute_user_app()
{
    // Re-map Interrupt vectors to the user application
    SCB->VTOR = (USER_APP_START_ADDR & 0x1FFFFF80);

    // Application code's RESET handler starts at Application Code + 4
    unsigned *pAppCodeStart = (unsigned *)(USER_APP_START_ADDR + 4);

    // Disable the PLL0
    do {
        LPC_SC->PLL0CON &= ~(1 << 1);
        LPC_SC->PLL0FEED = 0xAA;
        LPC_SC->PLL0FEED = 0x55;

        LPC_SC->PLL0CON &= ~(1 << 0);
        LPC_SC->PLL0FEED = 0xAA;
        LPC_SC->PLL0FEED = 0x55;

        LPC_SC->CLKSRCSEL = 0;   // Select IRC as clock source
        LPC_SC->PLL0CFG = 0;     // M=1, N=1 (disregarded anyway after PLL is disabled)
        LPC_SC->PLL0FEED = 0xAA;
        LPC_SC->PLL0FEED = 0x55;
        LPC_SC->CCLKCFG = 0;     // Divider = 1 when PLL is not used
    } while (0);

    // Disable all resources we used
    uart_deinit();
    timer0_deinit();

    // Get the function pointer of user application
    void (*user_code_entry)(void);
    user_code_entry = (void *) *pAppCodeStart;

    // Set user application stack size and run it
    __set_PSP((uint32_t)(pAppCodeStart-4));
    __set_MSP((uint32_t)(pAppCodeStart-4));
    user_code_entry();
}

void write_program_counter(int programModified)
{
    const metadata_t *pOldMetadata = (metadata_t*) META_DATA_SECTOR_ADDR;
    metadata_t newMetadata;

    /* Copy old metadata, and increment the counters */
    memcpy(&newMetadata, pOldMetadata, sizeof(newMetadata));

    /* Reset min/max if they were never programmed */
    if (UINT32_MAX == newMetadata.prog_size_max) {
        newMetadata.prog_size_max = 0;
    }
    if (0 == newMetadata.prog_size_min) {
        newMetadata.prog_size_min = UINT32_MAX;
    }

    /* Update the new meta-data */
    newMetadata.prog_time_ms = gProgtimeMs;
    newMetadata.prog_counter++;
    if (programModified) {
        newMetadata.prog_modify_counter++;
    }

    /* Update min/max counters */
    if (gProgrammedSize > newMetadata.prog_size_max) {
        newMetadata.prog_size_max = gProgrammedSize;
    }
    if (gProgrammedSize < newMetadata.prog_size_min) {
        newMetadata.prog_size_min = gProgrammedSize;
    }

    /* RTC should be default enabled on power-up */
    if (LPC_SC->PCONP & (1 << 9)) {
        if (LPC_RTC->CCR) {
            newMetadata.last_programming_time = *(rtc_t*) (&LPC_RTC->CTIME0);
        }
    }

    /* Erase the meta-data sector */
    u32IAP_PrepareSectors(META_DATA_SECTOR, META_DATA_SECTOR);
    u32IAP_EraseSectors(META_DATA_SECTOR, META_DATA_SECTOR);

    /* Write new meta-data */
    u32IAP_PrepareSectors(META_DATA_SECTOR, META_DATA_SECTOR);
    u32IAP_CopyRAMToFlash(META_DATA_SECTOR_ADDR, (uint32_t)&newMetadata, sizeof(newMetadata));
}
