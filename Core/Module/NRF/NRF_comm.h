/*
 * NRF_comm.h
 *
 *  Created on: Oct 8, 2026
 *      Author: Admin
 */

#ifndef NRF_COMM_H
#define NRF_COMM_H

#include "main.h"

// Định nghĩa "Khóa" bảo vệ gói tin
#define GIA_TRI_HEADER 0xAA55
#define GIA_TRI_FOOTER 0xBB

/* Khởi tạo cấu hình cho NRF24L01 */
void KhoiTao_NRF24(void);

/* Xử lý cờ ngắt khi có sự kiện từ chân IRQ */
void NRF_XuLyNgat(void);

/* Đóng gói và gửi dữ liệu tọa độ định kỳ sang Trạm ESP32 */
void NRF_GuiDinhKy(void);

#endif /* NRF_COMM_H */
