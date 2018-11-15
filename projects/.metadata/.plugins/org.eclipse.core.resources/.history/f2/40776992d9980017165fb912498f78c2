################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L5_Application/source/high_level_init.cpp \
../L5_Application/source/remote.cpp \
../L5_Application/source/terminal.cpp 

OBJS += \
./L5_Application/source/high_level_init.o \
./L5_Application/source/remote.o \
./L5_Application/source/terminal.o 

CPP_DEPS += \
./L5_Application/source/high_level_init.d \
./L5_Application/source/remote.d \
./L5_Application/source/terminal.d 


# Each subdirectory must supply rules for building sources it contributes
L5_Application/source/%.o: ../L5_Application/source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -ffunction-sections -fdata-sections -Wall -Wshadow -Wlogical-op -Wfloat-equal -DBUILD_CFG_MPU=0 -I"C:\SJSU_Dev\projects\lpc1758_freertos" -I"C:\SJSU_Dev\projects\lpc1758_freertos\newlib" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L0_LowLevel" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L1_FreeRTOS" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L1_FreeRTOS\include" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L1_FreeRTOS\portable" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L1_FreeRTOS\portable\no_mpu" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L2_Drivers" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L2_Drivers\base" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L3_Utils" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L3_Utils\tlm" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L4_IO" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L4_IO\fat" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L4_IO\wireless" -I"C:\SJSU_Dev\projects\lpc1758_freertos\L5_Application" -std=gnu++11 -fabi-version=0 -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


