#include "compass.h"
#include <math.h>

static float g_declination    = 0.0f;
static float g_heading_offset = 0.0f;   /* ← THÊM */

void Compass_SetDeclination(float degrees) {
    g_declination = degrees;
}

void Compass_SetHeadingOffset(float degrees) {   /* ← THÊM HÀM NÀY */
    g_heading_offset = degrees;
}

float Compass_GetHeading(BNO085 *imu) {
    float h = -imu->yaw + g_declination + g_heading_offset;   /* ← cộng offset */
    while (h < 0.0f)    h += 360.0f;
    while (h >= 360.0f) h -= 360.0f;
    return h;
}

const char* Compass_GetDirection(float heading) {
    if (heading < 22.5f)   return "N";
    if (heading < 67.5f)   return "NE";
    if (heading < 112.5f)  return "E";
    if (heading < 157.5f)  return "SE";
    if (heading < 202.5f)  return "S";
    if (heading < 247.5f)  return "SW";
    if (heading < 292.5f)  return "W";
    if (heading < 337.5f)  return "NW";
    return "N";
}

uint8_t Compass_IsReady(BNO085 *imu) {
    return (imu->calib_mag >= 3);
}
