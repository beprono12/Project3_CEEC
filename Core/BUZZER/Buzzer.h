/*
 * Buzzer.h
 * Buzzer chủ động (active), kích mức thấp, điều khiển bằng 1 chân GPIO.
 * Phân biệt sự kiện bằng NHỊP KÊU (dài/ngắn), không dùng cao độ.
 */
#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

typedef enum {
  BUZ_WAKE = 0,    /* nội bộ: đánh thức task để áp chu kỳ mới, không kêu */
  BUZ_CLICK,       /* bấm nút đổi màn hình: 1 tiếng rất ngắn */
  BUZ_NEW_DEST,    /* nhận đích mới từ Trạm: 2 tiếng ngắn */
  BUZ_ARRIVED,     /* đến nơi: 3 tiếng vừa */
  BUZ_COUNT
} BuzzerMau;

/* Gọi 1 lần trong main.c (trong USER CODE BEGIN RTOS_THREADS):
 * tạo hàng đợi + BuzzerTask. Trả về 1 nếu thành công, 0 nếu hết heap FreeRTOS. */
uint8_t Buzzer_Init(void);

/* Phát 1 nhịp. Không chặn, gọi được từ task lẫn ISR. */
void Buzzer_Play(BuzzerMau mau);

/* Chu kỳ kêu lặp (ms) kiểu cảm biến lùi xe. 0 = tắt. */
void Buzzer_SetChuKy(uint32_t ms);

#endif /* BUZZER_H */
