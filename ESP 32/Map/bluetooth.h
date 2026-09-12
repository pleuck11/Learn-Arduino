#pragma once
/* bluetooth.h — Bluetooth SPP (Classic) command parser for navigation */

#include <Arduino.h>
#include <BluetoothSerial.h>
#include "sensors.h"

// ของจริงอยู่ใน Map.ino
extern BluetoothSerial SerialBT;

// ---------- Utils ----------
inline String upperAscii(String s) {
  String r = s;
  for (size_t i = 0; i < r.length(); i++) {
    char c = r[i];
    if (c >= 'a' && c <= 'z') r.setCharAt(i, c - 32);
  }
  return r;
}

// ---------- Parser ----------
inline void btHandleLine(const String& rawLine) {
  String line = rawLine; line.trim();
  if (!line.length()) return;

  String U = upperAscii(line);

  if (U == "START")   { navSetStatus("NAV START"); navSetTurn(TURN_NONE); navSetDistanceMeters(0); return; }
  if (U == "ARRIVED") { navSetStatus("ARRIVED");   navSetTurn(TURN_NONE); navSetDistanceMeters(0); return; }
  if (U == "CLEAR" || U == "STOP") { navClear(); return; }

  if (U.startsWith("TURN:")) {
    String v = U.substring(5); v.trim();
    if (v == "LEFT")          navSetTurn(TURN_LEFT);
    else if (v == "RIGHT")    navSetTurn(TURN_RIGHT);
    else if (v == "STRAIGHT") navSetTurn(TURN_STRAIGHT);
    else if (v == "UTURN" || v == "U-TURN" || v == "U") navSetTurn(TURN_UTURN);
    navSetStatus("NAV");
    return;
  }

  if (U.startsWith("DIST:")) {
    String v = U.substring(5); v.trim();
    float value = 0.0f; String num = "";
    for (size_t i = 0; i < v.length(); i++) {
      char c = v[i]; if ((c >= '0' && c <= '9') || c == '.') num += c;
    }
    if (num.length()) value = num.toFloat();
    if (v.indexOf("KM") >= 0) navSetDistanceMeters(value * 1000.0f);
    else                      navSetDistanceMeters(value);
    return;
  }

  if (U.startsWith("ROAD:")) { String real = rawLine.substring(5); real.trim(); navSetRoad(real); return; }
  if (U.startsWith("ETA:"))  { String real = rawLine.substring(4); real.trim(); navSetETA(real); return; }

  // ไม่รู้จักคำสั่ง -> โชว์เป็นสถานะ
  navSetStatus(rawLine);
}

// ---------- Public API ----------
inline void btInit(const char* name) {
  // ต้องเป็น ESP32 รุ่นที่รองรับ Bluetooth Classic
  if (!SerialBT.begin(name)) {
    Serial.println(F("[BT] init failed"));
    navSetStatus("BT init failed!");
  } else {
    Serial.print(F("[BT] started as: "));
    Serial.println(name);
    navSetStatus("BT Ready");
  }
}

inline void btPoll() {
  static String rxLine;
  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    if (c == '\r') continue;
    if (c == '\n') {
      btHandleLine(rxLine);
      rxLine = "";
    } else {
      if (rxLine.length() < 120) rxLine += c;
    }
  }
}
