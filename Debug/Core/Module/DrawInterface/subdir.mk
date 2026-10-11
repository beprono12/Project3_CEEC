################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Module/DrawInterface/DrawInterface.c 

OBJS += \
./Core/Module/DrawInterface/DrawInterface.o 

C_DEPS += \
./Core/Module/DrawInterface/DrawInterface.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Module/DrawInterface/%.o Core/Module/DrawInterface/%.su Core/Module/DrawInterface/%.cyclo: ../Core/Module/DrawInterface/%.c Core/Module/DrawInterface/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Core/Module/NRF -I../Core/Module/TFT -I../Core/Module/DrawInterface -I../Core/Module/IMU -I../Core/Module/GPS -I../Core/Module/BUZZER -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Module-2f-DrawInterface

clean-Core-2f-Module-2f-DrawInterface:
	-$(RM) ./Core/Module/DrawInterface/DrawInterface.cyclo ./Core/Module/DrawInterface/DrawInterface.d ./Core/Module/DrawInterface/DrawInterface.o ./Core/Module/DrawInterface/DrawInterface.su

.PHONY: clean-Core-2f-Module-2f-DrawInterface

