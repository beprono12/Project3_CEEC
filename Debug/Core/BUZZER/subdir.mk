################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/BUZZER/Buzzer.c 

OBJS += \
./Core/BUZZER/Buzzer.o 

C_DEPS += \
./Core/BUZZER/Buzzer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/BUZZER/%.o Core/BUZZER/%.su Core/BUZZER/%.cyclo: ../Core/BUZZER/%.c Core/BUZZER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Core/NRF -I../Core/IMU -I../Core/TFT -I../Core/GPS -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Core/DrawInterface -I../Core/BUZZER -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-BUZZER

clean-Core-2f-BUZZER:
	-$(RM) ./Core/BUZZER/Buzzer.cyclo ./Core/BUZZER/Buzzer.d ./Core/BUZZER/Buzzer.o ./Core/BUZZER/Buzzer.su

.PHONY: clean-Core-2f-BUZZER

