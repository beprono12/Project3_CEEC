#ifndef COMPASS_H
#define COMPASS_H

#include "bno085.h"

/* Trả về heading 0..360° (0 = Bắc) */
float Compass_GetHeading(BNO085 *imu);

/* Đổi heading thành hướng chữ: "N", "NE", "E", ... */
const char* Compass_GetDirection(float heading);

/* Kiểm tra la bàn đã sẵn sàng (calib_mag >= 3) */
uint8_t Compass_IsReady(BNO085 *imu);

/* Bù từ thiên (magnetic declination) nếu cần */
void Compass_SetDeclination(float degrees);

void Compass_SetHeadingOffset(float degrees);
#endif
