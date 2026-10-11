################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Module/NRF/NRF24.c \
../Core/Module/NRF/NRF_comm.c 

OBJS += \
./Core/Module/NRF/NRF24.o \
./Core/Module/NRF/NRF_comm.o 

C_DEPS += \
./Core/Module/NRF/NRF24.d \
./Core/Module/NRF/NRF_comm.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Module/NRF/%.o Core/Module/NRF/%.su Core/Module/NRF/%.cyclo: ../Core/Module/NRF/%.c Core/Module/NRF/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Core/Module/NRF -I../Core/Module/TFT -I../Core/Module/DrawInterface -I../Core/Module/IMU -I../Core/Module/GPS -I../Core/Module/BUZZER -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Module-2f-NRF

clean-Core-2f-Module-2f-NRF:
	-$(RM) ./Core/Module/NRF/NRF24.cyclo ./Core/Module/NRF/NRF24.d ./Core/Module/NRF/NRF24.o ./Core/Module/NRF/NRF24.su ./Core/Module/NRF/NRF_comm.cyclo ./Core/Module/NRF/NRF_comm.d ./Core/Module/NRF/NRF_comm.o ./Core/Module/NRF/NRF_comm.su

.PHONY: clean-Core-2f-Module-2f-NRF

