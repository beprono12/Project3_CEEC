
#include "DrawInterface.h"
#include "cmsis_os.h"
#include "tft_gc9a01.h"
#include "gps.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "Buzzer.h"
/* Bảo vệ hằng số Pi */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* --- Define Màu sắc & Cấu hình --- */
#define MAU_NEN        GC9A01A_GREEN
#define MAU_LUOI       GC9A01A_BLACK
#define MAU_MUI_TEN    GC9A01A_ORANGE
#define MAU_DICH       GC9A01A_WHITE
#define MAU_TEXT       GC9A01A_BLACK

/* --- Kích thước giao diện radar --- */
const int16_t TAM_X = 120;
const int16_t TAM_Y = 120;
const int16_t BAN_KINH_RADAR = 100;
const float   MAX_RANGE_M = 50.0;
const int16_t KHOANG_CACH_LUOI = 20;

/* --- Các biến nội bộ của giao diện --- */
volatile TrangThaiManHinh manHinhHienTai = SCREEN_RADAR;
static TrangThaiManHinh manHinhTruocDo = SCREEN_MESSAGE;
static uint8_t  canXoaManHinh = 1;     /* 1 = cần vẽ lại nền */


//Cập nhật message
#define MSG_MAX_CHARS 12  /* khớp dest_name[13]: 12 ký tự + '\0' */

static char yawCu[24]               = "";
static char destCu[MSG_MAX_CHARS+1] = "";

/* Chép src vào out, căn giữa trong n ký tự (đệm khoảng trắng hai bên) */
static void canGiua(char *out, const char *src, int n) {
  int len = (int)strnlen(src, n);
  int trai = (n - len) / 2;
  memset(out, ' ', n);
  memcpy(out + trai, src, len);
  out[n] = '\0';
}
/* --- EXTERN: Mượn các biến toàn cục từ main.c --- */
extern GC9A01A tft;
extern osMutexId_t dataMutexHandle;
extern ToaDo viTriHienTai;
extern float HuongDi;
extern DuLieuNhan fromESP32;

/* ================= CÁC HÀM VẼ GIAO DIỆN (PRIVATE) ================= */

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



/* Tô lại nền radar (có lưới) trong ô vuông quanh (cx, cy) */
#define R_XOA 8   /* chấm bán kính 5, vòng ngoài phạm vi bán kính 7, +1 dư */
static void xoaVungRadar(int16_t cx, int16_t cy) {
  static uint16_t buf[(2 * R_XOA + 1) * (2 * R_XOA + 1)];
  const int16_t w  = 2 * R_XOA + 1;
  const int16_t x0 = cx - R_XOA;
  const int16_t y0 = cy - R_XOA;

  for (int16_t j = 0; j < w; j++) {
    for (int16_t i = 0; i < w; i++) {
      int16_t px = x0 + i, py = y0 + j;
      buf[j * w + i] = (px % KHOANG_CACH_LUOI == 0 || py % KHOANG_CACH_LUOI == 0)
                       ? MAU_LUOI : MAU_NEN;
    }
  }
  GC9A01A_draw_pixels(&tft, x0, y0, buf, w, w);
}

/*Vẽ chấm đích */
void veChamDich(float khoangCach, float gocTuongDoi) {

  //static giữ nguyên giá trị vị trí cũ sau mỗi lần thoát hàm
  static int16_t x_cu = -1, y_cu = -1;

  // Xóa chấm sáng cũ bằng màu nền
  // Xóa chấm cũ: khôi phục nền + lưới
  if (x_cu != -1 && y_cu != -1) xoaVungRadar(x_cu, y_cu);

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
}

/*Vẽ mũi tên thiết bị ở tâm*/
void veMuiTenThietBi() {
  int16_t dai = 12; int16_t rong = 8;
  int16_t x0 = TAM_X, y0 = TAM_Y - dai;
  int16_t x1 = TAM_X - rong, y1 = TAM_Y + dai / 2;
  int16_t x2 = TAM_X + rong, y2 = TAM_Y + dai / 2;
  TFT_FillTriangle(&tft, x0, y0, x1, y1, x2, y2, MAU_MUI_TEN);
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
  veMuiTenThietBi();   /* vẽ sau cùng để không bị ô xóa chấm cũ cắt vào */
}


