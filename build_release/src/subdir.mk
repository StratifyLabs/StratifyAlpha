################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/board_config.c \
../src/board_trace.c \
../src/link_transport.c \
../src/stratify_symbols.c 

OBJS += \
./src/board_config.o \
./src/board_trace.o \
./src/link_transport.o \
./src/stratify_symbols.o 

C_DEPS += \
./src/board_config.d \
./src/board_trace.d \
./src/link_transport.d \
./src/stratify_symbols.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__lpc407x_8x -D__ -Os -Wall -c -fmessage-length=0 -fomit-frame-pointer -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -U__SOFTFP__ -D__FPU_USED=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


