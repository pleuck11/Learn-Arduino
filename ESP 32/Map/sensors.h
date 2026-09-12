#pragma once
/* sensors.h — OLED helpers + shared nav types (SH1106, non-blocking if OLED missing) */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#define OLED_DRIVER_NAME "SH1106"

// ---------- ชนิดข้อมูลนำทาง ----------
enum TurnType { TURN_NONE, TURN_LEFT, TURN_RIGHT, TURN_STRAIGHT, TURN_UTURN };

struct NavData {
  TurnType turn = TURN_NONE;
  float    distMeters = 0.0f;   // หน่วยเมตร
  String   road = "";
  String   eta  = "";
  String   status = "READY";
};

// ---------- ของจริงอยู่ใน Map.ino ----------
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
extern NavData g_nav;
extern bool    g_display_ok;
extern uint8_t g_oled_addr;   // 0x3C/0x3D

// ---------- I2C helpers ----------
inline bool i2cPresent(uint8_t addr7) {
  Wire.beginTransmission(addr7);
  return (Wire.endTransmission() == 0);
}

inline uint8_t detectOLED() {
  bool has3C = i2cPresent(0x3C);
  bool has3D = i2cPresent(0x3D);

  if (has3C) Serial.println(F("[I2C] found device at 0x3C"));
  if (has3D) Serial.println(F("[I2C] found device at 0x3D"));

  if (has3C) return 0x3C;
  if (has3D) return 0x3D;
  return 0x00; // ไม่พบ
}

// ---------- UI helpers ----------
inline String formatDistance(float meters) {
  if (meters < 1000.0f) {
    int m = (int)round(meters);
    return String(m) + " m";
  } else {
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f km", meters / 1000.0f);
    return String(buf);
  }
}

inline void drawThickLine(int x0, int y0, int x1, int y1, uint8_t t = 3) {
  for (int i = -((int)t/2); i <= (int)(t/2); i++) {
    u8g2.drawLine(x0, y0+i, x1, y1+i);
  }
}

inline void drawArrow(TurnType t) {
  switch (t) {
    case TURN_LEFT:
      drawThickLine(84, 36, 44, 36, 3);
      u8g2.drawTriangle(44, 36, 60, 28, 60, 44);
      break;
    case TURN_RIGHT:
      drawThickLine(44, 36, 84, 36, 3);
      u8g2.drawTriangle(84, 36, 68, 28, 68, 44);
      break;
    case TURN_STRAIGHT:
      drawThickLine(64, 52, 64, 20, 3);
      u8g2.drawTriangle(64, 12, 56, 24, 72, 24);
      break;
    case TURN_UTURN:
      for (int r = 10; r <= 14; r += 2) {
        u8g2.drawCircle(64, 36, r, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
      }
      drawThickLine(64, 36, 64, 48, 3);
      u8g2.drawTriangle(64, 56, 56, 44, 72, 44);
      break;
    case TURN_NONE:
    default:
      break;
  }
}

// ---------- Display API ----------
inline void displayInit() {
  g_display_ok = false;

  Serial.print(F("[OLED] driver = "));
  Serial.println(F(OLED_DRIVER_NAME));

  // สแกนหา address
  uint8_t found = detectOLED();
  if (found == 0x00) {
    Serial.println(F("[OLED] not found at 0x3C/0x3D — BT-only mode"));
    return;  // ไม่พบบนบัส -> ไม่บล็อก loop
  }

  g_oled_addr = found;

  // u8g2 ใช้ 8-bit address => addr7 * 2
  u8g2.setI2CAddress(g_oled_addr * 2);
  u8g2.begin();
  g_display_ok = true;

  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(6, 28, "ESP32 NAV DISPLAY");
  u8g2.drawStr(6, 44, "OLED OK @ 0x3C/0x3D");
  u8g2.sendBuffer();

  Serial.print(F("[OLED] ready @ 0x"));
  Serial.println(g_oled_addr, HEX);
}

inline void displayUpdate() {
  if (!g_display_ok) return;  // จอไม่พร้อม -> ไม่วาด

  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 64);

  // ระยะทาง (ใหญ่) + ETA
  String distStr = formatDistance(g_nav.distMeters);
  u8g2.setFont(u8g2_font_logisoso18_tf);
  int distW = u8g2.getStrWidth(distStr.c_str());
  u8g2.drawStr((128 - distW)/2, 20, distStr.c_str());

  if (g_nav.eta.length()) {
    u8g2.setFont(u8g2_font_6x12_tf);
    int etaW = u8g2.getStrWidth(g_nav.eta.c_str());
    u8g2.drawStr(126 - etaW, 10, g_nav.eta.c_str());
  }

  // ลูกศร
  drawArrow(g_nav.turn);

  // แถบล่าง: road/status
  u8g2.setFont(u8g2_font_6x12_tf);
  String bottom = g_nav.road.length() ? g_nav.road : g_nav.status;
  if (bottom.length() > 20) bottom = bottom.substring(0, 20);
  u8g2.drawStr(4, 60, bottom.c_str());

  // BT indicator
  u8g2.drawStr(4, 10, "BT");

  u8g2.sendBuffer();
}

// ----- Setters -----
inline void navSetTurn(TurnType t)        { g_nav.turn = t; displayUpdate(); }
inline void navSetDistanceMeters(float m) { g_nav.distMeters = m; displayUpdate(); }
inline void navSetRoad(const String& s)   { g_nav.road = s; displayUpdate(); }
inline void navSetETA(const String& s)    { g_nav.eta = s; displayUpdate(); }
inline void navSetStatus(const String& s) { g_nav.status = s; displayUpdate(); }

inline void navClear() {
  g_nav.turn = TURN_NONE;
  g_nav.distMeters = 0;
  g_nav.road = "";
  g_nav.eta = "";
  g_nav.status = "READY";
  displayUpdate();
}
