#pragma once
#include <Arduino.h>
#include <math.h>
#include <DHTesp.h>   // "DHT sensor library for ESPx" (beegee_tokyo)

/* ====== Logging switch (0=ปิด, 1=เปิด) ====== */
#ifndef SENSORS_LOG
  #define SENSORS_LOG 0
#endif

/* ====== ปรับความถี่การอ่าน DHT11 (อย่างน้อย ~1000ms) ====== */
#ifndef DHT_MIN_PERIOD_MS
  #define DHT_MIN_PERIOD_MS 1200   // ตั้งค่าเริ่มต้น 1200 ms
#endif

/* ====== Pin config (override ได้ก่อน include) ====== */
#ifndef PIN_SMOKE_DO
  #define PIN_SMOKE_DO   23
#endif
#ifndef PIN_DHT
  #define PIN_DHT        22
#endif
#ifndef PIN_RELAY
  #define PIN_RELAY      4
#endif
#ifndef PIN_BUZZER
  #define PIN_BUZZER     27
#endif
#ifndef PIN_RGB_R
  #define PIN_RGB_R      25
#endif
#ifndef PIN_RGB_G
  #define PIN_RGB_G      32
#endif
#ifndef PIN_RGB_B
  #define PIN_RGB_B      33
#endif

/* ====== Logic options ====== */
#ifndef SMOKE_ACTIVE_LOW
  #define SMOKE_ACTIVE_LOW 1
#endif
#ifndef SMOKE_USE_PULLUP
  #define SMOKE_USE_PULLUP 0
#endif
#ifndef RELAY_ACTIVE_HIGH
  #define RELAY_ACTIVE_HIGH 1
#endif
#ifndef BUZZER_ACTIVE_HIGH
  #define BUZZER_ACTIVE_HIGH 1
#endif
#ifndef RGB_COMMON_ANODE_DEFAULT
  #define RGB_COMMON_ANODE_DEFAULT 0
#endif

#ifndef BUZZER_IS_PASSIVE
  #define BUZZER_IS_PASSIVE 1
#endif
#ifndef BUZZER_TONE_FREQ
  #define BUZZER_TONE_FREQ 3000
#endif

namespace Sensors {

  static bool    s_rgbCommonAnode = (RGB_COMMON_ANODE_DEFAULT != 0);
  static uint8_t s_r = 0, s_g = 0, s_b = 0;
  static bool    s_isRed = false;

  static bool s_relay  = false;
  static bool s_buzzer = false;

  static bool s_smoke  = false;

  static DHTesp   s_dht;
  static uint32_t s_dhtLastMs = 0;
  static uint16_t s_dhtPeriodMs = DHT_MIN_PERIOD_MS;  // runtime adjustable
  static float    s_tempC     = NAN;
  static float    s_humPct    = NAN;

  inline void _applyRgb(uint8_t r, uint8_t g, uint8_t b) {
    s_r = r; s_g = g; s_b = b;
    uint8_t vr = s_rgbCommonAnode ? (255 - r) : r;
    uint8_t vg = s_rgbCommonAnode ? (255 - g) : g;
    uint8_t vb = s_rgbCommonAnode ? (255 - b) : b;
    analogWrite(PIN_RGB_R, vr);
    analogWrite(PIN_RGB_G, vg);
    analogWrite(PIN_RGB_B, vb);
  }

