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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tft_gc9a01.h"
#include <stdio.h>
#include "NRF24.h"
#include "NRF24_reg_addresses.h"
// Lưu ý! math.h, string.h đã được include trong "tft_gc9a01.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define PIN_NUT_BAM PB0 // Chân nút bấm chuyển đổi

// ĐỊNH NGHĨA
// CẤU HÌNH GIAO DIỆN RADAR
const int16_t TAM_X = 120;
const int16_t TAM_Y = 120;
const int16_t BAN_KINH_RADAR = 100;
const float   MAX_RANGE_M = 50.0;

#define MAU_NEN        GC9A01A_GREEN
#define MAU_LUOI       GC9A01A_BLACK
#define MAU_MUI_TEN    GC9A01A_ORANGE
#define MAU_DICH       GC9A01A_WHITE
#define MAU_TEXT       GC9A01A_BLACK
const int16_t KHOANG_CACH_LUOI = 20;

typedef enum {
  SCREEN_RADAR,
  SCREEN_MESSAGE
} TrangThaiManHinh;

TrangThaiManHinh manHinhHienTai = SCREEN_RADAR;
TrangThaiManHinh manHinhTruocDo = SCREEN_MESSAGE;

uint8_t canXoaManHinh = 1;
uint32_t thoiGianQuetRadar = 0;

// DATA PACKAGE cho NRF
typedef struct __attribute__((packed)) {
	float lat; //vĩ độ
	float lon; // kinh độ
	float yaw; // góc yaw
} DuLieuGui;

typedef struct __attribute__((packed)) {
	float dest_lat; //sent: được gửi đi
	float dest_lon;
	char dest_name[13]; // name of that destination
} DuLieuNhan;

