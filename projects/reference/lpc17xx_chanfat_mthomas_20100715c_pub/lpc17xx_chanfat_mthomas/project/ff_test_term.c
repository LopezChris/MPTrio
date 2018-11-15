/*----------------------------------------------------------------------*/
/* FatFs Module Sample Program / LPC17xx        (C)ChaN, mthomas 2010   */
/*----------------------------------------------------------------------*/

#include "integer.h"
#include "monitor.h"
#include "diskio.h"
#include "ff.h"
#include "rtc_cal.h"

/*---------------------------------------------------------*/
/* Work Area                                               */
/*---------------------------------------------------------*/

DWORD acc_size; /* Working variables (fs command) */
WORD acc_files, acc_dirs;

FILINFO Finfo; /* Working variables (fs/fl command) */
#if _USE_LFN
char Lfname[_MAX_LFN + 1];
#endif

//char Line[256]; /* Console input buffer */
char Line[1024]; /* Console input buffer */

FATFS Fatfs[_DRIVES];		/* File system object for each logical drive */
FIL File1, File2; /* File objects */
DIR Dir; /* Directory object */

BYTE Buff[8*512] /* __attribute__ ((aligned (4)))*/;  /* Data buffer */

volatile UINT Timer; /* Performance timer (1kHz increment) */

/*---------------------------------------------------------*/
/* 1000Hz interval timer (CMT0)                            */
/*---------------------------------------------------------*/

void ff_test_term_timer_callback(void) /* ISR: Requires vect.h */
{
	Timer++;
}

/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */
/*--------------------------------------------------------------------------*/

static FRESULT scan_files( /* Scan directory contents */
		char* path /* Pointer to the path name working buffer */
)
{
	DIR dirs;
	FRESULT res;
	BYTE i;
	char *fn;

	res = f_opendir(&dirs, path); /* Open the directory */
	if (res == FR_OK)
	{
		i = xstrlen(path);
		while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0])
		{ /* Read an item */
			if (_FS_RPATH && Finfo.fname[0] == '.')
				continue; /* Ignore dot entries */
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname; /* Use LFN if available */
#else
			fn = Finfo.fname; /* Always use SFN */
#endif
			if (Finfo.fattrib & AM_DIR)
			{ /* Directory */
				acc_dirs++; /* Increment number of dirs */
				*(path + i) = '/';
				xstrcpy(path + i + 1, fn); /* Scan in the dir */
				res = scan_files(path);
				*(path + i) = '\0';
				if (res != FR_OK)
					break;
			}
			else
			{ /* File */
				/*	xprintf("%s/%s\n", path, fn); */
				acc_files++; /* Increment number of files */
				acc_size += Finfo.fsize; /* Add file size */
			}
		}
	}

	return res;
}

static
void put_rc( /* Put FatFs return code */
FRESULT rc)
{
	const char
			*str =
					"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
						"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
						"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
						"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (i = 0; i != rc && *str; i++)
	{
		while (*str++)
			;
	}
	xprintf("rc=%u FR_%s\n", (UINT) rc, str);
}


/*--------------------------------------------------------------------------*/
/* ff_test_term                                                             */
/*--------------------------------------------------------------------------*/

#if 1


