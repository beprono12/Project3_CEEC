################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/DrawInterface/DrawInterface.c 

OBJS += \
./Core/DrawInterface/DrawInterface.o 

C_DEPS += \
./Core/DrawInterface/DrawInterface.d 


# Each subdirectory must supply rules for building sources it contributes
Core/DrawInterface/%.o Core/DrawInterface/%.su Core/DrawInterface/%.cyclo: ../Core/DrawInterface/%.c Core/DrawInterface/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-DrawInterface

clean-Core-2f-DrawInterface:
	-$(RM) ./Core/DrawInterface/DrawInterface.cyclo ./Core/DrawInterface/DrawInterface.d ./Core/DrawInterface/DrawInterface.o ./Core/DrawInterface/DrawInterface.su

.PHONY: clean-Core-2f-DrawInterface

