################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/NRF/NRF24.c \
../Core/NRF/NRF_comm.c 

OBJS += \
./Core/NRF/NRF24.o \
./Core/NRF/NRF_comm.o 

C_DEPS += \
./Core/NRF/NRF24.d \
./Core/NRF/NRF_comm.d 


# Each subdirectory must supply rules for building sources it contributes
Core/NRF/%.o Core/NRF/%.su Core/NRF/%.cyclo: ../Core/NRF/%.c Core/NRF/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-NRF

clean-Core-2f-NRF:
	-$(RM) ./Core/NRF/NRF24.cyclo ./Core/NRF/NRF24.d ./Core/NRF/NRF24.o ./Core/NRF/NRF24.su ./Core/NRF/NRF_comm.cyclo ./Core/NRF/NRF_comm.d ./Core/NRF/NRF_comm.o ./Core/NRF/NRF_comm.su

.PHONY: clean-Core-2f-NRF

