@echo off

start OpenOCD\bin\openocd.exe -d1 -f interface/jtagkey2.cfg -f lpc1766.cfg  -c "adapter_khz 500" -c init -c "target_request debugmsgs enable"
start telnet localhost 4444

rem set IMGFILE=FLASH_RUN/project.elf
rem OpenOCD\bin\openocd.exe -d0 -f interface/jtagkey2.cfg -f lpc1766.cfg -c init -c "reset halt" -c "flash write_image erase %IMGFILE%" -c "verify_image %IMGFILE%" -c "reset run" -c shutdown
rem pause