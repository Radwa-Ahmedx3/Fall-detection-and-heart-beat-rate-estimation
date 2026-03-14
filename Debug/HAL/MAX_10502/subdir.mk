################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HAL/MAX_10502/MAX.c 

OBJS += \
./HAL/MAX_10502/MAX.o 

C_DEPS += \
./HAL/MAX_10502/MAX.d 


# Each subdirectory must supply rules for building sources it contributes
HAL/MAX_10502/%.o: ../HAL/MAX_10502/%.c HAL/MAX_10502/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


