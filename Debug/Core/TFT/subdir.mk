################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/TFT/gc9a01a.c \
../Core/TFT/tft_gc9a01.c 

OBJS += \
./Core/TFT/gc9a01a.o \
./Core/TFT/tft_gc9a01.o 

C_DEPS += \
./Core/TFT/gc9a01a.d \
./Core/TFT/tft_gc9a01.d 


# Each subdirectory must supply rules for building sources it contributes
Core/TFT/%.o Core/TFT/%.su Core/TFT/%.cyclo: ../Core/TFT/%.c Core/TFT/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Core/NRF -I../Core/IMU -I../Core/TFT -I../Core/GPS -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Core/DrawInterface -I../Core/BUZZER -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-TFT

clean-Core-2f-TFT:
	-$(RM) ./Core/TFT/gc9a01a.cyclo ./Core/TFT/gc9a01a.d ./Core/TFT/gc9a01a.o ./Core/TFT/gc9a01a.su ./Core/TFT/tft_gc9a01.cyclo ./Core/TFT/tft_gc9a01.d ./Core/TFT/tft_gc9a01.o ./Core/TFT/tft_gc9a01.su

.PHONY: clean-Core-2f-TFT

