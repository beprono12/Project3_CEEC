#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN  22
#define CSN_PIN 21

// Khởi tạo đối tượng radio
RF24 radio(CE_PIN, CSN_PIN);


// Gói nhận từ STM32 (Node)
typedef struct __attribute__((packed)) {
  float lat;
  float lon;
  float yaw;
} DuLieuNhan; // 12 bytes

// Gói gửi đi cho STM32 
typedef struct __attribute__((packed)) {
  float dest_lat;
  float dest_lon;
  char  dest_name[13];
} DuLieuGui;  // 

DuLieuNhan fromNode;
DuLieuGui  toNode;

// diaChiNRF
const uint8_t address[5] = {0x12, 0x34, 0x56, 0x78, 0x9A};

void setup() {
  Serial.begin(115200);
  Serial.println("Tram ESP32 is running");

 
  if (!radio.begin()) {
    Serial.println("NRF24 has errors.");
    while (1) {} // Dừng chương trình nếu lỗi phần cứng
  }
 // bắt buộc giống STM32
  radio.setChannel(90);                       // Kênh 90
  radio.setDataRate(RF24_1MBPS);              // Tốc độ 1Mbps
  radio.setPALevel(RF24_PA_MAX);              // Công suất Max
  radio.setCRCLength(RF24_CRC_8);             // CRC 1 byte

  radio.enableDynamicPayloads();              // Bật DPL
  radio.enableAckPayload();                   // Cho phép gửi kèm dữ liệu trong ACK

  // Mở luồng nhận dữ liệu (Pipe 1)
  radio.openReadingPipe(1, address);

  // TOẠ ĐỘ ĐÍCH và TÊN ĐỊA ĐIỂM (lấy từ WEBSERVER)
  toNode.dest_lat = 10.869752;
  toNode.dest_lon = 106.802605;
  strncpy(toNode.dest_name, "UIT_ToaE", 12); // Tối đa 12 ký tự + \0
  toNode.dest_name[12] = '\0'; // Đảm bảo an toàn kết thúc chuỗi

  // Nạp sẵn gói dữ liệu ACK vào bộ đệm TRƯỚC KHI bắt đầu nghe
  radio.writeAckPayload(1, &toNode, sizeof(toNode));

  // Bắt đầu chế độ lắng nghe (PRX)
  radio.startListening();
  Serial.println("NRF24 Setup OK. Waiting for STM32...");
}

void loop() {
  uint8_t pipeNum;
  
  // Kiểm tra xem có sóng gửi tới không
  if (radio.available(&pipeNum)) {
    // Đọc gói fromSTM32
    radio.read(&fromNode, sizeof(fromNode));

    // NRF24L01 đã tự động bắn gói "toNode" về cho STM32 bằng phần cứng
    // Nhiệm vụ của ta là nạp ngay gói ACK TIẾP THEO vào bộ đệm cho lần sau
    radio.writeAckPayload(1, &toNode, sizeof(toNode));

    // In ra Serial Monitor để giám sát
    Serial.println("--- PACKET RECEIVED ---");
    Serial.print("Node Lat: "); Serial.println(fromNode.lat, 6);
    Serial.print("Node Lon: "); Serial.println(fromNode.lon, 6);
    Serial.print("Node Yaw: "); Serial.println(fromNode.yaw, 1);
    Serial.println("ACK Data sent automatically.");
  }
}