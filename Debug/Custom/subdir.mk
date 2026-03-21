################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Custom/appl_main.c \
../Custom/lib_modbus.c \
../Custom/mgr_adc.c \
../Custom/mgr_can.c \
../Custom/mgr_gpio.c \
../Custom/mgr_i2c.c \
../Custom/mgr_uart.c 

OBJS += \
./Custom/appl_main.o \
./Custom/lib_modbus.o \
./Custom/mgr_adc.o \
./Custom/mgr_can.o \
./Custom/mgr_gpio.o \
./Custom/mgr_i2c.o \
./Custom/mgr_uart.o 

C_DEPS += \
./Custom/appl_main.d \
./Custom/lib_modbus.d \
./Custom/mgr_adc.d \
./Custom/mgr_can.d \
./Custom/mgr_gpio.d \
./Custom/mgr_i2c.d \
./Custom/mgr_uart.d 


# Each subdirectory must supply rules for building sources it contributes
Custom/%.o Custom/%.su Custom/%.cyclo: ../Custom/%.c Custom/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../modules/g4-lib-i2c -I.././Custom -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Custom

clean-Custom:
	-$(RM) ./Custom/appl_main.cyclo ./Custom/appl_main.d ./Custom/appl_main.o ./Custom/appl_main.su ./Custom/lib_modbus.cyclo ./Custom/lib_modbus.d ./Custom/lib_modbus.o ./Custom/lib_modbus.su ./Custom/mgr_adc.cyclo ./Custom/mgr_adc.d ./Custom/mgr_adc.o ./Custom/mgr_adc.su ./Custom/mgr_can.cyclo ./Custom/mgr_can.d ./Custom/mgr_can.o ./Custom/mgr_can.su ./Custom/mgr_gpio.cyclo ./Custom/mgr_gpio.d ./Custom/mgr_gpio.o ./Custom/mgr_gpio.su ./Custom/mgr_i2c.cyclo ./Custom/mgr_i2c.d ./Custom/mgr_i2c.o ./Custom/mgr_i2c.su ./Custom/mgr_uart.cyclo ./Custom/mgr_uart.d ./Custom/mgr_uart.o ./Custom/mgr_uart.su

.PHONY: clean-Custom

