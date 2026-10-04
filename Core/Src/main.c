/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "tft_gc9a01.h"
#include "NRF24.h"
#include "NRF24_reg_addresses.h"
#include "gps.h"

/* Lưu ý! math.h, string.h đã được include trong các file.h ở trên */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Màn hình đang hiển thị */
typedef enum {
    SCREEN_RADAR,
    SCREEN_MESSAGE
} TrangThaiManHinh;


/* Gói dữ liệu gửi: Node -> Trạm ESP32 */
typedef struct __attribute__((packed)) {
    uint16_t header;
	double lat;   /* vĩ độ */
    double lon;   /* kinh độ */
    float yaw;   /* góc yaw */
    uint8_t footer;
} DuLieuGui;


/* Gói dữ liệu nhận: Trạm ESP32 -> Node */
typedef struct __attribute__((packed)) {
	uint16_t header; // check
    double dest_lat;       /* vĩ độ đích */
    double dest_lon;       /* kinh độ đích */
    char  dest_name[13];  /* tên địa điểm đích */
    uint8_t footer;
} DuLieuNhan;
 _Static_assert(sizeof(DuLieuNhan) == 32, "ACK plod phai <= 32 byte");

/* Tọa độ: x = vĩ độ (lat), y = kinh độ (lon) */
typedef struct { double x; double y; } ToaDo;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Định nghĩa "Khóa" bảo vệ gói tin
#define GIA_TRI_HEADER 0xAA55
#define GIA_TRI_FOOTER 0xBB

#define PIN_NUT_BAM    PB0   /* chân nút bấm đổi màn hình */

/* Màu giao diện radar */
#define MAU_NEN        GC9A01A_GREEN
#define MAU_LUOI       GC9A01A_BLACK
#define MAU_MUI_TEN    GC9A01A_ORANGE
#define MAU_DICH       GC9A01A_WHITE
#define MAU_TEXT       GC9A01A_BLACK


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

