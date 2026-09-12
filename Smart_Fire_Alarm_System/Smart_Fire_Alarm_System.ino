#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// ---- ตั้งเกณฑ์อุณหภูมิที่บังคับเปิดปั๊ม ----
#ifndef PUMP_HOT_C
  #define PUMP_HOT_C 35.0f   // >= 30°C -> ปั๊ม ON
#endif

// ปรับก่อน include sensors.h ได้ตามต้องการ
#define BUZZER_IS_PASSIVE 1   // MH-FMD (ทดสอบ #3 ดัง)

#include "sensors.h"
#include "index_html.h"
#include "manager_html.h"

// ---------------- System state ----------------
Preferences prefs;
WebServer server(80);

const char* PREFS_NS = "wifi";
const char* KEY_SSID = "ssid";
const char* KEY_PASS = "pass";

String wifiSsid = "";
String wifiPass = "";

const char* AP_SSID = "SMART-FIRE-SETUP";
const char* AP_PASS = "12345678";
IPAddress AP_IP(192,168,4,1), AP_GW(192,168,4,1), AP_MASK(255,255,255,0);

// Devices state
bool relayState    = false;  // ปั๊มน้ำ/รีเลย์ (จริง)
bool buzzerState   = false;  // บัซเซอร์ (จริง)
bool buzzerDesired = false;  // ปุ่มใน Manual (ตอนปกติ)
bool pumpDesired   = false;  // ปุ่มใน Manual (ตอนปกติ)

// RGB override: ให้ผู้ใช้บังคับไฟ (ทั้ง Manual/Auto) ค้างไว้จนกว่าจะมีการเปลี่ยนสถานะควัน
bool rgbOverride   = false;
bool lastSmoke     = false;

// Mode
bool autoMode = false;

// Manual groups
bool enSmoke  = true;
bool enPump   = true;
bool enBuzzer = true;
bool enRGB    = true;

// ---------------- Utils ----------------
String jsonEscape(const String& s) {
  String o; o.reserve(s.length()+4);
  for (char c: s){
    if(c=='\"'||c=='\\'){ o+='\\'; o+=c; }
    else if(c=='\b') o+="\\b";
    else if(c=='\f') o+="\\f";
    else if(c=='\n') o+="\\n";
    else if(c=='\r') o+="\\r";
    else if(c=='\t') o+="\\t";
    else o+=c;
  }
  return o;
}

String buildStatusJson() {
  Sensors::sampleTemperature();               // อ่าน DHT (มีการหน่วงในไลบรารี)
  bool smoke = Sensors::readSmoke();

  uint8_t rr,gg,bb; Sensors::getRgb(rr,gg,bb);
  float    tC   = Sensors::temperatureC();
  uint16_t tRaw = Sensors::temperatureRaw();  // ใช้เป็น %RH

  String j = "{";
  j += "\"wifi_connected\":" + String(WiFi.isConnected() ? "true":"false") + ",";
  j += "\"ip\":\""   + WiFi.localIP().toString() + "\",";
  j += "\"ssid\":\"" + jsonEscape(WiFi.SSID()) + "\",";
  j += "\"rssi\":"   + String(WiFi.isConnected()? WiFi.RSSI() : 0) + ",";
  j += "\"auto\":"   + String(autoMode ? "true":"false") + ",";

  j += "\"groups\":{";
  j += "\"smoke\":"  + String(enSmoke?"true":"false") + ",";
  j += "\"pump\":"   + String(enPump?"true":"false") + ",";
  j += "\"buzzer\":" + String(enBuzzer?"true":"false") + ",";
  j += "\"rgb\":"    + String(enRGB?"true":"false");
  j += "},";

  j += "\"smoke\":" + String(smoke?"true":"false") + ",";
  j += "\"temp_c\":";
  if (isnan(tC)) j += "null"; else j += String(tC,2);
  j += ",";
  j += "\"temp_raw\":" + String(tRaw) + ",";

  j += "\"relay\":"  + String(Sensors::relayState() ? "true":"false") + ",";
  j += "\"buzzer\":" + String(Sensors::buzzerState() ? "true":"false") + ",";
  j += "\"rgb\":{\"r\":" + String(rr) + ",\"g\":" + String(gg) + ",\"b\":" + String(bb) + "}";
  j += "}";
  return j;
}

