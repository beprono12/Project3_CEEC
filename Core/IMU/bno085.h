#ifndef BNO085_H
#define BNO085_H

#include "stm32f4xx_hal.h"

/* Địa chỉ I2C 7-bit 0x4A, dịch trái 1 bit cho HAL */
#define BNO_ADDR (0x4A << 1)

typedef struct {
    I2C_HandleTypeDef *i2c;

    /* Hardware pins */
    GPIO_TypeDef *intPort;
    uint16_t intPin;
    GPIO_TypeDef *rstPort;
    uint16_t rstPin;

    /* SHTP sequence counters */
    uint8_t seq[6];

    /* Accelerometer (m/s^2) */
    float ax, ay, az;

    /* Gyroscope (rad/s) */
    float gx, gy, gz;

    /* Magnetic field (uT) */
    float mx, my, mz;

    /* Linear acceleration (m/s^2) */
    float lax, lay, laz;

    /* Quaternion */
    float qx, qy, qz, qw;

    /* Euler angles (DEGREE) — yaw chuẩn hóa 0..360 */
    float roll, pitch, yaw;

    /* Calibration status (0-3, 3 = tốt nhất) */
    uint8_t calib_mag;
    uint8_t calib_accel;
    uint8_t calib_gyro;

} BNO085;

void BNO085_Init(BNO085 *imu, I2C_HandleTypeDef *hi2c,
                 GPIO_TypeDef *intPort, uint16_t intPin,
                 GPIO_TypeDef *rstPort, uint16_t rstPin);

void BNO085_EnableAccelerometer(BNO085 *imu, uint32_t interval_us);
void BNO085_EnableGyroscope(BNO085 *imu, uint32_t interval_us);
void BNO085_EnableMagneticField(BNO085 *imu, uint32_t interval_us);
void BNO085_EnableLinearAccel(BNO085 *imu, uint32_t interval_us);
void BNO085_EnableRotationVector(BNO085 *imu, uint32_t interval_us);
void BNO085_EnableARVR(BNO085 *imu, uint32_t interval_us);

void BNO085_Process(BNO085 *imu, volatile uint8_t *dataReadyFlag);
HAL_StatusTypeDef BNO085_ReadPacket(BNO085 *imu);

#endif /* BNO085_H */