TIM_HandleTypeDef htim10;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for GPSTask */
osThreadId_t GPSTaskHandle;
const osThreadAttr_t GPSTask_attributes = {
  .name = "GPSTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for NRFTask */
osThreadId_t NRFTaskHandle;
const osThreadAttr_t NRFTask_attributes = {
  .name = "NRFTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for DisplayTask */
osThreadId_t DisplayTaskHandle;
const osThreadAttr_t DisplayTask_attributes = {
  .name = "DisplayTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for IMUTask */
osThreadId_t IMUTaskHandle;
const osThreadAttr_t IMUTask_attributes = {
  .name = "IMUTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for dataMutex */
osMutexId_t dataMutexHandle;
const osMutexAttr_t dataMutex_attributes = {
  .name = "dataMutex"
};
/* Definitions for nrfIrqSem */
osSemaphoreId_t nrfIrqSemHandle;
const osSemaphoreAttr_t nrfIrqSem_attributes = {
  .name = "nrfIrqSem"
};
/* USER CODE BEGIN PV */

extern osSemaphoreId_t nrfIrqSemHandle;   /* ngắt NRF (PA1) báo cho NRFTask   */
extern osMutexId_t     dataMutexHandle;   /* bảo vệ viTriHienTai, HuongDi,
                                             fromESP32, toESP32 (dữ liệu dùng
                                             chung giữa các task) */

/* --- Kích thước giao diện radar --- */
const int16_t TAM_X = 120;
const int16_t TAM_Y = 120;
const int16_t BAN_KINH_RADAR = 100;
const float   MAX_RANGE_M = 50.0;
const int16_t KHOANG_CACH_LUOI = 20;


/* --- Màn hình TFT --- */
GC9A01A tft;
extern SPI_HandleTypeDef hspi1;
volatile TrangThaiManHinh manHinhHienTai = SCREEN_RADAR;
TrangThaiManHinh manHinhTruocDo = SCREEN_MESSAGE;
uint8_t  canXoaManHinh = 1;       /* 1 = cần vẽ lại nền */
uint32_t thoiGianQuetRadar = 0;


/* --- Nút bấm --- */
volatile uint32_t last_press = 0; /* thời điểm bấm nút lần cuối */


/* --- NRF24 --- */
DuLieuGui  toESP32;
DuLieuNhan fromESP32;
uint8_t    diaChiNRF[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
uint32_t   thoiGianGuiNRF = 0;


/* --- Vị trí và hướng đi --- */
ToaDo viTriHienTai = {0.0f, 0.0f};   /* vị trí ban đầu */
float HuongDi = 0.0f;  /* hướng ban đầu */


/* --- IMU --- */
uint8_t imu_ok = 0;                  /* 1 = IMU khởi tạo thành công */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM10_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI3_Init(void);
void StartDefaultTask(void *argument);
void StartGPSTask(void *argument);
void StartNRFTask(void *argument);
void StartDisplayTask(void *argument);
void StartTask05(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void Debug_Print(const char *str) {
   (void)str;
}

/* Lấy tọa độ đích đã nhận từ Trạm ESP32 */
ToaDo layToaDoDich()    {
	ToaDo td = {fromESP32.dest_lat,  fromESP32.dest_lon};
    return td;
}

/* ========== NRF24 ========== */

void KhoiTao_NRF24(void) {
    nrf24_init();

    /*Sóng: kênh, tốc độ, công suất, crc*/
    nrf24_set_channel(90);
    nrf24_data_rate(_1mbps);
    nrf24_tx_pwr(_0dbm);
    nrf24_set_crc(en_crc, _1byte);

    /*Payload động và ACK kèm dữ liệu*/
    nrf24_dpl(enable);
    nrf24_set_rx_dpl(0, enable);
    nrf24_en_ack_pld(enable);

    /*Tự gửi lại khi chưa nhận được ACK*/
    nrf24_auto_retr_delay(5);
    nrf24_auto_retr_limit(15);

    /*Mở đường gửi và nhận (cùng địa chỉ đã khai báo)*/
    nrf24_open_tx_pipe(diaChiNRF);
    nrf24_open_rx_pipe(0, diaChiNRF);

    /*Xóa bộ đệm cũ*/
    nrf24_flush_rx(); // Xả rác bộ đệm nhận
    nrf24_flush_tx(); // Xả rác bộ đệm phát

    nrf24_stop_listen();
}


// CÁC HÀM VẼ GIAO DIỆN MÀN HÌNH

/* ========== GIAO DIỆN 1: RADAR ========== */

/*Vẽ nền radar*/
void veNenRadar() {
  TFT_FillScreen(&tft, MAU_NEN);

  for (int16_t x = 0; x < TFT_WIDTH; x += KHOANG_CACH_LUOI) {
      TFT_DrawFastVLine(&tft, x, 0, 240, MAU_LUOI);
  }
  for (int16_t y = 0; y < TFT_HEIGHT; y += KHOANG_CACH_LUOI) {
      TFT_DrawFastHLine(&tft, 0, y, 240, MAU_LUOI);
  }
}

/*Vẽ mũi tên thiết bị ở tâm*/
void veMuiTenThietBi() {
  int16_t dai = 12; int16_t rong = 8;
  int16_t x0 = TAM_X, y0 = TAM_Y - dai;
  int16_t x1 = TAM_X - rong, y1 = TAM_Y + dai / 2;
  int16_t x2 = TAM_X + rong, y2 = TAM_Y + dai / 2;
  TFT_FillTriangle(&tft, x0, y0, x1, y1, x2, y2, MAU_MUI_TEN);
}

/*Vẽ chấm đích */
void veChamDich(float khoangCach, float gocTuongDoi) {

  //static giữ nguyên giá trị vị trí cũ sau mỗi lần thoát hàm
  static int16_t x_cu = -1, y_cu = -1;
  static uint8_t ngoaiPhamVi_cu = 0;

  // Xóa chấm sáng cũ bằng màu nền
  if (x_cu != -1 && y_cu != -1) {
      TFT_FillCircle(&tft, x_cu, y_cu, 5, MAU_NEN);
      if (ngoaiPhamVi_cu) TFT_DrawCircle(&tft, x_cu, y_cu, 7, MAU_NEN);
  }

  // Tính vị trí mới
  float khoangCachVeMan = (khoangCach > MAX_RANGE_M) ? MAX_RANGE_M : khoangCach;
  uint8_t ngoaiPhamVi = (khoangCach > MAX_RANGE_M);
  float banKinhPx = (khoangCachVeMan / MAX_RANGE_M) * BAN_KINH_RADAR;
  float rad = gocTuongDoi * M_PI / 180.0;

  int16_t x = TAM_X + (int16_t)(banKinhPx * sinf(rad));
  int16_t y = TAM_Y - (int16_t)(banKinhPx * cosf(rad));

  //Vẽ chấm sáng mới
  TFT_FillCircle(&tft, x, y, 5, MAU_DICH);
  if (ngoaiPhamVi) TFT_DrawCircle(&tft, x, y, 7, MAU_DICH);

  //Cập nhật lại giá trị để xóa lần sau
  x_cu = x;
  y_cu = y;
  ngoaiPhamVi_cu = ngoaiPhamVi;
}


/* In khoảng cách*/
void veKhoangCach(float khoangCach) {
  char buf[24];
  if (khoangCach <= 4.0) {
    snprintf(buf, sizeof(buf), "    ARRIVED!   ");
  }
  else {
    if (khoangCach < 1000.0) {
      int phanNguyen = (int)khoangCach;
      int phanThapPhan = (int)((khoangCach - (float)phanNguyen) * 10);
      if(phanThapPhan < 0) phanThapPhan = -phanThapPhan;
      snprintf(buf, sizeof(buf), "   %d.%d m   ", phanNguyen, phanThapPhan);
    }
    else {
      float kcVm = khoangCach / 1000.0;
      int phanNguyen = (int)kcVm;
      int phanThapPhan = (int)((kcVm - (float)phanNguyen) * 100);
      if(phanThapPhan < 0) phanThapPhan = -phanThapPhan;
      snprintf(buf, sizeof(buf), "   %d.%02d km  ", phanNguyen, phanThapPhan);
    }
  }
  TFT_SetTextSize(2);
  TFT_SetTextDatum(BC_DATUM);
  TFT_DrawString(&tft, buf, TAM_X, TAM_Y + 80, MAU_TEXT, MAU_NEN);
}


/* Cập nhật toàn bộ giao diện Radar */
void capNhatRadar() {
  if (canXoaManHinh) {
    veNenRadar();
    canXoaManHinh = 0;
  }

  //Lấy vị trí, hướng hiện tại và vẽ thiết bị ở tâm
  osMutexAcquire(dataMutexHandle, osWaitForever);
  ToaDo      viTriThietBi = viTriHienTai;
  float      huongDi      = HuongDi;
  DuLieuNhan nhan         = fromESP32;        /* sao chép cả gói đích */
  osMutexRelease(dataMutexHandle);

  veMuiTenThietBi();

  //  Chỉ tính toán và vẽ chấm sáng NẾU đã nhận đích từ Trạm
  if (nhan.dest_name[0] != '\0') {
      ToaDo viTriDich = {nhan.dest_lat, nhan.dest_lon };
      float khoangCach = GPS_Calculate_Distance(viTriThietBi.x, viTriThietBi.y , viTriDich.x, viTriDich.y);
      float gocPhuongVi = GPS_Calculate_Bearing(viTriThietBi.x, viTriThietBi.y , viTriDich.x, viTriDich.y);
      float gocTuongDoi = gocPhuongVi - huongDi;

      if (gocTuongDoi < 0) gocTuongDoi += 360.0;

      veChamDich(khoangCach, gocTuongDoi); // Vẽ chấm sáng
      veKhoangCach(khoangCach);            // In khoảng cách
  }
  else {
      // Chưa có đích: Không vẽ chấm sáng + hiện SEARCHING
      TFT_SetTextSize(2);
      TFT_SetTextDatum(BC_DATUM);
      TFT_DrawString(&tft, "  SEARCHING..  ", TAM_X, TAM_Y + 80, GC9A01A_ORANGE, MAU_NEN);
  }
}


/* ========== GIAO DIỆN 2: MESSAGE ========== */

void capNhatMessage() {
  if (canXoaManHinh) {
    TFT_FillScreen(&tft, GC9A01A_BLACK);
    canXoaManHinh = 0;

    /* Khung viền và đường kẻ */
    TFT_DrawCircle(&tft, TAM_X, TAM_Y, 110, GC9A01A_GREEN);
    TFT_DrawCircle(&tft, TAM_X, TAM_Y, 106, GC9A01A_DARKGREEN);
    TFT_DrawFastHLine(&tft, 45, 75, 150, GC9A01A_GREEN);

    TFT_SetTextSize(2);
    TFT_SetTextDatum(TC_DATUM);
    TFT_DrawString(&tft, "MESSAGE", TAM_X, 40, GC9A01A_GREEN, GC9A01A_BLACK);

    char timeStr[30];
    sprintf(timeStr, "Time: 00:00");
    TFT_SetTextSize(2);
    TFT_SetTextDatum(CC_DATUM);
    TFT_DrawString(&tft, timeStr, TAM_X, 105, GC9A01A_GREEN, GC9A01A_BLACK);

    /* Điểm đến */
    TFT_SetTextSize(2);
    TFT_SetTextDatum(BC_DATUM);
    TFT_DrawString(&tft, "Destination:", TAM_X, 150, GC9A01A_GREEN, GC9A01A_BLACK);

    /* Tên địa điểm nhận từ ESP32 (chưa nhận thì hiện Waiting...) */
    if (fromESP32.dest_name[0] != '\0') {
            TFT_DrawString(&tft, fromESP32.dest_name, TAM_X, 178, GC9A01A_ORANGE, GC9A01A_BLACK);
    }
    else {
            TFT_DrawString(&tft, "Waiting...", TAM_X, 178, GC9A01A_ORANGE, GC9A01A_BLACK);
    }
  }
}

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

/* ---- NRF: xử lý cờ ngắt (nội dung y hệt nhánh PA1 cũ trong ISR) ---- */
void NRF_XuLyNgat(void)
{
    uint8_t status = nrf24_r_status();

    /* Gửi thành công */
    if (status & (1 << TX_DS)) {
        nrf24_clear_tx_ds();
    }

    /* Nhận được dữ liệu */
    if (status & (1 << RX_DR)) {
        if (nrf24_data_available()) {
            DuLieuNhan temp_nhan = {0};
            nrf24_receive((uint8_t *)&temp_nhan, sizeof(temp_nhan));
            if (temp_nhan.header == GIA_TRI_HEADER && temp_nhan.footer == GIA_TRI_FOOTER) {
            /* Chặn gói rỗng (0,0) như cũ */
            if (temp_nhan.dest_lat != 0.0f || temp_nhan.dest_lon != 0.0f)
            {
                temp_nhan.dest_name[12] = '\0';
                /* Ghi gói đích vào biến dùng chung: giữ mutex để radar
                   không đọc giữa chừng (struct 30 byte, copy không nguyên tử) */
                osMutexAcquire(dataMutexHandle, osWaitForever);
                fromESP32 = temp_nhan;
                osMutexRelease(dataMutexHandle);
            }
            }
            else {

            }
        }

        nrf24_clear_rx_dr();
        nrf24_flush_rx();
    }

    /* Lỗi kết nối (hết số lần gửi lại) */
    if (status & (1 << MAX_RT)) {
        nrf24_clear_max_rt();
        nrf24_flush_tx();
        nrf24_flush_rx();
    }
}

/* ---- NRF: gửi định kỳ 200 ms ---- */
void NRF_GuiDinhKy(void)
{
    if (HAL_GetTick() - thoiGianGuiNRF <= 200) return;
    thoiGianGuiNRF = HAL_GetTick();

    /* Chụp nhanh vị trí/hướng hiện tại vào gói gửi */
    osMutexAcquire(dataMutexHandle, osWaitForever);
    toESP32.header = GIA_TRI_HEADER;
    toESP32.lat = viTriHienTai.x;
    toESP32.lon = viTriHienTai.y;
    toESP32.yaw = HuongDi;
    toESP32.footer = GIA_TRI_FOOTER;
    osMutexRelease(dataMutexHandle);

    /* KHÔNG còn HAL_NVIC_DisableIRQ(EXTI1)/Enable nữa: ISR giờ không đụng
       SPI2, mọi truy cập NRF đều nằm trong NRFTask nên không tranh chấp. */
    nrf24_transmit((uint8_t *)&toESP32, sizeof(toESP32));
}


/* ---- Màn hình: đổi giao diện + vẽ lại ---- */
void Display_ChayMotVong(void)
{
    if (manHinhHienTai != manHinhTruocDo) {
        canXoaManHinh   = 1;
        manHinhTruocDo  = manHinhHienTai;
    }

    /* KHÔNG còn tắt EXTI1 khi vẽ: TFT dùng SPI1, NRF dùng SPI2 (khác nhau) */
    if (manHinhHienTai == SCREEN_RADAR)        capNhatRadar();
    else if (manHinhHienTai == SCREEN_MESSAGE) capNhatMessage();
}

/* ---- (TÙY CHỌN, khi tích hợp xong IMU) ----
void IMU_ChayMotVong(void)
{
    if (!imu_ok) return;
    IMU_Update();                                   // đọc ICM-20948 (SPI3)
    osMutexAcquire(dataMutexHandle, osWaitForever);
    HuongDi = yaw;                                  // yaw từ yaw_fusion
    osMutexRelease(dataMutexHandle);
}
*/

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM10_Init();
  MX_USART1_UART_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */

  Debug_Print("System Boot\r\n");

  /*Khởi tạo GPS*/
  GPS_Init(&huart1);

  /*Khởi tạo IMU*/
  imu_ok = IMU_Init();

  /*Khởi tạo màn hình GC9A01A*/
    GC9A01A_init(&tft, &hspi1,
                 GPIOA, GPIO_PIN_4, // CS
                 GPIOA, GPIO_PIN_3, // DC
                 GPIOA, GPIO_PIN_1, // BLK
                 GPIOA, GPIO_PIN_2  // RST
                 );
    veNenRadar();
    canXoaManHinh = 0;

  /*Khởi tạo NRF24*/
  KhoiTao_NRF24();

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();
  /* Create the mutex(es) */
  /* creation of dataMutex */
  dataMutexHandle = osMutexNew(&dataMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of nrfIrqSem */
  nrfIrqSemHandle = osSemaphoreNew(1, 1, &nrfIrqSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of GPSTask */
  GPSTaskHandle = osThreadNew(StartGPSTask, NULL, &GPSTask_attributes);

  /* creation of NRFTask */
  NRFTaskHandle = osThreadNew(StartNRFTask, NULL, &NRFTask_attributes);

  /* creation of DisplayTask */
  DisplayTaskHandle = osThreadNew(StartDisplayTask, NULL, &DisplayTask_attributes);

  /* creation of IMUTask */
  IMUTaskHandle = osThreadNew(StartTask05, NULL, &IMUTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //freertos lo
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi3.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 99;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TFT_RST_Pin|TFT_DC_Pin|TFT_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ICM_CS_GPIO_Port, ICM_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : NRF_IRQ_Pin */
  GPIO_InitStruct.Pin = NRF_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(NRF_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TFT_RST_Pin TFT_DC_Pin TFT_CS_Pin */
  GPIO_InitStruct.Pin = TFT_RST_Pin|TFT_DC_Pin|TFT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_PIN_Pin */
  GPIO_InitStruct.Pin = BTN_PIN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_PIN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : NRF_CSN_Pin */
  GPIO_InitStruct.Pin = NRF_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(NRF_CSN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : NRF_CE_Pin */
  GPIO_InitStruct.Pin = NRF_CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(NRF_CE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ICM_CS_Pin */
  GPIO_InitStruct.Pin = ICM_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(ICM_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* Xử lý ngắt ngoài: nút bấm (PB0) và NRF24 (PA1) */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/*--- Nút bấm (PB0): Đổi giao diện ---*/
    if (GPIO_Pin == GPIO_PIN_0)
    {
        uint32_t curr_time = HAL_GetTick();
        if (curr_time - last_press > 150)
        {
            if (manHinhHienTai == SCREEN_RADAR) {
                manHinhHienTai = SCREEN_MESSAGE;
            } else {
                manHinhHienTai = SCREEN_RADAR;
            }
            last_press = curr_time;
        }
    }

    // ---NRF24 (PA1): đọc trạng thái và xử lý ---
    if (GPIO_Pin == GPIO_PIN_1)
    {
        if (nrfIrqSemHandle != NULL)            /* ngắt có thể tới trước khi
                                                   kernel kịp tạo semaphore */
            osSemaphoreRelease(nrfIrqSemHandle);/* gọi được từ ISR (CMSIS-V2) */
    }

}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
	    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartGPSTask */
/**
* @brief Function implementing the GPSTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartGPSTask */
void StartGPSTask(void *argument)
{
  /* USER CODE BEGIN StartGPSTask */
  /* Infinite loop */
  for(;;)
  {
	    GPS_ChayMotVong();         /* đọc DMA + parse NMEA, mỗi 1 s cập nhật vị trí */
	    osDelay(20);               /* GPS 9600 baud ≈ 960 B/s, 20 ms là dư */

  }
  /* USER CODE END StartGPSTask */
}

/* USER CODE BEGIN Header_StartNRFTask */
/**
* @brief Function implementing the NRFTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartNRFTask */
void StartNRFTask(void *argument)
{
  /* USER CODE BEGIN StartNRFTask */
  /* Infinite loop */
  for(;;)
  {
	    /* chờ ngắt PA1 tối đa 20 ms; có ngắt -> xử lý cờ TX_DS/RX_DR/MAX_RT */
	    if (osSemaphoreAcquire(nrfIrqSemHandle, 20) == osOK)
	    {
	      NRF_XuLyNgat();
	    }
	    NRF_GuiDinhKy();           /* bên trong tự kiểm tra: chỉ gửi mỗi 200 ms */
  }
  /* USER CODE END StartNRFTask */
}

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the DisplayTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  uint32_t next = osKernelGetTickCount();
  /* Infinite loop */

  for(;;)
  {
	    Display_ChayMotVong();
	    next += 500;               /* chu kỳ cố định 500 ms (1 tick = 1 ms) */
	    osDelayUntil(next);

  }
  /* USER CODE END StartDisplayTask */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
* @brief Function implementing the IMUTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask05 */
void StartTask05(void *argument)
{
  /* USER CODE BEGIN StartTask05 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTask05 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM11 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM11)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
