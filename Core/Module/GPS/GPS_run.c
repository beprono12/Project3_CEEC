#include "GPS_run.h"
#include "cmsis_os.h"
#include "gps.h"

extern ToaDo viTriHienTai;
extern osMutexId_t dataMutexHandle;

/* ---- GPS: đọc DMA buffer + mỗi 1 s cập nhật vị trí dùng chung ---- */
void GPS_ChayMotVong(void)
{
    static uint32_t tick = 0;

    GPS_Process();                  /* lấy byte mới từ DMA, ghép dòng, parse NMEA */

    if (HAL_GetTick() - tick >= 1000)
    {
        tick = HAL_GetTick();

        if (sys_gps_data.is_valid)  /* chỉ ghi khi đã có fix */
        {
            osMutexAcquire(dataMutexHandle, osWaitForever);
            viTriHienTai.x = sys_gps_data.latitude;
            viTriHienTai.y = sys_gps_data.longitude;
            osMutexRelease(dataMutexHandle);
        }
    }
}

