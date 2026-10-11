/*
 * DrawInterface.h
 *
 *  Created on: Oct 8, 2026
 *      Author: Admin
 */

#ifndef DRAW_INTERFACE_H
#define DRAW_INTERFACE_H

#include "main.h"
/* Export biến trạng thái để ngắt nút bấm (EXTI) trong main.c có thể điều khiển */
extern volatile TrangThaiManHinh manHinhHienTai;

/* Hàm cập nhật giao diện công khai */
void Display_ChayMotVong(void);

#endif /* DRAW_INTERFACE_H */