  inline void begin() {
  #if SMOKE_USE_PULLUP
    pinMode(PIN_SMOKE_DO, INPUT_PULLUP);
  #else
    pinMode(PIN_SMOKE_DO, INPUT);
  #endif
    s_smoke = SMOKE_ACTIVE_LOW ? (digitalRead(PIN_SMOKE_DO) == LOW)
                               : (digitalRead(PIN_SMOKE_DO) == HIGH);

    pinMode(PIN_RELAY, OUTPUT);
    s_relay = false;
    digitalWrite(PIN_RELAY, RELAY_ACTIVE_HIGH ? LOW : HIGH);

    pinMode(PIN_BUZZER, OUTPUT);
  #if BUZZER_IS_PASSIVE
    noTone(PIN_BUZZER);
  #else
    s_buzzer = false;
    digitalWrite(PIN_BUZZER, BUZZER_ACTIVE_HIGH ? LOW : HIGH);
  #endif

    pinMode(PIN_RGB_R, OUTPUT);
    pinMode(PIN_RGB_G, OUTPUT);
    pinMode(PIN_RGB_B, OUTPUT);
    _applyRgb(0,0,0);

    s_dht.setup(PIN_DHT, DHTesp::DHT11);
  #if SENSORS_LOG
    Serial.print("[DHT] setup on GPIO"); Serial.print(PIN_DHT);
    Serial.println(" (DHT11)");
  #endif
  }

  // ปรับช่วงเวลาอ่าน DHT ระหว่างรัน (บังคับขั้นต่ำ 1000ms)
  inline void setDhtReadPeriod(uint16_t ms){
    if (ms < 1000) ms = 1000;
    s_dhtPeriodMs = ms;
  }

  inline void setRgbCommonAnode(bool c){ s_rgbCommonAnode=c; _applyRgb(s_r,s_g,s_b); }
  inline void setRgbRed(){ s_isRed=true; _applyRgb(255,0,0); }
  inline void setRgbBlue(){ s_isRed=false; _applyRgb(0,0,255); }
  inline void toggleRgbRB(){ if(s_isRed) setRgbBlue(); else setRgbRed(); }
  inline void writeRgb(uint8_t r,uint8_t g,uint8_t b){ _applyRgb(r,g,b); }
  inline void getRgb(uint8_t &r,uint8_t &g,uint8_t &b){ r=s_r; g=s_g; b=s_b; }
  inline void rgbSelfTest(){ setRgbRed(); delay(300); setRgbBlue(); delay(300); setRgbRed(); delay(150); setRgbBlue(); delay(150); _applyRgb(0,0,0); }

  inline void writeRelay(bool on){
    s_relay=on;
    digitalWrite(PIN_RELAY, RELAY_ACTIVE_HIGH ? (on?HIGH:LOW) : (on?LOW:HIGH));
  }
  inline bool relayState(){ return s_relay; }

  inline void writeBuzzer(bool on){
    s_buzzer=on;
  #if BUZZER_IS_PASSIVE
    if(on) tone(PIN_BUZZER, BUZZER_TONE_FREQ);
    else   noTone(PIN_BUZZER);
  #else
    digitalWrite(PIN_BUZZER, BUZZER_ACTIVE_HIGH ? (on?HIGH:LOW) : (on?LOW:HIGH));
  #endif
  }
  inline bool buzzerState(){ return s_buzzer; }

  inline bool readSmoke(){
    int d=digitalRead(PIN_SMOKE_DO);
    s_smoke = SMOKE_ACTIVE_LOW ? (d==LOW) : (d==HIGH);
    return s_smoke;
  }

  inline void sampleTemperature(){
    uint32_t now = millis();
    if (now - s_dhtLastMs < s_dhtPeriodMs) return;
    s_dhtLastMs = now;

    TempAndHumidity th = s_dht.getTempAndHumidity();
    if (isnan(th.temperature) || isnan(th.humidity)) {
    #if SENSORS_LOG
      const char* st = s_dht.getStatusString();
      Serial.print("[DHT] read FAIL (t="); Serial.print(th.temperature, 2);
      Serial.print(" h="); Serial.print(th.humidity, 2);
      Serial.print(") status="); Serial.println(st);
    #endif
      return;
    }
    s_tempC  = th.temperature;
    s_humPct = th.humidity;
  #if SENSORS_LOG
    Serial.print("[DHT] T="); Serial.print(s_tempC, 2); Serial.print("°C  H=");
    Serial.print(s_humPct, 1); Serial.println("%");
  #endif
  }

  inline float    temperatureC(){ return s_tempC; }
  inline uint16_t temperatureRaw(){ return isnan(s_humPct) ? 0 : (uint16_t)lroundf(s_humPct); } // ใช้ %RH
}