// ---------------- Handlers ----------------
void handleRoot()    { server.send(200, "text/html; charset=utf-8", INDEX_HTML); }
void handleManager() { server.send(200, "text/html; charset=utf-8", MANAGER_HTML); }
void handleStatus()  { server.send(200, "application/json", buildStatusJson()); }

// ปั๊มน้ำ / รีเลย์
void handleRelay() {
  if (!autoMode && !enPump) { server.send(403, "text/plain", "pump disabled in manual"); return; }
  if (!server.hasArg("on")) { server.send(400, "text/plain", "missing ?on=0|1"); return; }

  bool wantOn = (server.arg("on")=="1"||server.arg("on")=="true");
  pumpDesired = wantOn;

  if (!autoMode) {
    bool smokeNow = Sensors::readSmoke();
    bool target = smokeNow ? true : pumpDesired;
    Sensors::writeRelay(target);
    relayState = target;
  } else {
    relayState = Sensors::relayState();
  }
  server.send(200, "application/json", "{\"ok\":true}");
}

// บัซเซอร์
void handleBuzzer() {
  if (!autoMode && !enBuzzer) { server.send(403, "text/plain", "buzzer disabled in manual"); return; }
  if (!server.hasArg("on"))   { server.send(400, "text/plain", "missing ?on=0|1"); return; }

  bool wantOn = (server.arg("on")=="1"||server.arg("on")=="true");
  buzzerDesired = wantOn;

  if (!autoMode) {
    bool smokeNow = Sensors::readSmoke();
    bool target = smokeNow ? true : (enBuzzer ? buzzerDesired : false);
    Sensors::writeBuzzer(target);
    buzzerState = target;
  } else {
    buzzerState = Sensors::buzzerState();
  }
  server.send(200, "application/json", "{\"ok\":true}");
}

// RGB — ให้สั่งได้ทุกโหมด (Manual ต้องไม่ disable)
void handleRGB() {
  if (!server.hasArg("r") || !server.hasArg("g") || !server.hasArg("b")) {
    server.send(400, "text/plain", "missing r,g,b"); return;
  }
  if (!autoMode && !enRGB) {
    server.send(403, "text/plain", "rgb disabled in manual"); return;
  }
  int r = constrain(server.arg("r").toInt(),0,255);
  int g = constrain(server.arg("g").toInt(),0,255);
  int b = constrain(server.arg("b").toInt(),0,255);

  Sensors::writeRgb(r,g,b);
  rgbOverride = true;   // ผู้ใช้บังคับไฟ
  server.send(200,"application/json","{\"ok\":true}");
}

void handleMode() {
  if (!server.hasArg("m")) { server.send(400,"text/plain","missing ?m=manual|auto"); return; }
  autoMode = (server.arg("m")=="auto");
  server.send(200,"application/json","{\"ok\":true}");
}

void handleEnable() {
  if (!server.hasArg("device")||!server.hasArg("on")) { server.send(400,"text/plain","missing"); return; }
  String d=server.arg("device");
  bool on=(server.arg("on")=="1"||server.arg("on")=="true");
  if(d=="smoke")  enSmoke=on;
  else if(d=="pump")   enPump=on;
  else if(d=="buzzer") enBuzzer=on;
  else if(d=="rgb")    enRGB=on;
  server.send(200,"application/json","{\"ok\":true}");
}

// ====== Wi-Fi ======
void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.println("[WiFi] --- AP Mode ---");
  Serial.printf("[WiFi] SSID:%s PASS:%s\n",AP_SSID,AP_PASS);
  Serial.printf("[WiFi] IP:%s\n",WiFi.softAPIP().toString().c_str());
}
bool startSTA(const String& ssid,const String& pass,uint32_t timeoutMs=15000){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(),pass.c_str());
  Serial.printf("[WiFi] Connecting to %s\n",ssid.c_str());
  uint32_t t0=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t0<timeoutMs){ delay(300); Serial.print("."); }
  Serial.println();
  if(WiFi.status()==WL_CONNECTED){ Serial.printf("[WiFi] Connected. IP:%s\n",WiFi.localIP().toString().c_str()); return true; }
  Serial.println("[WiFi] Connect failed."); return false;
}

