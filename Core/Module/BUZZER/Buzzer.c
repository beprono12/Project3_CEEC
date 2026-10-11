/*
 * Buzzer.c
 */
#include "Buzzer.h"
#include <stddef.h>      /* NULL */
#include "main.h"        /* BUZZER_Pin, BUZZER_GPIO_Port (CubeMX sinh ra từ nhãn "BUZZER") */
#include "cmsis_os.h"

/* Mức logic của chân I/O. Module kích mức THẤP: RESET = kêu, SET = im. */
#define BUZ_ON    GPIO_PIN_RESET
#define BUZ_OFF   GPIO_PIN_SET

typedef struct {
  uint16_t on_ms;    /* thời gian kêu */
  uint16_t off_ms;   /* thời gian nghỉ giữa 2 tiếng */
  uint8_t  lap;      /* số tiếng */
} NhipKeu;

static const NhipKeu nhip[BUZ_COUNT] = {
  [BUZ_WAKE]     = {   0,   0, 0 },
  [BUZ_CLICK]    = {  30,   0, 1 },
  [BUZ_NEW_DEST] = {  80,  80, 2 },
  [BUZ_ARRIVED]  = { 200, 100, 3 },
};

#define KEU_LAP_MS  60   /* độ dài 1 tiếng của chế độ kêu lặp theo khoảng cách */

static osMessageQueueId_t buzQueue = NULL;
static volatile uint32_t  chuKyMs  = 0;

static void keu(uint16_t on_ms, uint16_t off_ms, uint8_t lap)
{
  for (uint8_t i = 0; i < lap; i++) {
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, BUZ_ON);
    osDelay(on_ms);
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, BUZ_OFF);
    if (off_ms && i < lap - 1) osDelay(off_ms);
  }
}

static void BuzzerTask(void *argument)
{
  (void)argument;
  uint8_t mau;

  for (;;) {
    /* Chờ lệnh; nếu đang bật kêu lặp thì chờ tối đa 1 chu kỳ */
    uint32_t cho = chuKyMs ? chuKyMs : osWaitForever;

    if (osMessageQueueGet(buzQueue, &mau, NULL, cho) == osOK) {
      if (mau < BUZ_COUNT) keu(nhip[mau].on_ms, nhip[mau].off_ms, nhip[mau].lap);
    }
    else if (chuKyMs) {
      keu(KEU_LAP_MS, 0, 1);   /* hết thời gian chờ = tới lượt kêu lặp */
    }
  }
}

uint8_t Buzzer_Init(void)
{
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, BUZ_OFF);

  buzQueue = osMessageQueueNew(8, sizeof(uint8_t), NULL);

  const osThreadAttr_t attr = {
    .name       = "BuzzerTask",
    .stack_size = 128 * 4,
    .priority   = (osPriority_t) osPriorityNormal,
  };
  osThreadId_t th = osThreadNew(BuzzerTask, NULL, &attr);

  return (buzQueue != NULL && th != NULL) ? 1 : 0;
}

void Buzzer_Play(BuzzerMau mau)
{
  if (buzQueue == NULL) return;          /* ngắt có thể tới trước khi queue được tạo */
  uint8_t m = (uint8_t)mau;
  osMessageQueuePut(buzQueue, &m, 0, 0); /* timeout 0 = an toàn trong ISR */
}

void Buzzer_SetChuKy(uint32_t ms)
{
  if (ms == chuKyMs) return;
  chuKyMs = ms;
  Buzzer_Play(BUZ_WAKE);   /* task đang chờ với chu kỳ cũ -> đánh thức để áp chu kỳ mới ngay */
}
