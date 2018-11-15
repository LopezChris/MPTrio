################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include zCpu/subdir.mk
-include startup/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
lpc1758_NoOS_HelloWorld.hex \

SECONDARY_LIST += \
lpc1758_NoOS_HelloWorld.lst \

SECONDARY_SIZE += \
lpc1758_NoOS_HelloWorld.siz \


# All Target
all: lpc1758_NoOS_HelloWorld.elf secondary-outputs

# Tool invocations
lpc1758_NoOS_HelloWorld.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Yagarto Windows GCC C Linker'
	arm-none-eabi-gcc -T"..\startup\loader.ld" -Xlinker --gc-sections -Wl,-Map,"lpc1758_NoOS_HelloWorld.map" -mcpu=cortex-m3 -mthumb -o "lpc1758_NoOS_HelloWorld.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

lpc1758_NoOS_HelloWorld.hex: lpc1758_NoOS_HelloWorld.elf
	@echo 'Invoking: ARM Yagarto Windows GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "lpc1758_NoOS_HelloWorld.elf"  "lpc1758_NoOS_HelloWorld.hex"
	@echo 'Finished building: $@'
	@echo ' '

lpc1758_NoOS_HelloWorld.lst: lpc1758_NoOS_HelloWorld.elf
	@echo 'Invoking: ARM Yagarto Windows GNU Create Listing'
	arm-none-eabi-objdump -h -S "lpc1758_NoOS_HelloWorld.elf" > "lpc1758_NoOS_HelloWorld.lst"
	@echo 'Finished building: $@'
	@echo ' '

lpc1758_NoOS_HelloWorld.siz: lpc1758_NoOS_HelloWorld.elf
	@echo 'Invoking: ARM Yagarto Windows GNU Print Size'
	arm-none-eabi-size  --format=berkeley -x -t "lpc1758_NoOS_HelloWorld.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_SIZE)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(SECONDARY_FLASH)$(EXECUTABLES)$(SECONDARY_LIST)$(S_UPPER_DEPS) lpc1758_NoOS_HelloWorld.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
