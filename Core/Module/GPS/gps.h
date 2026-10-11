#ifndef GPS_H
#define GPS_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define GPS_DEBUG_RAW 0

typedef struct {
    double   latitude;
    double   longitude;
    float    altitude;
    float    speed_kmh;
    float    course;
    uint8_t  satellites;
    uint8_t  fix_quality;
    uint8_t  hour, minute, second;
    uint8_t  day, month;
    uint16_t year;
    bool     is_valid;
} GPS_Data_t;

extern GPS_Data_t sys_gps_data;

void  GPS_Init(UART_HandleTypeDef *huart);
void  GPS_Process(void);
float GPS_Calculate_Distance(double lat1, double lon1, double lat2, double lon2);
float GPS_Calculate_Bearing(double lat1, double lon1, double lat2, double lon2);

#endif
