#include "nrf_comm.h"
#include "NRF24.h"
#include "NRF24_reg_addresses.h"
#include "cmsis_os.h"

/* --- Biến quản lý nội bộ --- */
static DuLieuGui toESP32;
static uint8_t   diaChiNRF[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};
static uint32_t  thoiGianGuiNRF = 0;

/* --- EXTERN: Mượn tài nguyên dùng chung từ main.c --- */
extern osMutexId_t dataMutexHandle;
extern ToaDo      viTriHienTai;
extern float      HuongDi;
extern DuLieuNhan fromESP32;

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

/* ---- NRF: xử lý cờ ngắt ---- */
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

