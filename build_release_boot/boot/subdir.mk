################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../boot/boot_board_config.c \
../boot/link_transport.c 

OBJS += \
./boot/boot_board_config.o \
./boot/link_transport.o 

C_DEPS += \
./boot/boot_board_config.d \
./boot/link_transport.d 


# Each subdirectory must supply rules for building sources it contributes
boot/%.o: ../boot/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__lpc407x_8x -D__ -D__HARDWARE_ID=0x0000000A -Os -Wall -c -fmessage-length=0 -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