DuLieuGui toESP32;
DuLieuNhan fromESP32;
uint8_t diaChiNRF[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
uint32_t thoiGianGuiNRF = 0;
//....................................

// DỮ LIỆU MÔ PHỎNG TỌA ĐỘ
// x = lat(vĩ độ), y = lon (kinh độ)

typedef struct { float x; float y; } ToaDo;
ToaDo layToaDoThietBi() { ToaDo td = {10.869089, 106.803975}; return td; } // lấy từ GPS (phải lọc trước đó)

ToaDo layToaDoDich()    {
	ToaDo td = {fromESP32.dest_lat,  fromESP32.dest_lon};  // lấy từ TRẠM ESP32
    return td; }

float layHuongDi()      { return 0.0; } // lấy từ IMU (phải lọc đã lọc)

// Đối tượng màn hình
GC9A01A tft;
extern SPI_HandleTypeDef hspi1;



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim10;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM10_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void KhoiTao_NRF24(void) {
    nrf24_init();
    nrf24_set_channel(90);
    nrf24_data_rate(_1mbps);
    nrf24_tx_pwr(_0dbm);
    nrf24_set_crc(en_crc, _1byte);

    nrf24_dpl(enable);
    nrf24_set_rx_dpl(0, enable);
    nrf24_en_ack_pld(enable);

    nrf24_auto_retr_delay(5);
    nrf24_auto_retr_limit(15);

    nrf24_open_tx_pipe(diaChiNRF);
    nrf24_open_rx_pipe(0, diaChiNRF);

    nrf24_flush_rx(); // Xả rác bộ đệm nhận
    nrf24_flush_tx(); // Xả rác bộ đệm phát

    nrf24_stop_listen();
}

// TÍNH TOÁN khoảng cách, góc đi
float sangRad(float Do) {
	return Do * M_PI / 180.0;
}
float sangDo(float rad) {
	return rad * 180.0 / M_PI;
}

// Thuật toán Haversine
const float R_TraiDat = 6371000;// mét
float tinhKhoangCach(ToaDo a, ToaDo b) {
		/// p ~ phi
		// l ~ lambda
		float p1 = sangRad(a.x);
		float p2 = sangRad(b.x);
		float delta_p = sangRad(b.x - a.x);
		float delta_l = sangRad(b.y - a.y);

		float h = sin(delta_p / 2.0) * sin(delta_p /2.0) +
				cos(p1) * cos(p2) *
				sin(delta_l / 2.0) * sin(delta_l / 2.0);
		float c = 2.0 * atan2(sqrt(h), sqrt(1.0 - h));

		return c * R_TraiDat;
}

// Tính góc Bearing: xác định hướng đi từ điểm A đến điểm B trên bề mặt cong của Trái Đất.
float tinhGocPhuongVi(ToaDo a, ToaDo b) {
  float dx = b.x - a.x; //lệch trục dọc
  float dy = b.y - a.y; //lệch trục ngang

  if (dx == 0.0f && dy == 0.0f) return 0.0f; //tránh nhiễu

  float goc = sangDo(atan2f(dy, dx)); // atan2f(Đông-Tây, Bắc-Nam)
  if (goc < 0.0f) goc += 360.0;

  return goc;
}

/* GIAO DIỆN 1: RADAR */
void veNenRadar() {
  TFT_FillScreen(&tft, MAU_NEN);

  for (int16_t x = 0; x < TFT_WIDTH; x += KHOANG_CACH_LUOI) {
      TFT_DrawFastVLine(&tft, x, 0, 240, MAU_LUOI);
  }
  for (int16_t y = 0; y < TFT_HEIGHT; y += KHOANG_CACH_LUOI) {
      TFT_DrawFastHLine(&tft, 0, y, 240, MAU_LUOI);
  }
}

void veMuiTenThietBi() {
  int16_t dai = 12; int16_t rong = 8;
  int16_t x0 = TAM_X, y0 = TAM_Y - dai;
  int16_t x1 = TAM_X - rong, y1 = TAM_Y + dai / 2;
  int16_t x2 = TAM_X + rong, y2 = TAM_Y + dai / 2;
  TFT_FillTriangle(&tft, x0, y0, x1, y1, x2, y2, MAU_MUI_TEN);
}

void veChamDich(float khoangCach, float gocTuongDoi) {
  // Biến static giúp giữ nguyên giá trị sau mỗi lần thoát hàm
  static int16_t x_cu = -1, y_cu = -1;
  static uint8_t ngoaiPhamVi_cu = 0;

  // Xóa chấm sáng cũ (Tô bằng màu nền)
  if (x_cu != -1 && y_cu != -1) {
      TFT_FillCircle(&tft, x_cu, y_cu, 5, MAU_NEN);
      if (ngoaiPhamVi_cu) TFT_DrawCircle(&tft, x_cu, y_cu, 7, MAU_NEN);
  }

  //  Tính toán vị trí mới
  float khoangCachVeMan = (khoangCach > MAX_RANGE_M) ? MAX_RANGE_M : khoangCach;
  uint8_t ngoaiPhamVi = (khoangCach > MAX_RANGE_M);
  float banKinhPx = (khoangCachVeMan / MAX_RANGE_M) * BAN_KINH_RADAR;
  float rad = gocTuongDoi * M_PI / 180.0;

  int16_t x = TAM_X + (int16_t)(banKinhPx * sinf(rad));
  int16_t y = TAM_Y - (int16_t)(banKinhPx * cosf(rad));

  //Vẽ chấm sáng mới
  TFT_FillCircle(&tft, x, y, 5, MAU_DICH);
  if (ngoaiPhamVi) TFT_DrawCircle(&tft, x, y, 7, MAU_DICH);

  //Cập nhật lại bộ nhớ tọa độ cũ
  x_cu = x;
  y_cu = y;
  ngoaiPhamVi_cu = ngoaiPhamVi;
}

// In số liệu khoảng cách
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

// Cập nhật giao diện Radar
void capNhatRadar() {
  if (canXoaManHinh) {
    veNenRadar();
    canXoaManHinh = 0;
  }

  //  Luôn lấy tọa độ và vẽ thiết bị ở center
  ToaDo viTriThietBi = layToaDoThietBi();
  float huongDi      = layHuongDi();
  veMuiTenThietBi();

  //  Chỉ tính toán và vẽ chấm sáng NẾU TrạmESP32 đã gửi dữ liệu về
  if (fromESP32.dest_name[0] != '\0') {
      ToaDo viTriDich    = layToaDoDich();
      float khoangCach = tinhKhoangCach(viTriThietBi, viTriDich);
      float gocPhuongVi = tinhGocPhuongVi(viTriThietBi, viTriDich);
      float gocTuongDoi = gocPhuongVi - huongDi;

      if (gocTuongDoi < 0) gocTuongDoi += 360.0;

      veChamDich(khoangCach, gocTuongDoi); // Vẽ chấm sáng
      veKhoangCach(khoangCach);            // In khoảng cách
  }
  else {
      // Nếu chưa có tọa độ đích: Không vẽ chấm sáng + hiện SEARCHING
      TFT_SetTextSize(2);
      TFT_SetTextDatum(BC_DATUM);
      TFT_DrawString(&tft, "  SEARCHING..  ", TAM_X, TAM_Y + 80, GC9A01A_ORANGE, MAU_NEN);
  }
}

/* GIAO DIỆN 2: MESSAGE */
void capNhatMessage() {
  if (canXoaManHinh) {
    TFT_FillScreen(&tft, GC9A01A_BLACK);
    canXoaManHinh = 0;

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

    TFT_SetTextSize(2);
    TFT_SetTextDatum(BC_DATUM);
    TFT_DrawString(&tft, "Destination:", TAM_X, 150, GC9A01A_GREEN, GC9A01A_BLACK);
    // Hiển thị tên địa điểm thực tế nhận từ ESP32
    if (fromESP32.dest_name[0] != '\0') {
            TFT_DrawString(&tft, fromESP32.dest_name, TAM_X, 178, GC9A01A_ORANGE, GC9A01A_BLACK);
    }
    else {
            TFT_DrawString(&tft, "Waiting...", TAM_X, 178, GC9A01A_ORANGE, GC9A01A_BLACK);
    }
  }
}

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
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM10_Init();
  /* USER CODE BEGIN 2 */
  // Khởi tạo cấu hình màn hình GC9A01A
    GC9A01A_init(&tft, &hspi1,
                 GPIOA, GPIO_PIN_4, // CS
                 GPIOA, GPIO_PIN_3, // DC
                 GPIOA, GPIO_PIN_1, // BLK
                 GPIOA, GPIO_PIN_2  // RST
                 );

    veNenRadar();
    canXoaManHinh = 0;

    KhoiTao_NRF24();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // Tự động kiểm tra sự thay đổi giao diện bởi ngắt
	        if (manHinhHienTai != manHinhTruocDo) {
	            canXoaManHinh = 1;
	            manHinhTruocDo = manHinhHienTai;
	        }

	        if (HAL_GetTick() - thoiGianGuiNRF > 200) {
	                  thoiGianGuiNRF = HAL_GetTick();

	                  ToaDo temp = layToaDoThietBi();
	                  toESP32.lat = temp.x ;
	                  toESP32.lon = temp.y;
	                  toESP32.yaw = layHuongDi();

	                  // Tạm khóa ngắt chân PA1 để độc chiếm đường truyền SPI
	                  HAL_NVIC_DisableIRQ(EXTI1_IRQn);

	                  nrf24_transmit((uint8_t *)&toESP32, sizeof(toESP32));

	                  //Mở khóa ngắt trở lại. Gói ACK nhận được sẽ kích hoạt hàm ngắt ngay lập tức một cách an toàn.
	                  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	         }

	        if (HAL_GetTick() - thoiGianQuetRadar > 500) {
	            thoiGianQuetRadar = HAL_GetTick();

	            HAL_NVIC_DisableIRQ(EXTI1_IRQn);
	            if (manHinhHienTai == SCREEN_RADAR) {
	                capNhatRadar();
	            }
	            else if (manHinhHienTai == SCREEN_MESSAGE) {
	                capNhatMessage();
	            }

	            HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	        }

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

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//Xử lý nút bấm = ngắt ngoài
/* Biến toàn cục lưu thời gian bấm nút lần cuối */
volatile uint32_t last_press = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// --- NGẮT BUTTON (PB0) ---
    if (GPIO_Pin == GPIO_PIN_0)
    {
        uint32_t curr_time = HAL_GetTick();
        if (curr_time - last_press > 150)
        {
            // đảo trạng thái
            if (manHinhHienTai == SCREEN_RADAR) {
                manHinhHienTai = SCREEN_MESSAGE;
            } else {
                manHinhHienTai = SCREEN_RADAR;
            }
            last_press = curr_time;
        }
    }

    // --- NGẮT NRF24 (PA1) ---
        if (GPIO_Pin == GPIO_PIN_1)
        {
            uint8_t status = nrf24_r_status();

            //Xử lý khi Gửi thành công
            if (status & (1 << TX_DS)) {
                nrf24_clear_tx_ds();
            }

            //Xử lý khi Nhận có dữ liệu
            if (status & (1 << RX_DR)) {

                if (nrf24_data_available()) {
                    // Khởi tạo vùng nhớ sạch
                    DuLieuNhan temp_nhan = {0};

                    nrf24_receive((uint8_t *)&temp_nhan, sizeof(temp_nhan));

                    // Chặn các gói rỗng (0.0, 0.0) tránh nhận gói rỗng làm dữ liệu đích đến -> rác
                    if (temp_nhan.dest_lat != 0.0f || temp_nhan.dest_lon != 0.0f)
                    {
                        fromESP32 = temp_nhan;
                        fromESP32.dest_name[12] = '\0';
                    }
                }

                nrf24_clear_rx_dr();
                nrf24_flush_rx();
            }

            // Xử lý khi Lỗi kết nối
            if (status & (1 << MAX_RT)) {
                nrf24_clear_max_rt();
                nrf24_flush_tx();
                nrf24_flush_rx();
            }
        }
}
/* USER CODE END 4 */

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
