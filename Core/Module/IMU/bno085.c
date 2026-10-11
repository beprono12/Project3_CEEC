#include "bno085.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

static uint8_t tx[32];

/* Q14: 2^14 = 16384 */
static float q14(int16_t v) { return ((float) v) / 16384.0f; }

/* Q9: 2^9 = 512 — gyro (rad/s) */
static float q9(int16_t v)  { return ((float) v) / 512.0f; }

/* Q8: 2^8 = 256 — accel / linear accel */
static float q8(int16_t v)  { return ((float) v) / 256.0f; }

/* Q4: 2^4 = 16 — magnetic field (uT) */
static float q4(int16_t v)  { return ((float) v) / 16.0f; }

static void quaternionToEuler(BNO085 *imu) {
    const float RAD2DEG = 57.2957795f;
    float x = imu->qx, y = imu->qy, z = imu->qz, w = imu->qw;

    imu->roll  = atan2f(2 * (w * x + y * z), 1 - 2 * (x * x + y * y)) * RAD2DEG;
    imu->pitch = asinf (2 * (w * y - z * x))                          * RAD2DEG;
    imu->yaw   = atan2f(2 * (w * z + x * y), 1 - 2 * (y * y + z * z)) * RAD2DEG;

    if (imu->yaw < 0.0f) imu->yaw += 360.0f;
}

static void sendPacket(BNO085 *imu, uint8_t channel, uint8_t *data, uint16_t len) {
    uint16_t plen = len + 4;
    plen &= 0x7FFF;

    tx[0] = plen & 0xFF;
    tx[1] = (plen >> 8) & 0x7F;
    tx[2] = channel;
    tx[3] = imu->seq[channel]++;

    memcpy(&tx[4], data, len);

    HAL_I2C_Master_Transmit(imu->i2c, BNO_ADDR, tx, plen, 100);
}

static void setFeature(BNO085 *imu, uint8_t report, uint32_t interval) {
    uint8_t cmd[17];
    memset(cmd, 0, sizeof(cmd));

    cmd[0] = 0xFD;
    cmd[1] = report;
    cmd[2] = 0x00;
    cmd[3] = 0x00;
    cmd[4] = 0x00;

    cmd[5] = interval & 0xFF;
    cmd[6] = (interval >> 8)  & 0xFF;
    cmd[7] = (interval >> 16) & 0xFF;
    cmd[8] = (interval >> 24) & 0xFF;

    sendPacket(imu, 2, cmd, 17);
}

void BNO085_EnableAccelerometer(BNO085 *imu, uint32_t interval_us) {
    setFeature(imu, 0x01, interval_us);
}
void BNO085_EnableGyroscope(BNO085 *imu, uint32_t interval_us) {
    setFeature(imu, 0x02, interval_us);
}
void BNO085_EnableMagneticField(BNO085 *imu, uint32_t interval_us) {
    setFeature(imu, 0x03, interval_us);
}
void BNO085_EnableLinearAccel(BNO085 *imu, uint32_t interval_us) {
    setFeature(imu, 0x04, interval_us);
}
void BNO085_EnableRotationVector(BNO085 *imu, uint32_t interval_us) {
    setFeature(imu, 0x05, interval_us);
}
void BNO085_EnableARVR(BNO085 *imu, uint32_t interval_us) {
    setFeature(imu, 0x08, interval_us);
}

