################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: StratifyAlpha.elf

# Tool invocations
StratifyAlpha.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-none-eabi-gcc -L"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/armv7e-m/fpu/fpv5-sp-d16" -L"/Applications/StratifyLabs-SDK/Tools/gcc/lib/gcc/arm-none-eabi/5.4.1/armv7e-m/fpu/fpv5-sp-d16" -Wl,-Map,fw.map,--gc,--defsym=mcu_core_hardware_id=0x0000000A -Ttext=0x40000 -Tldscripts/lpc4078-rom.ld -nostdlib -u symbols_table -o "StratifyAlpha.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) StratifyAlpha.elf
	-@echo ' '

post-build:
	-sh /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts/kernel-post-process.sh /Applications/StratifyLabs-SDK/Tools/gcc arm-none-eabi StratifyAlpha StratifyAlpha armv7e-m fpu/fpv5-sp-d16 _debug
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