int ff_test_term(void)
{
	char *ptr, *ptr2;
	long p1, p2, p3;
	BYTE res, b1;
	WORD w1;
	UINT s1, s2, cnt, blen = sizeof(Buff);
	static const BYTE ft[] = {0, 12, 16, 32};
	DWORD ofs = 0, sect = 0;
	FATFS *fs;				/* Pointer to file system object */
	RTC_TIME_Type rtc;

	xputs("\nFatFs module test monitor for LPC17xx/SSP\n");
	xputs(_USE_LFN ? "LFN Enabled" : "LFN Disabled");
	xprintf(", Code page: %u\n", _CODE_PAGE);

#if _USE_LFN
	Finfo.lfname = Lfname;
	Finfo.lfsize = sizeof(Lfname);
#endif

	for (;;) {
		xputc('>');
		get_line(Line, sizeof(Line));

		ptr = Line;
		switch (*ptr++) {
		case '?': /* ? - Show usage */
			xputs(
					"md <address> [<len>] - Dump system memory\n"
						"\n"
						"dd [<lba>] - Dump sector\n"
						"di [<drive>] - Initialize disk\n"
						"ds [<drive>] - Show disk status\n"
						"\n"
						"bd <ofs> - Dump working buffer\n"
						"be <ofs> [<data>] ... - Edit working buffer\n"
						"br <lba> [<num>] - Read disk into working buffer\n"
						"bw <lba> [<num>] - Write working buffer into disk\n"
						"bf <val> - Fill working buffer\n"
						"\n"
						"fi - Force initialized the volume\n"
						"fs - Show volume status\n"
						"fl [<path>] - Show a directory\n"
						"fo <mode> <file> - Open a file\n"
						"fc - Close the file\n"
						"fe <ofs> - Move fp\n"
						"fd <len> - Read and dump the file\n"
						"fr <len> - Read the file\n"
						"fw <len> <val> - Write to the file\n"
						"fn <object name> <new name> - Rename an object\n"
						"fu <object name> - Unlink an object\n"
						"fv - Truncate the file at current fp\n"
						"fk <dir name> - Create a directory\n"
						"fa <atrr> <mask> <object name> - Change object attribute\n"
						"ft <year> <month> <day> <hour> <min> <sec> <object name> - Change timestamp of an object\n"
						"fx <src file> <dst file> - Copy a file\n"
						"fg <path> - Change current directory\n"
						"fm <rule> <cluster size> - Create file system\n"
						"fz [<len>] - Change R/W block length for fr/fw/fx command\n"
						"\n");
			break;

		case 'm' :
			switch (*ptr++) {
			case 'd' :	/* md <address> [<count>] - Dump memory */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) p2 = 128;
				for (ptr=(char*)p1; p2 >= 16; ptr += 16, p2 -= 16)
					put_dump((BYTE*)ptr, (UINT)ptr, 16);
				if (p2) put_dump((BYTE*)ptr, (UINT)ptr, p2);
				break;
			}
			break;

		case 'd' :
			switch (*ptr++) {
			case 'd' :	/* dd <phy_drv#> [<lba>] - Dump sector */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) p2 = sect;
				res = disk_read((BYTE)p1, Buff, p2, 1);
				if (res) { xprintf("rc=%d\n", (WORD)res); break; }
				sect = p2 + 1;
				xprintf("Sector:%lu\n", p2);
				for (ptr=(char*)Buff, ofs = 0; ofs < 0x200; ptr+=16, ofs+=16)
					put_dump((BYTE*)ptr, ofs, 16);
				break;

			case 'i' :	/* di <phy_drv#> - Initialize disk */
				if (!xatoi(&ptr, &p1)) break;
				xprintf("rc=%d\n", (WORD)disk_initialize((BYTE)p1));
				break;

			case 's' :	/* ds <phy_drv#> - Show disk status */
				if (!xatoi(&ptr, &p1)) break;
				if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &p2) == RES_OK)
					{ xprintf("Drive size: %lu sectors\n", p2); }
				if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w1) == RES_OK)
					{ xprintf("Sector size: %u\n", w1); }
				if (disk_ioctl((BYTE)p1, GET_BLOCK_SIZE, &p2) == RES_OK)
					{ xprintf("Erase block size: %lu sectors\n", p2); }
				if (disk_ioctl((BYTE)p1, MMC_GET_TYPE, &b1) == RES_OK)
					{ xprintf("MMC/SDC type: %u\n", b1); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CSD, Buff) == RES_OK)
					{ xputs("CSD:\n"); put_dump(Buff, 0, 16); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CID, Buff) == RES_OK)
					{ xputs("CID:\n"); put_dump(Buff, 0, 16); }
				if (disk_ioctl((BYTE)p1, MMC_GET_OCR, Buff) == RES_OK)
					{ xputs("OCR:\n"); put_dump(Buff, 0, 4); }
				if (disk_ioctl((BYTE)p1, MMC_GET_SDSTAT, Buff) == RES_OK) {
					xputs("SD Status:\n");
					for (s1 = 0; s1 < 64; s1 += 16) put_dump(Buff+s1, s1, 16);
				}
				break;
			}
			break;

		case 'b' :
			switch (*ptr++) {
			case 'd' :	/* bd <addr> - Dump R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				for (ptr=(char*)&Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, ptr+=16, ofs+=16)
					put_dump((BYTE*)ptr, ofs, 16);
				break;

			case 'e' :	/* be <addr> [<data>] ... - Edit R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				if (xatoi(&ptr, &p2)) {
					do {
						Buff[p1++] = (BYTE)p2;
					} while (xatoi(&ptr, &p2));
					break;
				}
				for (;;) {
					xprintf("%04X %02X-", (WORD)(p1), (WORD)Buff[p1]);
					get_line(Line, sizeof(Line));
					ptr = Line;
					if (*ptr == '.') break;
					if (*ptr < ' ') { p1++; continue; }
					if (xatoi(&ptr, &p2))
						Buff[p1++] = (BYTE)p2;
					else
						xputs("???\n");
				}
				break;

			case 'r' :	/* br <phy_drv#> <sector> [<n>] - Read disk into R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				xprintf("rc=%u\n", disk_read((BYTE)p1, Buff, p2, p3));
				break;

			case 'w' :	/* bw <phy_drv#> <sector> [<n>] - Write R/W buffer into disk */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				xprintf("rc=%u\n", disk_write((BYTE)p1, Buff, p2, p3));
				break;

			case 'f' :	/* bf <val> - Fill working buffer */
				if (!xatoi(&ptr, &p1)) break;
				xmemset(Buff, (BYTE)p1, sizeof(Buff));
				break;

			}
			break;

		case 'f' :
			switch (*ptr++) {

			case 'i' :	/* fi <log drv#> - Initialize logical drive */
				if (!xatoi(&ptr, &p1)) break;
				put_rc(f_mount((BYTE)p1, &Fatfs[p1]));
				break;

			case 's' :	/* fs - Show logical drive status */
				while (_USE_LFN && *ptr == ' ') ptr++;
				res = f_getfree(ptr, (DWORD*)&p2, &fs);
				if (res) { put_rc(res); break; }
				xprintf("FAT type = FAT%u\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
						"Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
						"FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n\n...",
						ft[fs->fs_type & 3], (DWORD)fs->csize * 512, fs->n_fats,
						fs->n_rootdir, fs->fsize, (DWORD)fs->n_fatent - 2,
						fs->fatbase, fs->dirbase, fs->database
				);
				acc_size = acc_files = acc_dirs = 0;
				res = scan_files(ptr);
				if (res) { put_rc(res); break; }
				xprintf("\r%u files, %lu bytes.\n%u folders.\n"
						"%lu KB total disk space.\n%lu KB available.\n",
						acc_files, acc_size, acc_dirs,
						(fs->n_fatent - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
				);
				break;

			case 'l' :	/* fl [<path>] - Directory listing */
				while (*ptr == ' ') ptr++;
				res = f_opendir(&Dir, ptr);
				if (res) { put_rc(res); break; }
				p1 = s1 = s2 = 0;
				for(;;) {
					res = f_readdir(&Dir, &Finfo);
					if ((res != FR_OK) || !Finfo.fname[0]) break;
					if (Finfo.fattrib & AM_DIR) {
						s2++;
					} else {
						s1++; p1 += Finfo.fsize;
					}
					xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
							(Finfo.fattrib & AM_DIR) ? 'D' : '-',
							(Finfo.fattrib & AM_RDO) ? 'R' : '-',
							(Finfo.fattrib & AM_HID) ? 'H' : '-',
							(Finfo.fattrib & AM_SYS) ? 'S' : '-',
							(Finfo.fattrib & AM_ARC) ? 'A' : '-',
							(Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
							(Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,
							Finfo.fsize, &(Finfo.fname[0]));
#if _USE_LFN
					for (p2 = xstrlen(Finfo.fname); p2 < 14; p2++)
						xputc(' ');
					xprintf("%s\n", Lfname);
#else
					xputc('\n');
#endif
				}
				xprintf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
				if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
					xprintf(", %10lu bytes free\n", p1 * fs->csize * 512);
				break;

			case 'o' :	/* fo <mode> <file> - Open a file */
				if (!xatoi(&ptr, &p1)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_open(&File1, ptr, (BYTE)p1));
				break;

			case 'c' :	/* fc - Close a file */
				put_rc(f_close(&File1));
				break;

			case 'e' :	/* fe - Seek file pointer */
				if (!xatoi(&ptr, &p1)) break;
				res = f_lseek(&File1, p1);
				put_rc(res);
				if (res == FR_OK)
					xprintf("fptr=%lu(0x%lX)\n", File1.fptr, File1.fptr);
				break;

			case 'd' :	/* fd <len> - read and dump file from current fp */
				if (!xatoi(&ptr, &p1)) break;
				ofs = File1.fptr;
				while (p1) {
					if ((UINT)p1 >= 16) { cnt = 16; p1 -= 16; }
					else 				{ cnt = p1; p1 = 0; }
					res = f_read(&File1, Buff, cnt, &cnt);
					if (res != FR_OK) { put_rc(res); break; }
					if (!cnt) break;
					put_dump(Buff, ofs, cnt);
					ofs += 16;
				}
				break;

			case 'r' :	/* fr <len> - read file */
				if (!xatoi(&ptr, &p1)) break;
				p2 = 0;
				Timer = 0;
				while (p1) {
					if ((UINT)p1 >= blen) {
						cnt = blen; p1 -= blen;
					} else {
						cnt = p1; p1 = 0;
					}
					res = f_read(&File1, Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				xprintf("%lu bytes read with %lu kB/sec.\n", p2, Timer ? (p2 / Timer) : 0);
				break;

			case 'w' :	/* fw <len> <val> - write file */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				xmemset(Buff, (BYTE)p2, blen);
				p2 = 0;
				Timer = 0;
				while (p1) {
					if ((UINT)p1 >= blen) {
						cnt = blen; p1 -= blen;
					} else {
						cnt = p1; p1 = 0;
					}
					res = f_write(&File1, Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				xprintf("%lu bytes written with %lu kB/sec.\n", p2, Timer ? (p2 / Timer) : 0);
				break;

			case 'n' :	/* fn <old_name> <new_name> - Change file/dir name */
				while (*ptr == ' ') ptr++;
				ptr2 = xstrchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				while (*ptr2 == ' ') ptr2++;
				put_rc(f_rename(ptr, ptr2));
				break;

			case 'u' :	/* fu <name> - Unlink a file or dir */
				while (*ptr == ' ') ptr++;
				put_rc(f_unlink(ptr));
				break;

			case 'v' :	/* fv - Truncate file */
				put_rc(f_truncate(&File1));
				break;

			case 'k' :	/* fk <name> - Create a directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_mkdir(ptr));
				break;

			case 'a' :	/* fa <atrr> <mask> <name> - Change file/dir attribute */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_chmod(ptr, p1, p2));
				break;

			case 't' :	/* ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				Finfo.fdate = ((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31);
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				Finfo.ftime = ((p1 & 31) << 11) | ((p2 & 63) << 5) | ((p3 >> 1) & 31);
				put_rc(f_utime(ptr, &Finfo));
				break;

			case 'x' : /* fx <src_name> <dst_name> - Copy file */
				while (*ptr == ' ') ptr++;
				ptr2 = xstrchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				while (*ptr2 == ' ') ptr2++;
				xprintf("Opening \"%s\"", ptr);
				res = f_open(&File1, ptr, FA_OPEN_EXISTING | FA_READ);
				xputc('\n');
				if (res) {
					put_rc(res);
					break;
				}
				xprintf("Creating \"%s\"", ptr2);
				res = f_open(&File2, ptr2, FA_CREATE_ALWAYS | FA_WRITE);
				xputc('\n');
				if (res) {
					put_rc(res);
					f_close(&File1);
					break;
				}
				xprintf("Copying file...");
				Timer = 0;
				p1 = 0;
				for (;;) {
					res = f_read(&File1, Buff, blen, &s1);
					if (res || s1 == 0) break;   /* error or eof */
					res = f_write(&File2, Buff, s1, &s2);
					p1 += s2;
					if (res || s2 < s1) break;   /* error or disk full */
				}
				xprintf("%lu bytes copied with %lu kB/sec.\n", p1, p1 / Timer);
				f_close(&File1);
				f_close(&File2);
				break;
#if _FS_RPATH
			case 'g' :	/* fg <path> - Change current directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_chdir(ptr));
				break;

			case 'j' :	/* fj <drive#> - Change current drive */
				if (xatoi(&ptr, &p1)) {
					put_rc(f_chdrive((BYTE)p1));
				}
				break;
#endif
#if _USE_MKFS
			case 'm' :	/* fm <partition rule> <cluster size> - Create file system */
				if (!xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				xprintf("The card will be formatted. Are you sure? (Y/n)=");
				get_line(ptr, sizeof(Line));
				if (*ptr == 'Y')
					put_rc(f_mkfs(0, (BYTE)p2, (WORD)p3));
				break;
#endif
			case 'z' :	/* fz [<rw size>] - Change R/W length for fr/fw/fx command */
				if (xatoi(&ptr, &p1) && p1 >= 1 && (unsigned long)p1 <= sizeof(Buff))
					blen = p1;
				xprintf("blen=%u\n", blen);
				break;
			}
			break;

		case 't' :	/* t [<year> <mon> <mday> <hour> <min> <sec>] */
			if (xatoi(&ptr, &p1)) {
				rtc.YEAR = (WORD)p1;
				xatoi(&ptr, &p1); rtc.MONTH = (BYTE)p1;
				xatoi(&ptr, &p1); rtc.DOM = (BYTE)p1;
				xatoi(&ptr, &p1); rtc.HOUR = (BYTE)p1;
				xatoi(&ptr, &p1); rtc.MIN = (BYTE)p1;
				if (!xatoi(&ptr, &p1)) break;
				rtc.SEC = (BYTE)p1;
				rtc_cal_settime(&rtc);
			}
			rtc_cal_gettime(&rtc);
			xprintf("%u/%u/%u %02u:%02u:%02u\n",
					rtc.YEAR, rtc.MONTH, rtc.DOM, rtc.HOUR, rtc.MIN, rtc.SEC);

			break;

		}
	}
}

#else

static void delay_ms( UINT ms ) /* Delay in unit of ms */
{
	for (Timer = 0; Timer < ms;) { ; }
}

int ff_test_term(void)
{
	char *ptr, *ptr2;
	long p1, p2, p3;
	BYTE res;
	UINT s1, s2, cnt, blen = sizeof(Buff);
	DWORD ofs = 0, sect = 0;
	FATFS *fs;

	/* Startup message */
	delay_ms(10);
	xputs("\nFatFs module test monitor for LPC17xx\n");
	xputs(_USE_LFN ? "LFN Enabled" : "LFN Disabled");
	xprintf(", Code page: %u/ANSI\n", _CODE_PAGE);

#if _USE_LFN
	Finfo.lfname = Lfname;
	Finfo.lfsize = sizeof(Lfname);
#endif

	for (;;)
	{
		xputc('>');
		get_line(Line, sizeof(Line));

		ptr = Line;
		switch (*ptr++)
		{
		case '?': /* ? - Show usage */
			xputs(
					"md <address> [<len>] - Dump system memory\n"
						"\n"
						"dd [<lba>] - Dump secrtor\n"
						"di - Initialize disk\n"
						"ds - Show disk status\n"
						"\n"
						"bd <ofs> - Dump working buffer\n"
						"be <ofs> [<data>] ... - Edit working buffer\n"
						"br <lba> [<num>] - Read disk into working buffer\n"
						"bw <lba> [<num>] - Write working buffer into disk\n"
						"bf <val> - Fill working buffer\n"
						"\n"
						"fi - Force initialized the volume\n"
						"fs - Show volume status\n"
						"fl [<path>] - Show a directory\n"
						"fo <mode> <file> - Open a file\n"
						"fc - Close the file\n"
						"fe <ofs> - Move fp\n"
						"fd <len> - Read and dump the file\n"
						"fr <len> - Read the file\n"
						"fw <len> <val> - Write to the file\n"
						"fn <object name> <new name> - Rename an object\n"
						"fu <object name> - Unlink an object\n"
						"fv - Truncate the file at current fp\n"
						"fk <dir name> - Create a directory\n"
						"fa <atrr> <mask> <object name> - Change object attribute\n"
						"ft <year> <month> <day> <hour> <min> <sec> <object name> - Change timestamp of an object\n"
						"fx <src file> <dst file> - Copy a file\n"
						"fg <path> - Change current directory\n"
						"fm <rule> <cluster size> - Create file system\n"
						"fz [<len>] - Change R/W block length for fr/fw/fx command\n"
						"\n");
			break;

		case 'm':
			switch (*ptr++)
			{
			case 'd': /* md <address> [<count>] - Dump memory */
				if (!xatoi(&ptr, &p1))
					break;
				if (!xatoi(&ptr, &p2))
					p2 = 128;
				for (ptr = (char*) p1; p2 >= 16; ptr += 16, p2 -= 16)
					put_dump((BYTE*) ptr, (UINT) ptr, 16);
				if (p2)
					put_dump((BYTE*) ptr, (UINT) ptr, p2);
				break;
			}
			break;

		case 'd':
			switch (*ptr++)
			{
			case 'd': /* dd [<lba>] - Dump secrtor */
				if (!xatoi(&ptr, &p2))
					p2 = sect;
				res = disk_read(0, Buff, p2, 1);
				if (res)
				{
					xprintf("rc=%d\n", (WORD) res);
					break;
				}
				sect = p2 + 1;
				xprintf("Sector:%lu\n", p2);
				for (ptr = (char*) Buff, ofs = 0; ofs < 0x200; ptr += 16, ofs
						+= 16)
					put_dump((BYTE*) ptr, ofs, 16);
				break;

			case 'i': /* di - Initialize disk */
				xprintf("rc=%d\n", (WORD) disk_initialize(0));
				break;

			case 's': /* ds - Show disk status */
				show_disk_status(0);
				break;
			}
			break;

		case 'b':
			switch (*ptr++)
			{
			case 'd': /* bd <addr> - Dump R/W buffer */
				if (!xatoi(&ptr, &p1))
					break;
				for (ptr = (char*) &Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, ptr
						+= 16, ofs += 16)
					put_dump((BYTE*) ptr, ofs, 16);
				break;

			case 'e': /* be <addr> [<data>] ... - Edit R/W buffer */
				if (!xatoi(&ptr, &p1))
					break;
				if (xatoi(&ptr, &p2))
				{
					do
					{
						Buff[p1++] = (BYTE) p2;
					} while (xatoi(&ptr, &p2));
					break;
				}
				for (;;)
				{
					xprintf("0x%04X: 0x%02X-", (WORD) (p1), (WORD) Buff[p1]);
					get_line(Line, sizeof(Line));
					ptr = Line;
					if (*ptr == '.')
						break; /* Exit */
					if (*ptr < ' ')
					{
						p1++;
						continue;
					} /* Skip */
					if (xatoi(&ptr, &p2))
						Buff[p1++] = (BYTE) p2;
					else
						xputs("???\n");
				}
				break;

			case 'r': /* br <lba> [<num>] - Read disk into R/W buffer */
				if (!xatoi(&ptr, &p2))
					break;
				if (!xatoi(&ptr, &p3))
					p3 = 1;
				xprintf("rc=%u\n", (WORD) disk_read(0, Buff, p2, p3));
				break;

			case 'w': /* bw <lba> [<num>] - Write R/W buffer into disk */
				if (!xatoi(&ptr, &p2))
					break;
				if (!xatoi(&ptr, &p3))
					p3 = 1;
				xprintf("rc=%u\n", (WORD) disk_write(0, Buff, p2, p3));
				break;

			case 'f': /* bf <val> - Fill working buffer */
				if (!xatoi(&ptr, &p1))
					break;
				xmemset(Buff, (BYTE) p1, sizeof(Buff));
				break;

			}
			break;

		case 'f':
			switch (*ptr++)
			{

			case 'i': /* fi - Force initialized the logical drive */
				put_rc(f_mount(0, &Fatfs));
				break;

			case 's': /* fs - Show logical drive status */
				ptr = Line;
				*ptr = 0;
				show_fs_status(ptr);
				break;

			case 'l': /* fl [<path>] - Directory listing */
				while (*ptr == ' ')
					ptr++;
				res = f_opendir(&Dir, ptr);
				if (res)
				{
					put_rc(res);
					break;
				}
				p1 = s1 = s2 = 0;
				for (;;)
				{
					res = f_readdir(&Dir, &Finfo);
					if ((res != FR_OK) || !Finfo.fname[0])
						break;
					if (Finfo.fattrib & AM_DIR)
					{
						s2++;
					}
					else
					{
						s1++;
						p1 += Finfo.fsize;
					}
					xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
							(Finfo.fattrib & AM_DIR) ? 'D' : '-',
							(Finfo.fattrib & AM_RDO) ? 'R' : '-',
							(Finfo.fattrib & AM_HID) ? 'H' : '-',
							(Finfo.fattrib & AM_SYS) ? 'S' : '-',
							(Finfo.fattrib & AM_ARC) ? 'A' : '-', (Finfo.fdate
									>> 9) + 1980, (Finfo.fdate >> 5) & 15,
							Finfo.fdate & 31, (Finfo.ftime >> 11), (Finfo.ftime
									>> 5) & 63, Finfo.fsize, &(Finfo.fname[0]));
#if _USE_LFN
					for (p2 = xstrlen(Finfo.fname); p2 < 14; p2++)
						xputc(' ');
					xprintf("%s\n", Lfname);
#else
					xputc('\n');
#endif
				}
				xprintf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
				if (f_getfree(ptr, (DWORD*) &p1, &fs) == FR_OK)
					xprintf(", %10lu bytes free\n", p1 * fs->csize * 512);
				break;

			case 'o': /* fo <mode> <file> - Open a file */
				if (!xatoi(&ptr, &p1))
					break;
				while (*ptr == ' ')
					ptr++;
				put_rc(f_open(&File1, ptr, (BYTE) p1));
				break;

			case 'c': /* fc - Close a file */
				put_rc(f_close(&File1));
				break;

			case 'e': /* fe - Seek file pointer */
				if (!xatoi(&ptr, &p1))
					break;
				res = f_lseek(&File1, p1);
				put_rc(res);
				if (res == FR_OK)
					xprintf("fptr=%lu(0x%lX)\n", File1.fptr, File1.fptr);
				break;

			case 'd': /* fd <len> - read and dump file from current fp */
				if (!xatoi(&ptr, &p1))
					break;
				ofs = File1.fptr;
				while (p1)
				{
					if ((UINT) p1 >= 16)
					{
						cnt = 16;
						p1 -= 16;
					}
					else
					{
						cnt = p1;
						p1 = 0;
					}
					res = f_read(&File1, Buff, cnt, &cnt);
					if (res != FR_OK)
					{
						put_rc(res);
						break;
					}
					if (!cnt)
						break;
					put_dump(Buff, ofs, cnt);
					ofs += 16;
				}
				break;

			case 'r': /* fr <len> - read file */
				if (!xatoi(&ptr, &p1))
					break;
				p2 = 0;
				Timer = 0;
				while (p1)
				{
					if ((UINT) p1 >= blen)
					{
						cnt = blen;
						p1 -= blen;
					}
					else
					{
						cnt = p1;
						p1 = 0;
					}
					res = f_read(&File1, Buff, cnt, &s2);
					if (res != FR_OK)
					{
						put_rc(res);
						break;
					}
					p2 += s2;
					if (cnt != s2)
						break;
				}
				xprintf("%lu bytes read with %lu kB/sec.\n", p2, Timer ? (p2
						/ Timer) : 0);
				break;

			case 'w': /* fw <len> <val> - write file */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2))
					break;
				xmemset(Buff, (BYTE) p2, blen);
				p2 = 0;
				Timer = 0;
				while (p1)
				{
					if ((UINT) p1 >= blen)
					{
						cnt = blen;
						p1 -= blen;
					}
					else
					{
						cnt = p1;
						p1 = 0;
					}
					res = f_write(&File1, Buff, cnt, &s2);
					if (res != FR_OK)
					{
						put_rc(res);
						break;
					}
					p2 += s2;
					if (cnt != s2)
						break;
				}
				xprintf("%lu bytes written with %lu kB/sec.\n", p2, Timer ? (p2
						/ Timer) : 0);
				break;

			case 'n': /* fn <old_name> <new_name> - Change file/dir name */
				while (*ptr == ' ')
					ptr++;
				ptr2 = xstrchr(ptr, ' ');
				if (!ptr2)
					break;
				*ptr2++ = 0;
				while (*ptr2 == ' ')
					ptr2++;
				put_rc(f_rename(ptr, ptr2));
				break;

			case 'u': /* fu <name> - Unlink a file or dir */
				while (*ptr == ' ')
					ptr++;
				put_rc(f_unlink(ptr));
				break;

			case 'v': /* fv - Truncate file */
				put_rc(f_truncate(&File1));
				break;

			case 'k': /* fk <name> - Create a directory */
				while (*ptr == ' ')
					ptr++;
				put_rc(f_mkdir(ptr));
				break;

			case 'a': /* fa <atrr> <mask> <name> - Change file/dir attribute */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2))
					break;
				while (*ptr == ' ')
					ptr++;
				put_rc(f_chmod(ptr, p1, p2));
				break;

			case 't': /* ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3))
					break;
				Finfo.fdate = ((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31);
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3))
					break;
				Finfo.ftime = ((p1 & 31) << 11) | ((p2 & 63) << 5) | ((p3 >> 1)
						& 31);
				put_rc(f_utime(ptr, &Finfo));
				break;

			case 'x': /* fx <src_name> <dst_name> - Copy file */
				while (*ptr == ' ')
					ptr++;
				ptr2 = xstrchr(ptr, ' ');
				if (!ptr2)
					break;
				*ptr2++ = 0;
				while (*ptr2 == ' ')
					ptr2++;
				xprintf("Opening \"%s\"", ptr);
				res = f_open(&File1, ptr, FA_OPEN_EXISTING | FA_READ);
				xputc('\n');
				if (res)
				{
					put_rc(res);
					break;
				}
				xprintf("Creating \"%s\"", ptr2);
				res = f_open(&File2, ptr2, FA_CREATE_ALWAYS | FA_WRITE);
				xputc('\n');
				if (res)
				{
					put_rc(res);
					f_close(&File1);
					break;
				}
				xprintf("Copying file...");
				Timer = 0;
				p1 = 0;
				for (;;)
				{
					res = f_read(&File1, Buff, blen, &s1);
					if (res || s1 == 0)
						break; /* error or eof */
					res = f_write(&File2, Buff, s1, &s2);
					p1 += s2;
					if (res || s2 < s1)
						break; /* error or disk full */
				}
				xputc('\n');
				if (res)
					put_rc(res);
				else
					xprintf("%lu bytes copied with %lu kB/sec.\n", p1, p1
							/ Timer);
				f_close(&File1);
				f_close(&File2);
				break;
#if _FS_RPATH
				case 'g' : /* fg <path> - Change current directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_chdir(ptr));
				break;
#endif
#if _USE_MKFS
			case 'm': /* fm <partition rule> <cluster size> - Create file system */
				if (!xatoi(&ptr, &p2) || !xatoi(&ptr, &p3))
					break;
				xprintf("The card will be formatted. Are you sure? (Y/n)=");
				get_line(ptr, sizeof(Line));
				if (*ptr == 'Y')
					put_rc(f_mkfs(0, (BYTE) p2, (WORD) p3));
				break;
#endif
			case 'z': /* fz [<rw size>] - Change R/W length for fr/fw/fx command */
				if (xatoi(&ptr, &p1) && p1 >= 1 && (unsigned long)p1 <= sizeof(Buff))
					blen = p1;
				xprintf("blen=%u\n", blen);
				break;
			}
			break;
		}
	}
}

#endif
