// #define BLYNK_TEMPLATE_ID "TMPL6RTfQXE9a"
// #define BLYNK_TEMPLATE_NAME "IOT01"
// #define BLYNK_AUTH_TOKEN "ZVM5rObDnQLNDaM7UeNA_8stDBPM-ddC"

#pragma once

#define BLYNK_TEMPLATE_ID   "TMPL6RTfQXE9a"
#define BLYNK_TEMPLATE_NAME "IOT01"
#define BLYNK_AUTH_TOKEN    "ZVM5rObDnQLNDaM7UeNA_8stDBPM-ddC"

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>
#include <Blynk/BlynkTimer.h>

// --- ตัวแปรและฟังก์ชันที่ใช้ร่วมกับ Lab_5-P.ino ---
extern bool autoMode;
extern bool relayOn;
void applyRelay();

float readTempC();
float readHumid();
float computeFeelLikeC(float t, float h);
int   readLDRRaw();
bool  isNightByLDR();
bool  isRaining();
bool  isLDRPresent();
bool  isRainPresent();

// ===== Virtual Pin mapping =====
#define VP_TEMP      V0
#define VP_HUMID     V1
#define VP_FEEL      V2
#define VP_LDR_RAW   V3
#define VP_IS_NIGHT  V4
#define VP_IS_RAIN   V5
#define VP_RELAY     V6
#define VP_AUTO      V7

static BlynkTimer blynkTimer;
static bool blynkStarted = false;

// ===== ส่งค่าไป Blynk ทุกครั้งที่ถูกเรียก =====
static void blynkPushOnce(){
  static float lastT = NAN, lastH = NAN, lastF = 0;

  float t = readTempC();
  float h = readHumid();
  if (!isnan(t)) lastT = t;
  if (!isnan(h)) lastH = h;
  if (!isnan(lastT) && !isnan(lastH)) lastF = computeFeelLikeC(lastT, lastH);

  int  ldr   = isLDRPresent() ? readLDRRaw() : -1;
  bool night = isLDRPresent() ? isNightByLDR() : false;
  bool rain  = isRainPresent() ? isRaining()   : false;

  Blynk.virtualWrite(V0, isnan(lastT) ? 0.0f : lastT);
  Blynk.virtualWrite(V1, isnan(lastH) ? 0.0f : lastH);
  Blynk.virtualWrite(V2, lastF);
  Blynk.virtualWrite(V3, ldr);
  Blynk.virtualWrite(V4, night ? 1 : 0);
  Blynk.virtualWrite(V5, rain  ? 1 : 0);
  Blynk.virtualWrite(V6, relayOn  ? 1 : 0);
  Blynk.virtualWrite(V7, autoMode ? 1 : 0);
}

// ===== เริ่ม Blynk แบบ non-blocking =====
inline void blynkBegin(){
  if (blynkStarted || !WiFi.isConnected()) return;
  WiFi.setSleep(false);                     // กันหลับแล้วเน็ตหลุด

  Serial.println("[Blynk] Config...");
  Blynk.config(BLYNK_AUTH_TOKEN, "sgp1.blynk.cloud", 80);

  Serial.println("[Blynk] Connecting...");
  bool ok = Blynk.connect(5000);            // รอ 5 วิ
  Serial.println(ok ? "[Blynk] Connected" : "[Blynk] Connect failed");

  blynkTimer.setInterval(6000, blynkPushOnce);
  blynkStarted = true;
}

// ===== ให้ Blynk ทำงานใน loop() =====
inline void blynkRun(){
  if (!blynkStarted) blynkBegin();
  if (blynkStarted && WiFi.isConnected()) {
    Blynk.run();
  }
  blynkTimer.run();
}

// ===== รับคำสั่งจาก Blynk (สวิตช์บน Dashboard) =====

// --- Auto Mode (V7) ---
BLYNK_WRITE(VP_AUTO) {
  int v = param.asInt();              // 0 หรือ 1 จากสวิตช์ Auto
  autoMode = (v != 0);
  Serial.printf("[Blynk] Auto=%d\n", v);

  if (autoMode) {
    // ถ้าเปิด Auto → ควบคุมรีเลย์ตาม LDR ทันที
    relayOn = isLDRPresent() ? isNightByLDR() : false;
    applyRelay();
  }

  // sync กลับเพื่อป้องกันค่าไม่ตรง
  Blynk.virtualWrite(VP_AUTO, autoMode ? 1 : 0);
}

// --- Relay Control (V6) ---
BLYNK_WRITE(VP_RELAY) {
  int v = param.asInt();              // 0 หรือ 1 จากสวิตช์ Relay
  Serial.printf("[Blynk] Relay=%d (auto=%d)\n", v, autoMode);

  if (!autoMode) {                    // ควบคุมได้เฉพาะตอน Manual
    relayOn = (v != 0);
    applyRelay();
  }

  Blynk.virtualWrite(VP_RELAY, relayOn ? 1 : 0);
}
