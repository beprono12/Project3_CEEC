################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/IMU/icm20948.c \
../Core/IMU/imu_yaw.c \
../Core/IMU/mag_ak09916.c \
../Core/IMU/stationary.c \
../Core/IMU/yaw_fusion.c 

OBJS += \
./Core/IMU/icm20948.o \
./Core/IMU/imu_yaw.o \
./Core/IMU/mag_ak09916.o \
./Core/IMU/stationary.o \
./Core/IMU/yaw_fusion.o 

C_DEPS += \
./Core/IMU/icm20948.d \
./Core/IMU/imu_yaw.d \
./Core/IMU/mag_ak09916.d \
./Core/IMU/stationary.d \
./Core/IMU/yaw_fusion.d 


# Each subdirectory must supply rules for building sources it contributes
Core/IMU/%.o Core/IMU/%.su Core/IMU/%.cyclo: ../Core/IMU/%.c Core/IMU/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Core/NRF -I../Core/IMU -I../Core/TFT -I../Core/GPS -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-IMU

clean-Core-2f-IMU:
	-$(RM) ./Core/IMU/icm20948.cyclo ./Core/IMU/icm20948.d ./Core/IMU/icm20948.o ./Core/IMU/icm20948.su ./Core/IMU/imu_yaw.cyclo ./Core/IMU/imu_yaw.d ./Core/IMU/imu_yaw.o ./Core/IMU/imu_yaw.su ./Core/IMU/mag_ak09916.cyclo ./Core/IMU/mag_ak09916.d ./Core/IMU/mag_ak09916.o ./Core/IMU/mag_ak09916.su ./Core/IMU/stationary.cyclo ./Core/IMU/stationary.d ./Core/IMU/stationary.o ./Core/IMU/stationary.su ./Core/IMU/yaw_fusion.cyclo ./Core/IMU/yaw_fusion.d ./Core/IMU/yaw_fusion.o ./Core/IMU/yaw_fusion.su

.PHONY: clean-Core-2f-IMU

