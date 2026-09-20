#ifndef TFT_GC9A01_H
#define TFT_GC9A01_H

#include "gc9a01a.h"
#include <stdint.h>
#include <string.h>
#include <math.h>

//Định nghĩa chiều rộng + dài của màn tft
#define TFT_WIDTH 240
#define TFT_HEIGHT 240

// CẤU HÌNH FONT CHỮ VÀ CĂN LỀ
#define TL_DATUM 0 //TOP-LEFT
#define TC_DATUM 1 //TOP-CORNER
#define TR_DATUM 2 //TOP-RIGHT
#define ML_DATUM 3 //MIDDLE-LEFT
#define CC_DATUM 4 //CENTER-CENTER
#define MR_DATUM 5//MIDDLE-RIGHT
#define BL_DATUM 6 //BOTTOM-LEFT
#define BC_DATUM 7//BOTTOM-CENTER
#define BR_DATUM 8//BOTTOM-RIGHT

//Lưu ý: các hàm ở dưới phục vụ cho màn tft tròn 240x240
//Nếu muốn tích hợp cho nhiều loại màn thì cấu hình lại, thêm hàm phù hợp...

void TFT_FillScreen(GC9A01A *tft, uint16_t color); //Tô màu full màn
void TFT_DrawFastVLine(GC9A01A *tft, int16_t x, int16_t y, int16_t h, uint16_t color); // Vertical Line
void TFT_DrawFastHLine(GC9A01A *tft, int16_t x, int16_t y, int16_t w, uint16_t color); // Horizontal line


void TFT_SetTextDatum(uint8_t datum); // Thiết lập Căn lề chữ (Trái
void TFT_SetTextSize(uint8_t size); //Cỡ chữ

// Hàm vẽ ký tự (có đệm)
void TFT_DrawChar(GC9A01A *tft, int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
// Hàm in chuỗi văn bản
void TFT_DrawString(GC9A01A *tft, const char* str, int16_t x, int16_t y, uint16_t color, uint16_t bg);
// Hàm vẽ đường tròn
void TFT_DrawCircle(GC9A01A *tft, int16_t x0, int16_t y0, int16_t r, uint16_t color);
void TFT_FillCircle(GC9A01A *tft, int16_t x0, int16_t y0, int16_t r, uint16_t color);

//Vẽ và tô màu tam giác/ mũi tên
void TFT_FillTriangle(GC9A01A *tft, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);



#endif /*TFT_GC9A01_H*/
