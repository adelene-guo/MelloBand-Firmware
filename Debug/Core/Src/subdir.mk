################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/button.c \
../Core/Src/charge_status.c \
../Core/Src/drv2605.c \
../Core/Src/gpio.c \
../Core/Src/haptic.c \
../Core/Src/i2c.c \
../Core/Src/main.c \
../Core/Src/rgb_led.c \
../Core/Src/stm32f4xx_hal_i2c.c \
../Core/Src/stm32f4xx_hal_i2c_ex.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_hal_tim.c \
../Core/Src/stm32f4xx_hal_tim_ex.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tim.c 

OBJS += \
./Core/Src/button.o \
./Core/Src/charge_status.o \
./Core/Src/drv2605.o \
./Core/Src/gpio.o \
./Core/Src/haptic.o \
./Core/Src/i2c.o \
./Core/Src/main.o \
./Core/Src/rgb_led.o \
./Core/Src/stm32f4xx_hal_i2c.o \
./Core/Src/stm32f4xx_hal_i2c_ex.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_hal_tim.o \
./Core/Src/stm32f4xx_hal_tim_ex.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tim.o 

C_DEPS += \
./Core/Src/button.d \
./Core/Src/charge_status.d \
./Core/Src/drv2605.d \
./Core/Src/gpio.d \
./Core/Src/haptic.d \
./Core/Src/i2c.d \
./Core/Src/main.d \
./Core/Src/rgb_led.d \
./Core/Src/stm32f4xx_hal_i2c.d \
./Core/Src/stm32f4xx_hal_i2c_ex.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_hal_tim.d \
./Core/Src/stm32f4xx_hal_tim_ex.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tim.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F413xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/button.cyclo ./Core/Src/button.d ./Core/Src/button.o ./Core/Src/button.su ./Core/Src/charge_status.cyclo ./Core/Src/charge_status.d ./Core/Src/charge_status.o ./Core/Src/charge_status.su ./Core/Src/drv2605.cyclo ./Core/Src/drv2605.d ./Core/Src/drv2605.o ./Core/Src/drv2605.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/haptic.cyclo ./Core/Src/haptic.d ./Core/Src/haptic.o ./Core/Src/haptic.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/rgb_led.cyclo ./Core/Src/rgb_led.d ./Core/Src/rgb_led.o ./Core/Src/rgb_led.su ./Core/Src/stm32f4xx_hal_i2c.cyclo ./Core/Src/stm32f4xx_hal_i2c.d ./Core/Src/stm32f4xx_hal_i2c.o ./Core/Src/stm32f4xx_hal_i2c.su ./Core/Src/stm32f4xx_hal_i2c_ex.cyclo ./Core/Src/stm32f4xx_hal_i2c_ex.d ./Core/Src/stm32f4xx_hal_i2c_ex.o ./Core/Src/stm32f4xx_hal_i2c_ex.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_hal_tim.cyclo ./Core/Src/stm32f4xx_hal_tim.d ./Core/Src/stm32f4xx_hal_tim.o ./Core/Src/stm32f4xx_hal_tim.su ./Core/Src/stm32f4xx_hal_tim_ex.cyclo ./Core/Src/stm32f4xx_hal_tim_ex.d ./Core/Src/stm32f4xx_hal_tim_ex.o ./Core/Src/stm32f4xx_hal_tim_ex.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su

.PHONY: clean-Core-2f-Src

