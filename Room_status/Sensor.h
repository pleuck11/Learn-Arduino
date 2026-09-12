#include <dummy.h>

// Sensor.h
#pragma once
#include <Arduino.h>
#include <DHT.h>

#define DHTPIN   33
#define DHTTYPE  DHT22
#define LDR_PIN  12

// ค่าประมาณ เริ่มต้น (ปรับตามหน้างาน)
#define LDR_MIN_RAW   150   // ค่าตอน "สว่างสุด"
#define LDR_MAX_RAW  3500   // ค่าตอน "มืดสุด"

#define LDR_DARK_PCT    20  // ≤20% = มืด
#define LDR_BRIGHT_PCT  60  // ≥60% = สว่าง

inline float lightPctCal(uint16_t raw){
  raw = constrain(raw, (uint16_t)LDR_MIN_RAW, (uint16_t)LDR_MAX_RAW);
  float pct = 100.0f * (raw - LDR_MIN_RAW) / (LDR_MAX_RAW - LDR_MIN_RAW);
  pct = constrain(pct, 0.0f, 100.0f);
  return 100.0f - pct;   // invert: raw สูง=มืด -> % ต่ำ
}
inline const char* lightStateFromPct(float pct){
  if (pct >= LDR_BRIGHT_PCT) return "สว่าง";
  if (pct <= LDR_DARK_PCT)   return "มืด";
  return "สลัว";
}

inline DHT dht(DHTPIN, DHTTYPE);

struct SensorData {
  float    temperature = NAN;
  float    humidity    = NAN;
  uint16_t lightRaw    = 0;
  float    lightPct    = NAN;
  const char* lightState = "ไม่ทราบ";
  uint32_t ts = 0;
};

inline void initSensors(){
  dht.begin();
  delay(2000);
#if defined(ARDUINO_ARCH_ESP32)
  #if defined(ADC_ATTEN_DB_11)
    analogSetPinAttenuation(LDR_PIN, ADC_ATTEN_DB_11);
  #elif defined(ADC_11db)
    analogSetPinAttenuation(LDR_PIN, ADC_11db);
  #endif
#endif
  pinMode(LDR_PIN, INPUT);
}

inline bool readDHT(float &t, float &h){
  h = dht.readHumidity();
  t = dht.readTemperature();
  static uint8_t failCount = 0;
  if (isnan(t) || isnan(h)) {
    if (++failCount >= 3) { dht.begin(); failCount = 0; }
    return false;
  }
  failCount = 0;
  return true;
}

inline uint16_t readLightRaw(){ return analogRead(LDR_PIN); }

inline bool readSensors(SensorData &out){
  float t, h;
  bool ok = readDHT(t, h);
  out.temperature = t;
  out.humidity    = h;
  out.lightRaw    = readLightRaw();
  out.lightPct    = lightPctCal(out.lightRaw);
  out.lightState  = lightStateFromPct(out.lightPct);
  out.ts          = millis();
  return ok;
}