/* ========== GIAO DIỆN 2: MESSAGE ========== */
void capNhatMessage() {
  if (canXoaManHinh) {
    TFT_FillScreen(&tft, GC9A01A_BLACK);

    /* Khung viền và đường kẻ */
    TFT_DrawCircle(&tft, TAM_X, TAM_Y, 110, GC9A01A_GREEN);
    TFT_DrawCircle(&tft, TAM_X, TAM_Y, 106, GC9A01A_DARKGREEN);
    TFT_DrawFastHLine(&tft, 45, 75, 150, GC9A01A_GREEN);

    TFT_SetTextSize(2);
    TFT_SetTextDatum(TC_DATUM);
    TFT_DrawString(&tft, "MESSAGE", TAM_X, 40, GC9A01A_GREEN, GC9A01A_BLACK);

    TFT_SetTextDatum(BC_DATUM);
    TFT_DrawString(&tft, "Destination:", TAM_X, 150, GC9A01A_GREEN, GC9A01A_BLACK);

    /* Buộc phần động vẽ lại ở lần đầu */
    yawCu[0]  = '\0';
    destCu[0] = '\0';
    canXoaManHinh = 0;
  }

  /* Lấy dữ liệu dưới mutex */
  osMutexAcquire(dataMutexHandle, osWaitForever);
  float      huongDi = HuongDi;
  DuLieuNhan nhan    = fromESP32;
  osMutexRelease(dataMutexHandle);

  /* --- Yaw: định dạng độ dài cố định (15 ký tự) nên không sót chữ cũ --- */
  char yawStr[24];
  snprintf(yawStr, sizeof(yawStr), "YAW: %6.2f deg", huongDi);
  if (strcmp(yawStr, yawCu) != 0) {
    TFT_SetTextSize(2);
    TFT_SetTextDatum(CC_DATUM);
    TFT_DrawString(&tft, yawStr, TAM_X, 105, GC9A01A_GREEN, GC9A01A_BLACK);
    strcpy(yawCu, yawStr);
  }

  /* --- Destination: đệm về 16 ký tự để ghi đè hết tên cũ --- */
  char destStr[MSG_MAX_CHARS + 1];
  canGiua(destStr, (nhan.dest_name[0] != '\0') ? nhan.dest_name : "Waiting...", MSG_MAX_CHARS);
  if (strcmp(destStr, destCu) != 0) {
    TFT_SetTextSize(2);
    TFT_SetTextDatum(BC_DATUM);
    TFT_DrawString(&tft, destStr, TAM_X, 178, GC9A01A_ORANGE, GC9A01A_BLACK);
    strcpy(destCu, destStr);
  }
}


/* ===== CẢNH BÁO BUZZER (chạy mỗi vòng, độc lập màn hình đang hiển thị) ===== */
#define BUZZER_PROXIMITY  1      /* 0 = tắt kêu lặp theo khoảng cách */
#define NGUONG_DEN_NOI    4.0f   /* khớp ngưỡng ARRIVED trong veKhoangCach */
#define NGUONG_MO_LAI     8.0f   /* trễ: ra xa quá mức này mới được báo lại */

static void canhBaoCapNhat(void) {
  static char    destBaoCu[13] = "";
  static uint8_t daBaoDenNoi   = 0;

  osMutexAcquire(dataMutexHandle, osWaitForever);
  ToaDo      viTri = viTriHienTai;
  DuLieuNhan nhan  = fromESP32;
  osMutexRelease(dataMutexHandle);

  /* Đích mới: tên đổi so với lần trước */
  if (strncmp(nhan.dest_name, destBaoCu, 12) != 0) {
    strncpy(destBaoCu, nhan.dest_name, 12);
    destBaoCu[12] = '\0';
    daBaoDenNoi = 0;                                   /* đích mới thì cho báo "đến nơi" lại */
    if (nhan.dest_name[0] != '\0') Buzzer_Play(BUZ_NEW_DEST);
  }

  /* Chưa có đích hoặc chưa có GPS fix (vị trí còn 0,0): im */
  if (nhan.dest_name[0] == '\0' || (viTri.x == 0.0f && viTri.y == 0.0f)) {
    Buzzer_SetChuKy(0);
    return;
  }

  float kc = GPS_Calculate_Distance(viTri.x, viTri.y, nhan.dest_lat, nhan.dest_lon);

  if (daBaoDenNoi) {                                   /* đã báo: chờ ra xa mới mở lại */
    if (kc > NGUONG_MO_LAI) daBaoDenNoi = 0;
    else { Buzzer_SetChuKy(0); return; }
  }
  if (kc <= NGUONG_DEN_NOI) {
    Buzzer_Play(BUZ_ARRIVED);
    daBaoDenNoi = 1;
    Buzzer_SetChuKy(0);
    return;
  }
#if BUZZER_PROXIMITY
  uint32_t ms = 0;                                     /* >50 m: im */
  if      (kc <= 10.0f) ms = 500;
  else if (kc <= 20.0f) ms = 1000;
  else if (kc <= 50.0f) ms = 2000;
  Buzzer_SetChuKy(ms);
#endif
}



/* ================= HÀM PUBLIC CHO MAIN ================= */
/* ---- Màn hình: đổi giao diện + vẽ lại ---- */
void Display_ChayMotVong(void)
{
    if (manHinhHienTai != manHinhTruocDo) {
        canXoaManHinh   = 1;
        manHinhTruocDo  = manHinhHienTai;
    }

    canhBaoCapNhat();
    /* KHÔNG còn tắt EXTI1 khi vẽ: TFT dùng SPI1, NRF dùng SPI2 (khác nhau) */
    if (manHinhHienTai == SCREEN_RADAR)        capNhatRadar();
    else if (manHinhHienTai == SCREEN_MESSAGE) capNhatMessage();
}
