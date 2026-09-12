#pragma once
#include <Arduino.h>
#include <DHT.h>

// ================= Pin mapping =================
// DHT11 (อุณหภูมิ/ความชื้นอากาศ) ใช้ขาเดียวสำหรับอ่านทั้ง 2 ค่า
#define DHTPIN   23          // ตามที่ระบุ: DHT11 ต่อ D23
#define DHTTYPE  DHT11
static DHT dht(DHTPIN, DHTTYPE);

// LDR แบบดิจิทัล (มี DO ออกมาเป็น HIGH/LOW)
#define LDR_PIN  22          // ตามที่ระบุ: LDR(DO) ต่อ D22

// ความชื้นดิน/ความชื้นแอนะล็อก
#define MOISTURE_A_PIN  4    // ตามที่ระบุ: ต่อ D4 (ADC2) — ถ้าเปิด Wi-Fi อาจเพี้ยน

// ไฟ/รีเลย์
#define LIGHT_PIN  2         // เพิ่ม: ไฟ/รีเลย์ ต่อ D2

// ================= Init =================
inline void initSensors() {
  // DHT11
  dht.begin();

  // LDR digital
  pinMode(LDR_PIN, INPUT);

  // Light/Relay
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW); // เริ่มปิดไฟ

  // ตั้งค่า ADC (แนะนำให้วงจรอนาล็อก 0~3.3V)
  analogSetWidth(12);             // 0..4095
  analogSetAttenuation(ADC_11db); // ช่วงแรงดันกว้างขึ้น (ประมาณ 0~3.6V บน ESP32)
}

// ================= Light control =================
inline void setLight(bool on) {
  digitalWrite(LIGHT_PIN, on ? HIGH : LOW);
}

// ================= LDR (Digital) =================
// raw: HIGH=สว่าง, LOW=มืด (ขึ้นกับโมดูล ถ้ากลับกันให้สลับเงื่อนไข)
inline int readLdrDigitalRaw() {
  return digitalRead(LDR_PIN);
}

inline String getDayNight() {
  return (readLdrDigitalRaw() == HIGH) ? "Day" : "Night";
}

// ================= DHT11 =================
inline float readTempC() {
  float t = dht.readTemperature();   // °C
  return isnan(t) ? NAN : t;
}

inline float readAirHumidityPct() {
  float h = dht.readHumidity();      // %RH
  return isnan(h) ? NAN : h;
}

// ================= Soil/Analog Moisture =================
// อ่านค่าแอนะล็อก 0..4095
inline int readMoistureRaw() {
  return analogRead(MOISTURE_A_PIN);
}

// ปกติ soil moisture: ค่าน้อย = ชื้นมาก, ค่าสูง = แห้ง
// แปลงเป็น %ชื้น (0=แห้ง, 100=ชื้นมาก)
// ให้ปรับคาลิเบรตที่ dryCal/wetCal ให้เข้ากับเซนเซอร์จริง
inline int readMoisturePercent(int dryCal = 3600, int wetCal = 1200) {
  int raw = readMoistureRaw();
  raw = constrain(raw, wetCal, dryCal);
  int pct = map(raw, dryCal, wetCal, 0, 100); // กลับสเกล
  return constrain(pct, 0, 100);
}
