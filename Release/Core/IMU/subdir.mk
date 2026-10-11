################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/IMU/bno085.c \
../Core/IMU/compass.c 

OBJS += \
./Core/IMU/bno085.o \
./Core/IMU/compass.o 

C_DEPS += \
./Core/IMU/bno085.d \
./Core/IMU/compass.d 


# Each subdirectory must supply rules for building sources it contributes
Core/IMU/%.o Core/IMU/%.su Core/IMU/%.cyclo: ../Core/IMU/%.c Core/IMU/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-IMU

clean-Core-2f-IMU:
	-$(RM) ./Core/IMU/bno085.cyclo ./Core/IMU/bno085.d ./Core/IMU/bno085.o ./Core/IMU/bno085.su ./Core/IMU/compass.cyclo ./Core/IMU/compass.d ./Core/IMU/compass.o ./Core/IMU/compass.su

.PHONY: clean-Core-2f-IMU

