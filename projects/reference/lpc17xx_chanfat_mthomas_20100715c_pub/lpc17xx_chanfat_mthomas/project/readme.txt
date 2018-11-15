******************************************************************************
  ChaN's FatFs on LPC17xx
  Version Timestamp 20100715
  by Martin Thomas, Kaiserslautern, Germany <mthomas(at)rhrk.uni-kl.de>
******************************************************************************

/* Copyright (c) 2010
   - ChaN (FAT-Code, support-functions, test-terminal)
   - Martin Thomas (LPC diskio-functions, modifications for LPC17xx, ...)
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */
******************************************************************************

Changes:
20100715 - added LPC RTC support, other minor modifications

TODOs:
- rework block read/write with DMA to get rid of dummy-buffers
- remove need for NVIC/DMA-interrupt when using DMA
- avoid remaining warnings

******************************************************************************

This is a small demo-application to show how ChaN's FAT-Module (FatFs)
can be used to interface MM-, SD- and SDHC-Cards with a NXP LPC17xx
microcontroller (ARM Ltd. Cortex-M3 core).

The example also includes an experimental interface between FatFs
and USB-host/mass-storage-code to access USB mass-storage devices.

Code from others used in this project:
- ChaN's FatFs available from elm-chan.org
- NXP's device library available from nxp.com
- NXP/OnChip Technology USB Host Stack Lite available from nxp.com

The LPC SSP FIFO code is inspired by the work of Mike Anton for the EFSL.

Tested with:
- Controller: NXP LPC1766, card connected to SSP1
- Board: Olimex LPC1766-STK
- Card(s): Kingston MircoSD 2GB, Fujifilm 8GB MicroSDHC 
- USB Mass-Storage Device(s): "Pendrive" USB Flash drive 512MB

Tools:
- Compiler/Toolchain: GNU cross-toolchain for ARM (binutils, gcc)
  with newlib as libc as in the package Codesourcery G++ lite for 
  ARM EABI
- IDEs/Editors: 
  - Eclipse IDE for C/C++ Developers, Additional plug-ins: 
    - CDT Hardware Debugging, Target Management Terminal, 
    - RXTX Enduser Runtime. This is a "Makefile project"
  - Keil µVision4 evaluation version, configured to use a 
    GNU toolchain (Realview tools not supported)
  - Programmers Notepad
- OpenOCD for "batch programming" (make program) and debugging with
  gdb and Eclipse.
- Amontec JTAGkey2 FTDI2232H-based JTAG-interface

******************************************************************************

Test with Kingston 2GB microSD-card using DMA
---------------------------------------------

(kB = KiB = 1024 bytes)

Hello from the ChaN FatFs Demo on LPC1700
Version 0.0.1, Martin Thomas 7/2010
xprintf is working