void BNO085_Init(BNO085 *imu, I2C_HandleTypeDef *hi2c,
                 GPIO_TypeDef *intPort, uint16_t intPin,
                 GPIO_TypeDef *rstPort, uint16_t rstPin) {
    imu->i2c     = hi2c;
    imu->intPort = intPort;
    imu->intPin  = intPin;
    imu->rstPort = rstPort;
    imu->rstPin  = rstPin;

    memset(imu->seq, 0, 6);

    HAL_GPIO_WritePin(imu->rstPort, imu->rstPin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(imu->rstPort, imu->rstPin, GPIO_PIN_SET);
    HAL_Delay(600);
}

void BNO085_Process(BNO085 *imu, volatile uint8_t *dataReadyFlag) {
    if (*dataReadyFlag) {
        *dataReadyFlag = 0;

        for (uint8_t n = 0; n < 8; n++) {
            if (HAL_GPIO_ReadPin(imu->intPort, imu->intPin) != GPIO_PIN_RESET) break;
            if (BNO085_ReadPacket(imu) != HAL_OK) break;
        }
    }
}

static void parse(BNO085 *imu, uint8_t *payload, uint16_t payloadLen) {
    if (payloadLen < 5) return;

    uint16_t i = 5;

    while (i < payloadLen) {
        uint8_t id = payload[i];

        if (id == 0x01) {           /* Accelerometer 10B Q8 */
            if (i + 10 > payloadLen) break;
            imu->ax = q8((int16_t)(payload[i + 4] | (payload[i + 5] << 8)));
            imu->ay = q8((int16_t)(payload[i + 6] | (payload[i + 7] << 8)));
            imu->az = q8((int16_t)(payload[i + 8] | (payload[i + 9] << 8)));
            i += 10;

        } else if (id == 0x02) {    /* Gyroscope 10B Q9 */
            if (i + 10 > payloadLen) break;
            imu->gx = q9((int16_t)(payload[i + 4] | (payload[i + 5] << 8)));
            imu->gy = q9((int16_t)(payload[i + 6] | (payload[i + 7] << 8)));
            imu->gz = q9((int16_t)(payload[i + 8] | (payload[i + 9] << 8)));
            i += 10;

        } else if (id == 0x03) {    /* Magnetic Field 10B Q4 */
            if (i + 10 > payloadLen) break;
            imu->mx = q4((int16_t)(payload[i + 4] | (payload[i + 5] << 8)));
            imu->my = q4((int16_t)(payload[i + 6] | (payload[i + 7] << 8)));
            imu->mz = q4((int16_t)(payload[i + 8] | (payload[i + 9] << 8)));
            i += 10;

        } else if (id == 0x04) {    /* Linear Accel 10B Q8 */
            if (i + 10 > payloadLen) break;
            imu->lax = q8((int16_t)(payload[i + 4] | (payload[i + 5] << 8)));
            imu->lay = q8((int16_t)(payload[i + 6] | (payload[i + 7] << 8)));
            imu->laz = q8((int16_t)(payload[i + 8] | (payload[i + 9] << 8)));
            i += 10;

        } else if (id == 0x05) {    /* Rotation Vector 14B Q14 */
            if (i + 14 > payloadLen) break;

            uint8_t status = payload[i + 2];
            imu->calib_mag   =  status        & 0x03;
            imu->calib_accel = (status >> 2)  & 0x03;
            imu->calib_gyro  = (status >> 4)  & 0x03;

            imu->qx = q14((int16_t)(payload[i + 4]  | (payload[i + 5]  << 8)));
            imu->qy = q14((int16_t)(payload[i + 6]  | (payload[i + 7]  << 8)));
            imu->qz = q14((int16_t)(payload[i + 8]  | (payload[i + 9]  << 8)));
            imu->qw = q14((int16_t)(payload[i + 10] | (payload[i + 11] << 8)));

            quaternionToEuler(imu);
            i += 14;

        } else if (id == 0x08) {    /* ARVR Stabilized RV 14B */
            i += 14;

        } else {
            break;
        }
    }
}




HAL_StatusTypeDef BNO085_ReadPacket(BNO085 *imu) {
    static uint8_t packet[300];
    HAL_StatusTypeDef status;

    status = HAL_I2C_Master_Receive(imu->i2c, BNO_ADDR, packet, 4, 100);
    if (status != HAL_OK) return status;

    uint16_t len = ((uint16_t)packet[1] << 8) | packet[0];
    len &= 0x7FFF;

    if (len < 4 || len > sizeof(packet)) return HAL_ERROR;

    status = HAL_I2C_Master_Receive(imu->i2c, BNO_ADDR, packet, len, 100);
    if (status != HAL_OK) return status;

    uint8_t channel = packet[2];
    uint8_t seq     = packet[3];

    imu->seq[channel] = seq;

    if (channel == 3) {
        parse(imu, &packet[4], len - 4);
    }

    return HAL_OK;
}
