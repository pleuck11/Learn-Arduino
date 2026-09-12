#include <Arduino.h>
#include <WiFi.h>
#include "sensors.h"

// ===== ใส่ WiFi ของคุณ =====
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

void setup() {
  Serial.begin(115200);
  initSensors();

  // เริ่มเชื่อมต่อ WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // แสดงเฉพาะ IP Address
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // ถ้าไม่ต้องการแสดงข้อมูลอะไรเลย ก็ปล่อยว่างไว้ได้
}