FatFs module test monitor for LPC17xx
LFN Enabled, Code page: 1252/ANSI
>di 0
rc=0
>ds
Drive size: 3854336 sectors
Sector size: 512
Erase block size: 8192 sectors
Card type: SDv2(Byte)
CSD:
00000000  00 4F 00 32 5B 5A A3 AC FF FF FF 8F 0A 80 00 45 .O.2[Z.........E
CID:
00000000  02 54 4D 53 41 30 32 47 03 9C B5 F1 9B 00 95 7D .TMSA02G.......}
OCR:
00000000  80 FF 80 00 ....
SD Status:
00000000  00 00 00 00 00 00 00 28 02 02 90 01 00 AA 00 00 .......(........
00000010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
00000020  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
00000030  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
>fi 0
rc=0 FR_OK
>fl
D---- 2000/01/01 00:00         0  LOGDAT        
----A 2009/08/20 19:54     15440  FIRMWARE.BIN  
----A 2009/07/30 13:55       191  settings.csv  
----A 2009/07/26 14:57       227  settings.ini  
----A 2009/08/23 20:29     16640  FIRMWARE.EBL  
----- 1980/00/00 00:00       594  LOGDATD.TXT   
----A 2010/04/26 00:00   2110000  2             
----A 2010/04/26 00:00   5120000  6             
----A 2010/04/26 00:00    500000  4             
----A 2010/04/26 00:00   5120000  5             
----A 2010/04/26 00:00      1536  7             
----A 2010/04/26 00:00   5120000  8             
----A 2010/04/26 00:00      1536  9             
----A 2010/04/26 00:00      1536  a             
----A 2010/04/26 00:00      1536  b             
----A 2010/04/26 00:00   5120000  1             
  15 File(s),  23129236 bytes total
   1 Dir(s), 1947566080 bytes free
>fo 10 1
rc=0 FR_OK
>fw 5120000 1
5120000 bytes written with 441 kB/sec.
>fc
rc=0 FR_OK
>fo 1 1
rc=0 FR_OK
>fr 5120000 
5120000 bytes read with 1420 kB/sec.
>fo 10 2
rc=0 FR_OK
>fw 5120000 1
5120000 bytes written with 432 kB/sec.
>fc
rc=0 FR_OK
>fo 1 2
rc=0 FR_OK
>fr 5120000
5120000 bytes read with 1436 kB/sec.
>


Test with Kingston 2GB microSD-card using FIFO
----------------------------------------------

Hello from the ChaN FatFs Demo on LPC1700
Version 0.0.1, Martin Thomas 7/2010
xprintf is working

FatFs module test monitor for LPC17xx
LFN Enabled, Code page: 1252/ANSI
>di 0 
rc=0
>ds
Drive size: 3854336 sectors
Sector size: 512
Erase block size: 8192 sectors
Card type: SDv2(Byte)
CSD:
00000000  00 4F 00 32 5B 5A A3 AC FF FF FF 8F 0A 80 00 45 .O.2[Z.........E
CID:
00000000  02 54 4D 53 41 30 32 47 03 9C B5 F1 9B 00 95 7D .TMSA02G.......}
OCR:
00000000  80 FF 80 00 ....
SD Status:
00000000  00 00 00 00 00 00 00 28 02 02 90 01 00 AA 00 00 .......(........
00000010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
00000020  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
00000030  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
>fi 0
rc=0 FR_OK
>fl
D---- 2000/01/01 00:00         0  LOGDAT        
----A 2009/08/20 19:54     15440  FIRMWARE.BIN  
----A 2009/07/30 13:55       191  settings.csv  
----A 2009/07/26 14:57       227  settings.ini  
----A 2009/08/23 20:29     16640  FIRMWARE.EBL  
----- 1980/00/00 00:00       594  LOGDATD.TXT   
----A 2010/04/26 00:00   5120000  2             
----A 2010/04/26 00:00   5120000  6             
----A 2010/04/26 00:00    500000  4             
----A 2010/04/26 00:00   5120000  5             
----A 2010/04/26 00:00      1536  7             
----A 2010/04/26 00:00   5120000  8             
----A 2010/04/26 00:00      1536  9             
----A 2010/04/26 00:00      1536  a             
----A 2010/04/26 00:00      1536  b             
----A 2010/04/26 00:00   5120000  1             
  15 File(s),  26139236 bytes total
     1 Dir(s), 1944551424 bytes free
>fo 10 c
rc=0 FR_OK
>fw 5120000 2
5120000 bytes written with 449 kB/sec.
>fc
rc=0 FR_OK
>fo 1 c
rc=0 FR_OK
>fr 5120000
5120000 bytes read with 1565 kB/sec.
>fc
rc=0 FR_OK
>fo 10 d
rc=0 FR_OK
>fw 5120000 3
5120000 bytes written with 446 kB/sec.
>fc
rc=0 FR_OK
>fo 1 d
rc=0 FR_OK
>fr 5120000
5120000 bytes read with 1556 kB/sec.
>fc
rc=0 FR_OK
>

-----

Test with USB Mass Storage device
----------------------------------

Hello from the ChaN FatFs Demo on LPC1700
Version 0.0.1, Martin Thomas 7/2010
xprintf is working

FatFs module test monitor for LPC17xx/SSP
LFN Enabled, Code page: 1252
>fi 1
rc=0 FR_OK
>fs 1:
Init called with 1
Initializing Host Stack
Host Initialized
Connect a Mass Storage device
Mass Storage device connected
FAT type = FAT32
Bytes/Cluster = 4096
Number of FATs = 2
Root DIR entries = 0
Sectors/FAT = 958
Number of clusters = 122504
FAT start (lba) = 68
DIR start (lba,clustor) = 2
Data start (lba) = 1984

61 files, 332406084 bytes.
5 folders.
490016 KB total disk space.
165248 KB available.
>fl 1:
D---- 2010/07/11 14:12         0  TRANSP~1      transport
D---- 2010/07/11 14:11         0  ONEWIR~1      onewire_zeugs
   0 File(s),         0 bytes total
   2 Dir(s),  169213952 bytes free
>fo 10 1:/1
rc=0 FR_OK
>fw 512 1
512 bytes written with 46 kB/sec.
>fw 512 2
512 bytes written with 256 kB/sec.
>fw 512 3
512 bytes written with 256 kB/sec.
>fc
rc=0 FR_OK
>fl 1:
D---- 2010/07/11 14:12         0  TRANSP~1      transport
----A 2010/04/26 00:00      1536  1             
D---- 2010/07/11 14:11         0  ONEWIR~1      onewire_zeugs
   1 File(s),      1536 bytes total
   2 Dir(s),  169209856 bytes free
>fo 1 1:/1
rc=0 FR_OK
>fc
rc=0 FR_OK
>fl
rc=12 FR_NOT_ENABLED
>fl 1:
D---- 2010/07/11 14:12         0  TRANSP~1      transport
----A 2010/04/26 00:00      1536  1             
D---- 2010/07/11 14:11         0  ONEWIR~1      onewire_zeugs
   1 File(s),      1536 bytes total
   2 Dir(s),  169209856 bytes free
>fo 10 1:/2.tst
rc=0 FR_OK
>fw 512 1
512 bytes written with 64 kB/sec.
>fw 512 2
512 bytes written with 170 kB/sec.
>fw 512 3
512 bytes written with 170 kB/sec.
>fc
rc=0 FR_OK
>fl 1:
D---- 2010/07/11 14:12         0  TRANSP~1      transport
----A 2010/04/26 00:00      1536  1             
----A 2010/04/26 00:00      1536  2.tst         
D---- 2010/07/11 14:11         0  ONEWIR~1      onewire_zeugs
   2 File(s),      3072 bytes total
   2 Dir(s),  169205760 bytes free
>fo 1 1:/2.tst
rc=0 FR_OK
>fd 512
00000000  01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 ................
00000010  01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 ................
[...]
000001E0  01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 ................
000001F0  01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 ................
>fd 512
00000200  02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 ................
00000210  02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 ................
[...]
000003E0  02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 ................
000003F0  02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 02 ................
>fd 512
00000400  03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 ................
00000410  03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 ................
[...]
000005E0  03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 ................
000005F0  03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 03 ................
>fc
rc=0 FR_OK
>fo 10 1:/3
rc=0 FR_OK
>fw 5120000 1
5120000 bytes written with 369 kB/sec.
>fc
rc=0 FR_OK
>fo 1 1:/3
rc=0 FR_OK
>fr 5120000
5120000 bytes read with 679 kB/sec.
>fx 0:/d 1:/dst
Opening "0:/d"
Creating "1:/dst"
Copying file...5120000 bytes copied with 302 kB/sec.
>fl 0:
D---- 2000/01/01 00:00         0  LOGDAT        
----A 2009/08/20 19:54     15440  FIRMWARE.BIN  
----A 2009/07/30 13:55       191  settings.csv  
----A 2009/07/26 14:57       227  settings.ini  
----A 2009/08/23 20:29     16640  FIRMWARE.EBL  
----- 1980/00/00 00:00       594  LOGDATD.TXT   
----A 2010/04/26 00:00   5120000  2             
----A 2010/04/26 00:00   5120000  6             
----A 2010/04/26 00:00    500000  4             
----A 2010/04/26 00:00   5120000  5             
----A 2010/04/26 00:00      1536  7             
----A 2010/04/26 00:00   5120000  8             
----A 2010/04/26 00:00      1536  9             
----A 2010/04/26 00:00      1536  a             
----A 2010/04/26 00:00      1536  b             
----A 2010/04/26 00:00   5120000  1             
----A 2010/04/26 00:00   5120000  c             
----A 2010/04/26 00:00   5120000  d             
  17 File(s),  36379236 bytes total
   1 Dir(s), 1934262272 bytes free
>fl 1:
D---- 2010/07/11 14:12         0  TRANSP~1      transport
----A 2010/04/26 00:00      1536  1             
----A 2010/04/26 00:00      1536  2.tst         
----A 2010/04/26 00:00   5120000  3             
----A 2010/04/26 00:00   5120000  dst           
D---- 2010/07/11 14:11         0  ONEWIR~1      onewire_zeugs
   4 File(s),  10243072 bytes total
   2 Dir(s),  158965760 bytes free
>


-----

Test with USB Mass Storage device and Fujifilm 8GB microSDHC
------------------------------------------------------------

Hello from the ChaN FatFs Demo on LPC1700
Version 0.0.2, Martin Thomas 7/2010
xprintf is working

FatFs module test monitor for LPC17xx/SSP
LFN Enabled, Code page: 1252
>di 0
rc=0
>ds 0
Drive size: 15544320 sectors
Sector size: 512
Erase block size: 8192 sectors
MMC/SDC type: 12
CSD:
00000000  40 0E 00 32 5B 59 00 00 3B 4B 7F 80 0A 40 40 41 @..2[Y..;K...@@A
CID:
00000000  1B 53 4D 30 30 30 30 30 10 74 1F 28 E0 00 9C 39 .SM00000.t.(...9
OCR:
00000000  C0 FF 80 00 ....
SD Status:
00000000  00 00 00 00 03 00 00 00 01 01 90 00 00 01 00 00 ................
00000010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
00000020  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
00000030  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
>fi 0
rc=0 FR_OK
>fl 0:
----A 2010/05/15 12:01 100000000  1             
----A 2010/05/16 01:41   7000000  2             
----A 2010/05/16 01:42  10000000  3             
----A 2010/05/16 01:43         1  ALONGF~1      A long Filename with Umlauts ÜÖÄüöä
----A 2091/05/29 15:27       216  LOGDAT23.TXT  
----A 2091/05/27 01:17    300000  4             
----A 2010/05/30 01:54       189  LOGDAT31.TXT  
D---- 2092/02/12 08:09         0  data          
----A 2010/07/15 11:56      1536  t1.bin        
   8 File(s), 117301942 bytes total
   1 Dir(s), 3527737344 bytes free
>fo 10 5
rc=0 FR_OK
>fw 512 3
512 bytes written with 15 kB/sec.
>fw 512 4
512 bytes written with 170 kB/sec.
>fw 512 5
512 bytes written with 170 kB/sec.
>fw 5120000 6
5120000 bytes written with 549 kB/sec.
>fc
rc=0 FR_OK
>di 1
rc=0
>ds 1
Drive size: 982015 sectors
Sector size: 512
Erase block size: 1 sectors
>fi 1
rc=0 FR_OK
>fs 1:
FAT type = FAT32
Bytes/Cluster = 4096
Number of FATs = 2
Root DIR entries = 0
Sectors/FAT = 958
Number of clusters = 122504
FAT start (lba) = 68
DIR start (lba,clustor) = 2
Data start (lba) = 1984

...
65 files, 342649156 bytes.
5 folders.
490016 KB total disk space.
155240 KB available.
>fl 1:/
D---- 2010/07/11 14:12         0  TRANSP~1      transport
----A 2010/04/26 00:00      1536  1             
----A 2010/04/26 00:00      1536  2.tst         
----A 2010/04/26 00:00   5120000  3             
----A 2010/04/26 00:00   5120000  dst           
D---- 2010/07/11 14:11         0  ONEWIR~1      onewire_zeugs
   4 File(s),  10243072 bytes total
   2 Dir(s),  158965760 bytes free
>fx 0:/5 1:/5usb
Opening "0:/5"
Creating "1:/5usb"
Copying file...5121536 bytes copied with 303 kB/sec.
>fo 1 1:/5usb
rc=0 FR_OK
>fr 512
512 bytes read with 170 kB/sec.
>fd 512
00000200  04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 ................
00000210  04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 ................
[...]
000003E0  04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 ................
000003F0  04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 ................
>fd 512
00000400  05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 ................
00000410  05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 ................
[...]
000005E0  05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 ................
000005F0  05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 05 ................
>fr 10000000
5120000 bytes read with 453 kB/sec.
>fc
rc=0 FR_OK
>fl 1:/
D---- 2010/07/11 14:12         0  TRANSP~1      transport
----A 2010/04/26 00:00      1536  1             
----A 2010/04/26 00:00      1536  2.tst         
----A 2010/04/26 00:00   5120000  3             
----A 2010/04/26 00:00   5120000  dst           
----A 2010/07/15 11:58   5121536  5usb          
D---- 2010/07/11 14:11         0  ONEWIR~1      onewire_zeugs
   5 File(s),  15364608 bytes total
   2 Dir(s),  153841664 bytes free
>t
2010/7/15 11:56:34
>t 2010 7 15 2 3 0
2010/7/15 02:03:00
>t
2010/7/15 02:03:03
>t
2010/7/15 02:03:05
>


-----


C:\WinARM\examples\lpc17xx_chanfat_mthomas\project>make clean all program
COMSPEC detected  C:\WINDOWS\system32\cmd.exe
SHELL is C:/WINDOWS/system32/cmd.exe, REMOVE_CMD is cs-rm
--------  begin, mode: FLASH_RUN  --------
Cleaning project:
cs-rm -f FLASH_RUN/project.map
cs-rm -f FLASH_RUN/project.elf
cs-rm -f FLASH_RUN/project.hex
cs-rm -f FLASH_RUN/project.bin
cs-rm -f FLASH_RUN/project.sym
cs-rm -f FLASH_RUN/project.lss
cs-rm -f FLASH_RUN/startup_LPC17xx.o FLASH_RUN/main.o FLASH_RUN/comm.o FLASH_RUN
/monitor.o FLASH_RUN/ff_test_term.o FLASH_RUN/rtc_cal.o FLASH_RUN/ff.o FLASH_RUN
/ccsbcs.o FLASH_RUN/diskio.o FLASH_RUN/spi_sd_lpc17xx.o FLASH_RUN/fattime.o FLAS
H_RUN/core_cm3.o FLASH_RUN/system_LPC17xx.o FLASH_RUN/lpc17xx_nvic.o FLASH_RUN/l
pc17xx_clkpwr.o FLASH_RUN/lpc17xx_pinsel.o FLASH_RUN/lpc17xx_gpio.o FLASH_RUN/lp
c17xx_uart.o FLASH_RUN/lpc17xx_ssp.o FLASH_RUN/lpc17xx_gpdma.o FLASH_RUN/lpc17xx
_rtc.o FLASH_RUN/usbhost_lpc17xx.o FLASH_RUN/usbhost_ms.o
cs-rm -f FLASH_RUN/startup_LPC17xx.lst FLASH_RUN/main.lst FLASH_RUN/comm.lst FLA
SH_RUN/monitor.lst FLASH_RUN/ff_test_term.lst FLASH_RUN/rtc_cal.lst FLASH_RUN/ff
.lst FLASH_RUN/ccsbcs.lst FLASH_RUN/diskio.lst FLASH_RUN/spi_sd_lpc17xx.lst FLAS
H_RUN/fattime.lst FLASH_RUN/core_cm3.lst FLASH_RUN/system_LPC17xx.lst FLASH_RUN/
lpc17xx_nvic.lst FLASH_RUN/lpc17xx_clkpwr.lst FLASH_RUN/lpc17xx_pinsel.lst FLASH
_RUN/lpc17xx_gpio.lst FLASH_RUN/lpc17xx_uart.lst FLASH_RUN/lpc17xx_ssp.lst FLASH
_RUN/lpc17xx_gpdma.lst FLASH_RUN/lpc17xx_rtc.lst FLASH_RUN/usbhost_lpc17xx.lst F
LASH_RUN/usbhost_ms.lst
cs-rm -f FLASH_RUN/dep/startup_LPC17xx.o.d FLASH_RUN/dep/main.o.d FLASH_RUN/dep/
comm.o.d FLASH_RUN/dep/monitor.o.d FLASH_RUN/dep/ff_test_term.o.d FLASH_RUN/dep/
rtc_cal.o.d FLASH_RUN/dep/ff.o.d FLASH_RUN/dep/ccsbcs.o.d FLASH_RUN/dep/diskio.o
.d FLASH_RUN/dep/spi_sd_lpc17xx.o.d FLASH_RUN/dep/fattime.o.d FLASH_RUN/dep/core
_cm3.o.d FLASH_RUN/dep/system_LPC17xx.o.d FLASH_RUN/dep/lpc17xx_nvic.o.d FLASH_R
UN/dep/lpc17xx_clkpwr.o.d FLASH_RUN/dep/lpc17xx_pinsel.o.d FLASH_RUN/dep/lpc17xx
_gpio.o.d FLASH_RUN/dep/lpc17xx_uart.o.d FLASH_RUN/dep/lpc17xx_ssp.o.d FLASH_RUN
/dep/lpc17xx_gpdma.o.d FLASH_RUN/dep/lpc17xx_rtc.o.d FLASH_RUN/dep/usbhost_lpc17
xx.o.d FLASH_RUN/dep/usbhost_ms.o.d
cs-rm -f main.s comm.s monitor.s ff_test_term.s rtc_cal.s ./Libraries/fat_sd/ff.
s ./Libraries/fat_sd/option/ccsbcs.s ./Libraries/fat_sd/diskio.s ./Libraries/fat
_sd/spi_sd_lpc17xx.s ./Libraries/fat_sd/fattime.s ./Libraries/NXP/Core/CM3/CoreS
upport/core_cm3.s ./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx/system_LPC1
7xx.s ./Libraries/NXP/Drivers/source/lpc17xx_nvic.s ./Libraries/NXP/Drivers/sour
ce/lpc17xx_clkpwr.s ./Libraries/NXP/Drivers/source/lpc17xx_pinsel.s ./Libraries/
NXP/Drivers/source/lpc17xx_gpio.s ./Libraries/NXP/Drivers/source/lpc17xx_uart.s
./Libraries/NXP/Drivers/source/lpc17xx_ssp.s ./Libraries/NXP/Drivers/source/lpc1
7xx_gpdma.s ./Libraries/NXP/Drivers/source/lpc17xx_rtc.s ./Libraries/USBHostLite
/usbhost_lpc17xx.s ./Libraries/USBHostLite/usbhost_ms.s
cs-rm -f
cs-rm -f
cs-rm -f
--------  end  --------
arm-none-eabi-gcc (Sourcery G++ Lite 2010q1-188) 4.4.1
Copyright (C) 2009 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

---- Assembling: startup_LPC17xx.S to FLASH_RUN/startup_LPC17xx.o
arm-none-eabi-gcc -c -mthumb -mcpu=cortex-m3 -mthumb-interwork -I. -x assembler-
with-cpp -D__ASSEMBLY__  -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766
_STK -Wa,-adhlns=FLASH_RUN/startup_LPC17xx.lst -Wa,-ggdb -I./Libraries/NXP/Drive
rs/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I./Libraries/NX
P/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostLite startup_LP
C17xx.S -o FLASH_RUN/startup_LPC17xx.o
---- Compiling C: main.c to FLASH_RUN/main.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/main.lst -MMD -MP -MF FLASH_RUN/dep/main.o.d -Wnested-externs  -std=
gnu99 main.c -o FLASH_RUN/main.o
---- Compiling C: comm.c to FLASH_RUN/comm.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/comm.lst -MMD -MP -MF FLASH_RUN/dep/comm.o.d -Wnested-externs  -std=
gnu99 comm.c -o FLASH_RUN/comm.o
---- Compiling C: monitor.c to FLASH_RUN/monitor.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/monitor.lst -MMD -MP -MF FLASH_RUN/dep/monitor.o.d -Wnested-externs
 -std=gnu99 monitor.c -o FLASH_RUN/monitor.o
---- Compiling C: ff_test_term.c to FLASH_RUN/ff_test_term.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/ff_test_term.lst -MMD -MP -MF FLASH_RUN/dep/ff_test_term.o.d -Wneste
d-externs  -std=gnu99 ff_test_term.c -o FLASH_RUN/ff_test_term.o
---- Compiling C: rtc_cal.c to FLASH_RUN/rtc_cal.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/rtc_cal.lst -MMD -MP -MF FLASH_RUN/dep/rtc_cal.o.d -Wnested-externs
 -std=gnu99 rtc_cal.c -o FLASH_RUN/rtc_cal.o
---- Compiling C: Libraries/fat_sd/ff.c to FLASH_RUN/ff.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/ff.lst -MMD -MP -MF FLASH_RUN/dep/ff.o.d -Wnested-externs  -std=gnu9
9 Libraries/fat_sd/ff.c -o FLASH_RUN/ff.o
---- Compiling C: Libraries/fat_sd/option/ccsbcs.c to FLASH_RUN/ccsbcs.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/ccsbcs.lst -MMD -MP -MF FLASH_RUN/dep/ccsbcs.o.d -Wnested-externs  -
std=gnu99 Libraries/fat_sd/option/ccsbcs.c -o FLASH_RUN/ccsbcs.o
---- Compiling C: Libraries/fat_sd/diskio.c to FLASH_RUN/diskio.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/diskio.lst -MMD -MP -MF FLASH_RUN/dep/diskio.o.d -Wnested-externs  -
std=gnu99 Libraries/fat_sd/diskio.c -o FLASH_RUN/diskio.o
Libraries/fat_sd/diskio.c: In function 'disk_write':
Libraries/fat_sd/diskio.c:210: warning: cast discards qualifiers from pointer ta
rget type
---- Compiling C: Libraries/fat_sd/spi_sd_lpc17xx.c to FLASH_RUN/spi_sd_lpc17xx.
o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/spi_sd_lpc17xx.lst -MMD -MP -MF FLASH_RUN/dep/spi_sd_lpc17xx.o.d -Wn
ested-externs  -std=gnu99 Libraries/fat_sd/spi_sd_lpc17xx.c -o FLASH_RUN/spi_sd_
lpc17xx.o
---- Compiling C: Libraries/fat_sd/fattime.c to FLASH_RUN/fattime.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/fattime.lst -MMD -MP -MF FLASH_RUN/dep/fattime.o.d -Wnested-externs
 -std=gnu99 Libraries/fat_sd/fattime.c -o FLASH_RUN/fattime.o
---- Compiling C: Libraries/NXP/Core/CM3/CoreSupport/core_cm3.c to FLASH_RUN/cor
e_cm3.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/core_cm3.lst -MMD -MP -MF FLASH_RUN/dep/core_cm3.o.d -Wnested-extern
s  -std=gnu99 Libraries/NXP/Core/CM3/CoreSupport/core_cm3.c -o FLASH_RUN/core_cm
3.o
---- Compiling C: Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx/system_LPC17x
x.c to FLASH_RUN/system_LPC17xx.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/system_LPC17xx.lst -MMD -MP -MF FLASH_RUN/dep/system_LPC17xx.o.d -Wn
ested-externs  -std=gnu99 Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx/syste
m_LPC17xx.c -o FLASH_RUN/system_LPC17xx.o
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_nvic.c to FLASH_RUN/lpc17
xx_nvic.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_nvic.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_nvic.o.d -Wneste
d-externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_nvic.c -o FLASH_RUN/l
pc17xx_nvic.o
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_clkpwr.c to FLASH_RUN/lpc
17xx_clkpwr.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_clkpwr.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_clkpwr.o.d -Wn
ested-externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_clkpwr.c -o FLASH
_RUN/lpc17xx_clkpwr.o
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_pinsel.c to FLASH_RUN/lpc
17xx_pinsel.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_pinsel.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_pinsel.o.d -Wn
ested-externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_pinsel.c -o FLASH
_RUN/lpc17xx_pinsel.o
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_gpio.c to FLASH_RUN/lpc17
xx_gpio.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_gpio.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_gpio.o.d -Wneste
d-externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_gpio.c -o FLASH_RUN/l
pc17xx_gpio.o
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_uart.c to FLASH_RUN/lpc17
xx_uart.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_uart.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_uart.o.d -Wneste
d-externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_uart.c -o FLASH_RUN/l
pc17xx_uart.o
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_ssp.c to FLASH_RUN/lpc17x
x_ssp.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_ssp.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_ssp.o.d -Wnested-
externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_ssp.c -o FLASH_RUN/lpc1
7xx_ssp.o
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_gpdma.c to FLASH_RUN/lpc1
7xx_gpdma.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_gpdma.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_gpdma.o.d -Wnes
ted-externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_gpdma.c -o FLASH_RU
N/lpc17xx_gpdma.o
Libraries/NXP/Drivers/source/lpc17xx_gpdma.c: In function 'GPDMA_Setup':
Libraries/NXP/Drivers/source/lpc17xx_gpdma.c:246: warning: cast discards qualifi
ers from pointer target type
Libraries/NXP/Drivers/source/lpc17xx_gpdma.c: In function 'GPDMA_ChannelCmd':
Libraries/NXP/Drivers/source/lpc17xx_gpdma.c:379: warning: cast discards qualifi
ers from pointer target type
---- Compiling C: Libraries/NXP/Drivers/source/lpc17xx_rtc.c to FLASH_RUN/lpc17x
x_rtc.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/lpc17xx_rtc.lst -MMD -MP -MF FLASH_RUN/dep/lpc17xx_rtc.o.d -Wnested-
externs  -std=gnu99 Libraries/NXP/Drivers/source/lpc17xx_rtc.c -o FLASH_RUN/lpc1
7xx_rtc.o
---- Compiling C: Libraries/USBHostLite/usbhost_lpc17xx.c to FLASH_RUN/usbhost_l
pc17xx.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/usbhost_lpc17xx.lst -MMD -MP -MF FLASH_RUN/dep/usbhost_lpc17xx.o.d -
Wnested-externs  -std=gnu99 Libraries/USBHostLite/usbhost_lpc17xx.c -o FLASH_RUN
/usbhost_lpc17xx.o
---- Compiling C: Libraries/USBHostLite/usbhost_ms.c to FLASH_RUN/usbhost_ms.o
arm-none-eabi-gcc -c -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWIT
H_USB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Libra
ries/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I
./Libraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostL
ite -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-alig
n -Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhln
s=FLASH_RUN/usbhost_ms.lst -MMD -MP -MF FLASH_RUN/dep/usbhost_ms.o.d -Wnested-ex
terns  -std=gnu99 Libraries/USBHostLite/usbhost_ms.c -o FLASH_RUN/usbhost_ms.o
---- Linking: FLASH_RUN/project.elf
arm-none-eabi-gcc -mthumb -ggdb -Os -mcpu=cortex-m3 -mthumb-interwork   -DWITH_U
SB_MS=1 -DVECT_TAB_FLASH -DFLASH_RUN -DLPC1766 -DOLIMEX_LPC1766_STK -I./Librarie
s/NXP/Drivers/include -I./Libraries/NXP/Core/CM3/DeviceSupport/NXP/LPC17xx -I./L
ibraries/NXP/Core/CM3/CoreSupport -I./Libraries/fat_sd -I./Libraries/USBHostLite
 -I. -ffunction-sections -fdata-sections -Wall -Wextra -Wimplicit -Wcast-align -
Wpointer-arith -Wredundant-decls -Wshadow -Wcast-qual -Wcast-align -Wa,-adhlns=F
LASH_RUN/startup_LPC17xx.lst -MMD -MP -MF FLASH_RUN/dep/project.elf.d FLASH_RUN/
startup_LPC17xx.o FLASH_RUN/main.o FLASH_RUN/comm.o FLASH_RUN/monitor.o FLASH_RU
N/ff_test_term.o FLASH_RUN/rtc_cal.o FLASH_RUN/ff.o FLASH_RUN/ccsbcs.o FLASH_RUN
/diskio.o FLASH_RUN/spi_sd_lpc17xx.o FLASH_RUN/fattime.o FLASH_RUN/core_cm3.o FL
ASH_RUN/system_LPC17xx.o FLASH_RUN/lpc17xx_nvic.o FLASH_RUN/lpc17xx_clkpwr.o FLA
SH_RUN/lpc17xx_pinsel.o FLASH_RUN/lpc17xx_gpio.o FLASH_RUN/lpc17xx_uart.o FLASH_
RUN/lpc17xx_ssp.o FLASH_RUN/lpc17xx_gpdma.o FLASH_RUN/lpc17xx_rtc.o FLASH_RUN/us
bhost_lpc17xx.o FLASH_RUN/usbhost_ms.o --output FLASH_RUN/project.elf -nostartfi
les -Wl,-Map=FLASH_RUN/project.map,--cref,--gc-sections -lc -lm -lc -lgcc -lstdc
++  -L.   -T./LPC1766_flash.ld
Creating Extended Listing/Disassembly: FLASH_RUN/project.lss
arm-none-eabi-objdump -h -S -C -r FLASH_RUN/project.elf > FLASH_RUN/project.lss
Creating Symbol Table: FLASH_RUN/project.sym
arm-none-eabi-nm -n FLASH_RUN/project.elf > FLASH_RUN/project.sym
Creating load file: FLASH_RUN/project.hex
arm-none-eabi-objcopy -O ihex FLASH_RUN/project.elf FLASH_RUN/project.hex
Creating load file: FLASH_RUN/project.bin
arm-none-eabi-objcopy -O binary FLASH_RUN/project.elf FLASH_RUN/project.bin
Size after build:
arm-none-eabi-size -A  FLASH_RUN/project.elf
FLASH_RUN/project.elf  :
section             size        addr
.text              27208           0
.ARM.exidx             8       27208
.data                 16   268435456
.bss                8976   268435472
.heap               4096   268444448
.stack               768   268467456
.comment             946           0
.debug_aranges      2840           0
.debug_pubnames     5265           0
.debug_info        49207           0
.debug_abbrev      10523           0
.debug_line        16257           0
.debug_frame        7576           0
.debug_str         13456           0
.debug_loc         30496           0
.ARM.attributes       45           0
.debug_ranges       3128           0
Total             180811


"Programming with OPENOCD"
.\OpenOCD\bin\openocd -d0 -f interface/jtagkey2.cfg -f LPC1766.cfg -c init -c ta
rgets -c "reset init" -c "poll off" -c "flash write_image erase FLASH_RUN/projec
t.elf" -c "verify_image FLASH_RUN/project.elf" -c "reset run" -c shutdown
Open On-Chip Debugger 0.5.0-dev-snapshot (2010-07-08-20:52)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.berlios.de/doc/doxygen/bugs.html
debug_level: 0
adapter_nsrst_delay: 300
jtag_ntrst_delay: 300
trst_and_srst separate srst_gates_jtag trst_push_pull srst_open_drain
500 kHz
    TargetName         Type       Endian TapName            State
--  ------------------ ---------- ------ ------------------ ------------
 0* lpc1766.cpu        cortex_m3  little lpc1766.cpu        running
target state: halted
target halted due to debug-request, current mode: Thread
xPSR: 0x01000000 pc: 0x1fff0080 msp: 0x10001ffc
500 kHz
RCLK - adaptive
auto erase enabled
wrote 28672 bytes from file FLASH_RUN/project.elf in 3.578125s (7.825 KiB/s)
verified 27232 bytes in 0.953125s (27.902 KiB/s)
shutdown command invoked

C:\WinARM\examples\lpc17xx_chanfat_mthomas\project>

