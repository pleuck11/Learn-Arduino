#pragma once
#include <Arduino.h>
#include <DHT.h>

/* ===== Pin Mapping (ตามที่คุณกำหนด) =====
 * D22 → LDR Sensor (Digital Out / DO)
 * D23 → DHT11 (Temperature + Humidity) [ขาเดียวอ่านได้ทั้งคู่]
 * D2  → Relay / LED
 * D4  → Rain Sensor (Digital Out / DO)
 */

// --- DHT11 (Temp+Humidity) ---
#define DHTPIN   23     // D23 → DHT11
#define DHTTYPE  DHT11
static DHT dht(DHTPIN, DHTTYPE);

// --- LDR (Digital) ---
#define LDR_ANALOG_MODE   0      // 1=analogRead, 0=digitalRead (ที่นี่ใช้ DO)
#define LDR_PIN_ANALOG    34     // ใช้กรณี analog mode เท่านั้น
#define LDR_PIN_DIGITAL   22     // D22 → LDR DO
static const int LDR_THRESHOLD = 1500; // สำหรับ analog mode

// --- Rain Sensor (Digital) ---
#define RAIN_PIN 4               // D4 → Rain DO (HIGH = ฝนตก, LOW = แห้ง)

// --- Relay / LED ---
#define RELAY_PIN 2              // D2 → Relay/LED
#define RELAY_ACTIVE_LOW 0       // 1=Active-Low, 0=Active-High

// ----- presence flags -----
static bool _ldrPresent  = true;
static bool _rainPresent = true;

inline bool isLDRPresent()  { return _ldrPresent; }
inline bool isRainPresent() { return _rainPresent; }

// ===== presence detection =====
inline bool detectRainPresentOnce() {
  pinMode(RAIN_PIN, INPUT);
  delay(2);
  int a = digitalRead(RAIN_PIN);
  pinMode(RAIN_PIN, INPUT_PULLUP);
  delay(2);
  int b = digitalRead(RAIN_PIN);
  pinMode(RAIN_PIN, INPUT);
  return (a == b); // true = มีโมดูลต่ออยู่
}

inline bool detectLDRPresentOnce() {
#if LDR_ANALOG_MODE
  int mn = 4096, mx = 0;
  for (int i=0;i<16;i++) {
    int v = analogRead(LDR_PIN_ANALOG);
    mn = min(mn,v);
    mx = max(mx,v);
    delay(2);
  }
  int span = mx - mn;
  bool stuckNearEdge = (mx > 4085 || mn < 10);
  if (span < 4 && stuckNearEdge) return false;
  return true;
#else
  // Digital DO: ลองสลับ pullup → ถ้าขาลอยผลจะไม่เสถียร
  pinMode(LDR_PIN_DIGITAL, INPUT);
  delay(2); int a = digitalRead(LDR_PIN_DIGITAL);
  pinMode(LDR_PIN_DIGITAL, INPUT_PULLUP);
  delay(2); int b = digitalRead(LDR_PIN_DIGITAL);
  pinMode(LDR_PIN_DIGITAL, INPUT);
  return (a == b);
#endif
}

inline void refreshPresenceCheck() {
  _rainPresent = detectRainPresentOnce();
  _ldrPresent  = detectLDRPresentOnce();
}

// ===== Init =====
inline void initSensors() {
  dht.begin();

#if LDR_ANALOG_MODE
  pinMode(LDR_PIN_ANALOG, INPUT);
#else
  pinMode(LDR_PIN_DIGITAL, INPUT);
#endif
  pinMode(RAIN_PIN, INPUT);

#if RELAY_ACTIVE_LOW
  digitalWrite(RELAY_PIN, LOW);   // ปิดก่อนค่อยเซ็ต OUTPUT
#else
  digitalWrite(RELAY_PIN, HIGH);
#endif
  pinMode(RELAY_PIN, OUTPUT);

  refreshPresenceCheck();
}

// ===== DHT =====
inline float readTempC() { float t=dht.readTemperature(); return isnan(t)?NAN:t; }
inline float readHumid() { float h=dht.readHumidity();    return isnan(h)?NAN:h; }
inline float computeFeelLikeC(float t,float h){
  return (isnan(t)||isnan(h))?0.0f:dht.computeHeatIndex(t,h,false);
}

// ===== LDR =====
inline int  readLDRRaw(){
  if (!_ldrPresent) return -1;
#if LDR_ANALOG_MODE
  return analogRead(LDR_PIN_ANALOG); // 0..4095
#else
  return digitalRead(LDR_PIN_DIGITAL)?4095:0; // DO: 1=สว่าง, 0=มืด
#endif
}
inline bool isNightByLDR(){
  if (!_ldrPresent) return false;
#if LDR_ANALOG_MODE
  return readLDRRaw() < LDR_THRESHOLD;
#else
  return digitalRead(LDR_PIN_DIGITAL) == LOW; // DO=0 = มืด
#endif
}

// ===== Rain (แก้สลับค่า) =====
inline bool isRaining(){
  if (!_rainPresent) return false;
  // เดิม: LOW=ฝนตก → แก้ให้ HIGH=ฝนตก
  return digitalRead(RAIN_PIN) == HIGH;
}

// ===== Relay =====
inline void setRelay(bool on){
#if RELAY_ACTIVE_LOW
  digitalWrite(RELAY_PIN, on?LOW:HIGH);
#else
  digitalWrite(RELAY_PIN, on?HIGH:LOW);
#endif
}
