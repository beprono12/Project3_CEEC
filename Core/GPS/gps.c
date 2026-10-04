#include "gps.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#if GPS_DEBUG_RAW
extern UART_HandleTypeDef huart1;
#endif

#define GPS_RX_BUF_SIZE   1024
#define GPS_LINE_BUF_SIZE 128
#define EARTH_RADIUS_M    6371000.0
#define PI_D              3.14159265358979323846
#define DEG_TO_RAD(d)     ((d) * (PI_D / 180.0))
#define RAD_TO_DEG(r)     ((r) * (180.0 / PI_D))

GPS_Data_t sys_gps_data = {0};

static UART_HandleTypeDef *gps_huart;
static uint8_t  dma_buf[GPS_RX_BUF_SIZE];
static uint16_t old_pos = 0;

static char     line_buf[GPS_LINE_BUF_SIZE];
static uint16_t line_pos = 0;

static double nmea_to_decimal(const char *raw, char dir){
    if (raw[0] == '\0') return 0.0;

    double val     = atof(raw);
    int    deg     = (int)(val / 100.0);
    double min     = val - (deg * 100.0);
    double decimal = deg + (min / 60.0);

    if (dir == 'S' || dir == 'W') decimal = -decimal;
    return decimal;
}

static int nmea_split(char *s, char *f[], int max){
    int n = 0;
    f[n++] = s;
    while (*s) {
        if (*s == ',') {
            *s = '\0';
            if (n < max) f[n++] = s + 1;
        } else if (*s == '*') {
            *s = '\0';
            break;
        }
        s++;
    }
    return n;
}

static bool nmea_checksum_ok(const char *line)
{
    if (line[0] != '$') return false;

    uint8_t cs = 0;
    const char *p = line + 1;
    while (*p && *p != '*') cs ^= (uint8_t)*p++;
    if (*p != '*') return false;

    return cs == (uint8_t)strtoul(p + 1, NULL, 16);
}

static void parse_GGA(char *line)
{
    char *f[15] = {0};
    int n = nmea_split(line, f, 15);
    if (n < 10) return;

    sys_gps_data.fix_quality = (uint8_t)atoi(f[6]);
    sys_gps_data.satellites  = (uint8_t)atoi(f[7]);

    if (sys_gps_data.fix_quality > 0 &&
        f[2][0] && f[3][0] && f[4][0] && f[5][0]) {
        sys_gps_data.latitude  = nmea_to_decimal(f[2], f[3][0]);
        sys_gps_data.longitude = nmea_to_decimal(f[4], f[5][0]);
        sys_gps_data.altitude  = (float)atof(f[9]);
        sys_gps_data.is_valid  = true;
    } else {
        sys_gps_data.is_valid = false;
    }
}

static uint8_t two_digits(const char *s){
    return (uint8_t)((s[0] - '0') * 10 + (s[1] - '0'));
}

static void parse_RMC(char *line){
    char *f[13] = {0};
    int n = nmea_split(line, f, 13);
    if (n < 10) return;
    if (f[2][0] != 'A') return;
    if (strlen(f[1]) >= 6){
        sys_gps_data.hour   = two_digits(&f[1][0]);
        sys_gps_data.minute = two_digits(&f[1][2]);
        sys_gps_data.second = two_digits(&f[1][4]);
    }
    if (strlen(f[9]) >= 6){
        sys_gps_data.day   = two_digits(&f[9][0]);
        sys_gps_data.month = two_digits(&f[9][2]);
        sys_gps_data.year  = (uint16_t)(2000 + two_digits(&f[9][4]));
    }
    sys_gps_data.speed_kmh = (float)(atof(f[7]) * 1.852);
    sys_gps_data.course    = (float)atof(f[8]);
}

static void gps_parse_line(char *line){
    if (strlen(line) < 6) return;
    if (!nmea_checksum_ok(line)) return;   /* bo dong hong */
	#if GPS_DEBUG_RAW
		HAL_UART_Transmit(&huart1, (uint8_t *)line, strlen(line), 100);
		HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, 10);
	#endif
    if (strncmp(&line[3], "GGA", 3) == 0)      parse_GGA(line);
    else if (strncmp(&line[3], "RMC", 3) == 0) parse_RMC(line);
}

void GPS_Init(UART_HandleTypeDef *huart){
    gps_huart = huart;
    old_pos   = 0;
    line_pos  = 0;
    memset(&sys_gps_data, 0, sizeof(sys_gps_data));
    HAL_UART_Receive_DMA(gps_huart, dma_buf, GPS_RX_BUF_SIZE);
}

void GPS_Process(void){
    uint16_t new_pos = GPS_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(gps_huart->hdmarx);
    if (new_pos >= GPS_RX_BUF_SIZE) new_pos = 0;

    while (old_pos != new_pos){
        char c = (char)dma_buf[old_pos];
        old_pos = (old_pos + 1) % GPS_RX_BUF_SIZE;

        if (c == '\r') {
        } else if (c == '\n'){
            if (line_pos > 0){
                line_buf[line_pos] = '\0';
                if (line_buf[0] == '$') gps_parse_line(line_buf);
            }
            line_pos = 0;
        }else{
            if (line_pos < (GPS_LINE_BUF_SIZE - 1)) {
                line_buf[line_pos++] = c;
            }else{
                line_pos = 0;
            }
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
    if (huart == gps_huart) {
        if (HAL_UART_Receive_DMA(gps_huart, dma_buf, GPS_RX_BUF_SIZE) == HAL_OK) {
            old_pos  = 0;
            line_pos = 0;
        }
    }
}

// BẮT BUỘC dùng double ở tham số truyền vào
float GPS_Calculate_Bearing(double lat1, double lon1, double lat2, double lon2){

    // Đổi vĩ độ hiện tại sang Radian để tính tỷ lệ bù
    double lat1_rad = lat1 * M_PI / 180.0;

    double dx = lat2 - lat1;
    // Nhân thêm cos(lat) để bù độ cong trái đất cho trục Đông/Tây
    double dy = (lon2 - lon1) * cos(lat1_rad);

    if (dx == 0.0 && dy == 0.0) return 0.0f; // Tránh nhiễu chia 0

    // Vẫn gọi atan2(dy, dx) để lấy chuẩn La bàn
    float b = RAD_TO_DEG(atan2(dy, dx));

    if (b < 0.0f) b += 360.0f;
    return b;
}

float GPS_Calculate_Distance(double lat1, double lon1, double lat2, double lon2){

	double phi1 = DEG_TO_RAD(lat1);
	double phi2 = DEG_TO_RAD(lat2);
	double dp   = DEG_TO_RAD(lat2 - lat1);
	double dl   = DEG_TO_RAD(lon2 - lon1);

	double sdp = sin(dp / 2.0);
	double sdl = sin(dl / 2.0);
	double a   = sdp * sdp + cos(phi1) * cos(phi2) * sdl * sdl;
	if (a > 1.0) a = 1.0;

	double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return (float)(EARTH_RADIUS_M * c);
}
