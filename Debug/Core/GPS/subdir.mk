################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/GPS/gps.c 

OBJS += \
./Core/GPS/gps.o 

C_DEPS += \
./Core/GPS/gps.d 


# Each subdirectory must supply rules for building sources it contributes
Core/GPS/%.o Core/GPS/%.su Core/GPS/%.cyclo: ../Core/GPS/%.c Core/GPS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Core/NRF -I../Core/IMU -I../Core/TFT -I../Core/GPS -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-GPS

clean-Core-2f-GPS:
	-$(RM) ./Core/GPS/gps.cyclo ./Core/GPS/gps.d ./Core/GPS/gps.o ./Core/GPS/gps.su

.PHONY: clean-Core-2f-GPS

