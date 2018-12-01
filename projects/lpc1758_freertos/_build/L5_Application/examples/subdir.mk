################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../L5_Application/examples/examples.cpp \
../L5_Application/examples/rn_xv_task.cpp 

OBJS += \
./L5_Application/examples/examples.o \
./L5_Application/examples/rn_xv_task.o 

CPP_DEPS += \
./L5_Application/examples/examples.d \
./L5_Application/examples/rn_xv_task.d 


# Each subdirectory must supply rules for building sources it contributes
L5_Application/examples/%.o: ../L5_Application/examples/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -ffunction-sections -fdata-sections -Wall -Wshadow -Wlogical-op -Wfloat-equal -DBUILD_CFG_MPU=0 -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\newlib" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L0_LowLevel" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L1_FreeRTOS" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L1_FreeRTOS\include" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L1_FreeRTOS\portable" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L1_FreeRTOS\portable\no_mpu" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L2_Drivers" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L2_Drivers\base" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L3_Utils" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L3_Utils\tlm" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L4_IO" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L4_IO\fat" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L4_IO\wireless" -I"C:\Users\cfarnes\Documents\MPTrio\projects\lpc1758_freertos\L5_Application" -std=gnu++11 -fabi-version=0 -fno-exceptions -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


