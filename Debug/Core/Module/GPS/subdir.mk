################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Module/GPS/GPS_run.c \
../Core/Module/GPS/gps.c 

OBJS += \
./Core/Module/GPS/GPS_run.o \
./Core/Module/GPS/gps.o 

C_DEPS += \
./Core/Module/GPS/GPS_run.d \
./Core/Module/GPS/gps.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Module/GPS/%.o Core/Module/GPS/%.su Core/Module/GPS/%.cyclo: ../Core/Module/GPS/%.c Core/Module/GPS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Core/Module/NRF -I../Core/Module/TFT -I../Core/Module/DrawInterface -I../Core/Module/IMU -I../Core/Module/GPS -I../Core/Module/BUZZER -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Module-2f-GPS

clean-Core-2f-Module-2f-GPS:
	-$(RM) ./Core/Module/GPS/GPS_run.cyclo ./Core/Module/GPS/GPS_run.d ./Core/Module/GPS/GPS_run.o ./Core/Module/GPS/GPS_run.su ./Core/Module/GPS/gps.cyclo ./Core/Module/GPS/gps.d ./Core/Module/GPS/gps.o ./Core/Module/GPS/gps.su

.PHONY: clean-Core-2f-Module-2f-GPS

