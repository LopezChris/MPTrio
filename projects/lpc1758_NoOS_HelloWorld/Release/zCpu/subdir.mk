################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../zCpu/core_cm3.c \
../zCpu/printf-stdarg.c \
../zCpu/system_LPC17xx.c 

OBJS += \
./zCpu/core_cm3.o \
./zCpu/printf-stdarg.o \
./zCpu/system_LPC17xx.o 

C_DEPS += \
./zCpu/core_cm3.d \
./zCpu/printf-stdarg.d \
./zCpu/system_LPC17xx.d 


# Each subdirectory must supply rules for building sources it contributes
zCpu/%.o: ../zCpu/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Yagarto Windows GCC C Compiler'
	arm-none-eabi-gcc -I"D:\SJSU_Dev\projects\lpc1758_NoOS_HelloWorld\zCpu" -Os -ffunction-sections -fdata-sections -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=cortex-m3 -mthumb -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


