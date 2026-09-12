/* Map.ino — ESP32 Bluetooth Navigation Display (BT-first, SH1106) */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <BluetoothSerial.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define BT_NAME "ESP32-Map"   // ชื่อบลูทูธ

#include "sensors.h"
#include "bluetooth.h"

// ===== อ็อบเจกต์จริง (ประกาศแค่ไฟล์นี้ไฟล์เดียว) =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,        // rotation 0°
  U8X8_PIN_NONE   // reset pin (ไม่ใช้)
);
BluetoothSerial SerialBT;

NavData g_nav;                // สถานะนำทาง
bool    g_display_ok = false; // จอพร้อม?
uint8_t g_oled_addr = 0x3C;   // จะตรวจจริงอีกทีใน displayInit()

void setup() {
  Serial.begin(115200);
  delay(100);

  // เริ่ม I2C (กำหนดขา)
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  // เปิด BT ก่อนเสมอ
  btInit(BT_NAME);

  // เริ่มจอ (สแกน 0x3C/0x3D อัตโนมัติ)
  displayInit();

  // ข้อความเริ่มต้น (ถ้าจอพร้อมจะเห็น)
  navSetStatus("BT Ready");
  displayUpdate();
}

void loop() {
  btPoll();   // อ่านคำสั่งจากมือถือ
  delay(2);
}