// เพิ่ม /savewifi ไว้แล้วในเวอร์ชันก่อนหน้านี้

void setupServer() {
  server.on("/",HTTP_GET,handleRoot);
  server.on("/manager",HTTP_GET,handleManager);
  server.on("/api/status",HTTP_GET,handleStatus);
  server.on("/api/relay",HTTP_GET,handleRelay);
  server.on("/api/buzzer",HTTP_GET,handleBuzzer);
  server.on("/api/rgb",HTTP_GET,handleRGB);
  server.on("/api/mode",HTTP_GET,handleMode);
  server.on("/api/enable",HTTP_GET,handleEnable);

  // บันทึก Wi-Fi (รองรับทั้ง POST/GET)
  server.on("/savewifi",HTTP_POST,[](){  // ย้าย body ไปฟังก์ชัน inline เพื่อย่อโค้ด
    String ssid = server.hasArg("ssid") ? server.arg("ssid") : "";
    String pass = server.hasArg("pass") ? server.arg("pass") : "";
    if (ssid.length()==0){ server.send(400,"text/plain","missing ssid"); return; }
    prefs.begin(PREFS_NS,false);
    prefs.putString(KEY_SSID, ssid);
    prefs.putString(KEY_PASS, pass);
    prefs.end();
    bool ok = startSTA(ssid, pass, 10000);
    server.send(200,"text/plain", ok ? "Saved & connected. Rebooting..." : "Saved, but connect failed. Rebooting...");
    delay(500); ESP.restart();
  });
  server.on("/savewifi",HTTP_GET,[](){ server.send(405,"text/plain","POST only"); });

  server.begin();
}

// ---------------- Setup / Loop ----------------
void setup(){
  Serial.begin(115200); delay(100);

  Sensors::begin();
  Sensors::rgbSelfTest();

  lastSmoke = Sensors::readSmoke();

  prefs.begin(PREFS_NS,true);
  wifiSsid=prefs.getString(KEY_SSID,"");
  wifiPass=prefs.getString(KEY_PASS,"");
  prefs.end();

  bool ok = wifiSsid.length()? startSTA(wifiSsid,wifiPass):false;
  if(!ok) startAP();

  setupServer();
  Serial.println("Setup done.");
}

void loop(){
  server.handleClient();

  // อ่านเซ็นเซอร์หลัก
  Sensors::sampleTemperature();                 // มี rate limit ภายใน
  float tC   = Sensors::temperatureC();
  bool  hot  = (!isnan(tC) && tC >= PUMP_HOT_C); // >= เกณฑ์ -> บังคับปั๊ม ON
  bool  smoke = Sensors::readSmoke();

  // ถ้าสถานะควันเปลี่ยน -> ยกเลิก override ของผู้ใช้
  if (smoke != lastSmoke) { lastSmoke = smoke; rgbOverride = false; }

  // ---------- กฎแรก: อุณหภูมิสูง (Fail-safe) ----------
  if (hot) {
    Sensors::writeRelay(true);
    relayState = true;
  } else
  // ---------- โหมด Auto ----------
  if (autoMode) {
    Sensors::writeBuzzer(smoke); buzzerState = smoke;

    if (enPump) { Sensors::writeRelay(smoke); relayState = smoke; }
    else        { Sensors::writeRelay(false); relayState = false; }

    if (enRGB && !rgbOverride) {
      if (smoke) Sensors::writeRgb(255,0,0);
      else       Sensors::writeRgb(0,255,0);
    }
  } else {
    // ---------- โหมด Manual ----------
    if (smoke && enBuzzer) { Sensors::writeBuzzer(true); buzzerState = true; }
    else {
      bool target = enBuzzer ? buzzerDesired : false;
      Sensors::writeBuzzer(target); buzzerState = target;
    }

    if (enPump) {
      bool target = smoke ? true : pumpDesired;  // ถ้าไม่ร้อนและไม่ควัน -> ตามปุ่ม
      Sensors::writeRelay(target); relayState = target;
    } else {
      Sensors::writeRelay(false); relayState = false;
    }

    if (enRGB && !rgbOverride) {
      if (smoke) Sensors::writeRgb(255,0,0);
      else       Sensors::writeRgb(0,255,0);
    }
  }
}
