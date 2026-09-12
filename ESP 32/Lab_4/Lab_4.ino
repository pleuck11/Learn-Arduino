#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <HTTPClient.h>

#include "index_html.h"
#include "manager_html.h"
#include "sensors.h"

// ---------- กำหนดค่าหลัก ----------
const char* AP_SSID     = "ESP32-Setup";
const char* AP_PASSWORD = "12345678";   // อย่างน้อย 8 ตัวอักษร
const byte  DNS_PORT    = 53;

// >>> เปลี่ยนปลายทาง PHP ของคุณที่นี่ <<<
String PHP_ENDPOINT = "http://192.168.100.40:8088/php/insert.php";  // <-- แก้ให้ตรงกับเซิร์ฟเวอร์คุณ

// ความถี่อัปโหลดขึ้น PHP (มิลลิวินาที)
const unsigned long UPLOAD_INTERVAL_MS   = 5000;  // ทุก 5 วินาที
const unsigned long PRESENCE_REFRESH_MS  = 15000; // รีเฟรชเช็คการต่อเซนเซอร์ทุก 15 วิ

// ---------- สถานะระบบ ----------
Preferences prefs;
WebServer  server(80);
DNSServer  dnsServer;

bool   inPortal   = false;  // อยู่ในโหมด AP/Portal หรือไม่
bool   autoMode   = true;   // Auto = เปิดไฟตาม LDR
bool   relayOn    = false;  // สถานะรีเลย์ปัจจุบัน (สำหรับ manual)
String savedSSID, savedPASS;

unsigned long lastUploadMs   = 0;
unsigned long lastPresenceMs = 0;

// ---------- โปรโตไทป์ ----------
bool  tryConnectFromNVS();
void  startPortal();
void  stopPortal();
bool  captivePortalRedirect();
void  handleRoot();
void  handleManager();
void  handleScan();
void  handleSave();
void  handleStatus();
void  handleAct();
void  applyRelay(bool on);
void  periodicUploadToPHP();

// =================================
//             SETUP
// =================================
void setup() {
  Serial.begin(115200);
  delay(150);

  initSensors();  // จาก sensors.h

  prefs.begin("wifi", false); // NVS namespace

  // พยายามเชื่อมต่อด้วยค่าที่บันทึกไว้
  bool ok = tryConnectFromNVS();
  if (!ok) {
    Serial.println("[WiFi] No saved Wi-Fi or connect failed → start AP portal");
    startPortal();
  }

  // -------- Routes --------
  server.on("/", HTTP_GET, handleRoot);
  server.on("/home", HTTP_GET, handleRoot);

  server.on("/manager", HTTP_GET, handleManager);
  server.on("/scan",    HTTP_GET, handleScan);
  server.on("/save",    HTTP_POST, handleSave);

  server.on("/status.json", HTTP_GET, handleStatus);
  server.on("/act",         HTTP_POST, handleAct);

  // fallback captive portal
  server.onNotFound([]() {
    if (inPortal && captivePortalRedirect()) return;
    server.send(404, "text/plain; charset=utf-8", "404 Not Found");
  });

  server.begin();
  Serial.println("[HTTP] Server started");
}

// =================================
//              LOOP
// =================================
void loop() {
  if (inPortal) {
    dnsServer.processNextRequest();
  }
  server.handleClient();

  // รีเฟรชการมีอยู่ของเซนเซอร์เป็นระยะ
  unsigned long now = millis();
  if (now - lastPresenceMs >= PRESENCE_REFRESH_MS) {
    lastPresenceMs = now;
    refreshPresenceCheck();
  }

  // โหมด Auto: คุมรีเลย์ตาม LDR กลางวัน/กลางคืน
  if (autoMode) {
    bool night = isNightByLDR();
    applyRelay(night);
  }

  // อัปโหลดขึ้น PHP ทุกช่วงที่กำหนด (เมื่ออยู่ในโหมด STA เท่านั้น)
  if (WiFi.status() == WL_CONNECTED && (now - lastUploadMs >= UPLOAD_INTERVAL_MS)) {
    lastUploadMs = now;
    periodicUploadToPHP();
  }
}

// =================================
//        Wi-Fi Connect/Portal
// =================================
bool tryConnectFromNVS() {
  savedSSID = prefs.getString("ssid", "");
  savedPASS = prefs.getString("pass", "");

  if (savedSSID.isEmpty()) return false;

  Serial.printf("[WiFi] Try connect saved SSID: %s\n", savedSSID.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSSID.c_str(), savedPASS.c_str());

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 12000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    inPortal = false;
    IPAddress ip = WiFi.localIP();
    Serial.printf("[WiFi] Connected! SSID=%s  IP=%s  RSSI=%d dBm\n",
                  WiFi.SSID().c_str(), ip.toString().c_str(), WiFi.RSSI());
    return true;
  }
  return false;
}

void startPortal() {
  inPortal = true;

  WiFi.mode(WIFI_AP);
  bool apok = WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress apIP = WiFi.softAPIP();
  Serial.printf("[AP] %s  IP=%s  (%s)\n", AP_SSID, apIP.toString().c_str(), apok ? "ok" : "fail");

  dnsServer.start(DNS_PORT, "*", apIP);
}

void stopPortal() {
  if (!inPortal) return;
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  inPortal = false;
}

// คืนค่า true ถ้าทำ redirect แล้ว
bool captivePortalRedirect() {
  if (!inPortal) return false;
  IPAddress apIP = WiFi.softAPIP();
  String host = server.hostHeader();
  if (!host.equals(apIP.toString())) {
    String redirectUrl = String("http://") + apIP.toString() + "/manager";
    server.sendHeader("Location", redirectUrl, true);
    server.send(302, "text/plain", "");
    return true;
  }
  return false;
}

// =================================
//             Handlers
// =================================
void handleRoot() {
  // หน้า Dashboard
  server.send(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleManager() {
  // แทนค่า {{AP_IP}} ใน MANAGER_HTML
  IPAddress apIP = WiFi.softAPIP();
  String html = MANAGER_HTML;
  html.replace("{{AP_IP}}", apIP.toString());
  server.send(200, "text/html; charset=utf-8", html);
}

void handleScan() {
  int n = WiFi.scanNetworks();
  String out = "[";
  for (int i = 0; i < n; i++) {
    if (i) out += ",";
    out += "{\"ssid\":\"" + String(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  out += "]";
  WiFi.scanDelete();
  server.send(200, "application/json; charset=utf-8", out);
}

void handleSave() {
  // จากหน้า /manager → POST: ssid, pass
  if (!server.hasArg("ssid")) {
    server.send(400, "text/plain; charset=utf-8", "missing ssid");
    return;
  }
  String ss = server.arg("ssid");
  String pw = server.hasArg("pass") ? server.arg("pass") : "";

  prefs.putString("ssid", ss);
  prefs.putString("pass", pw);

  // ลองเชื่อมต่อ
  WiFi.mode(WIFI_STA);
  WiFi.begin(ss.c_str(), pw.c_str());
  unsigned long t0 = millis();
  bool ok = false;
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(300);
    Serial.print("#");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    stopPortal();
    IPAddress ip = WiFi.localIP();
    Serial.printf("[WiFi] Connected via /save. IP=%s\n", ip.toString().c_str());
    String j = String("{\"ok\":true,\"redirect\":\"/home\",\"ip\":\"") + ip.toString() + "\"}";
    server.send(200, "application/json; charset=utf-8", j);
  } else {
    // ค้างในพอร์ทัลต่อไป
    String j = "{\"ok\":false,\"msg\":\"connect failed\"}";
    server.send(200, "application/json; charset=utf-8", j);
  }
}

void handleStatus() {
  // สร้าง JSON ตอบสถานะระบบ/เซนเซอร์ (ไม่ใช้ ArduinoJson เพื่อลด lib)
  float t = readTempC();
  float h = readHumid();
  float f = computeFeelLikeC(t, h);

  int   ldrRaw = readLDRRaw();
  bool  night  = isNightByLDR();
  bool  rain   = isRaining();

  String ssid  = (WiFi.getMode() & WIFI_MODE_STA) ? WiFi.SSID() : "";
  String mode  = inPortal ? "AP" : ((WiFi.status()==WL_CONNECTED) ? "STA" : "IDLE");
  String ip    = inPortal ? WiFi.softAPIP().toString() :
                 (WiFi.status()==WL_CONNECTED ? WiFi.localIP().toString() : "-");
  long   rssi  = (WiFi.status()==WL_CONNECTED) ? WiFi.RSSI() : 0;
  uint32_t upS = millis()/1000;

  String out = "{";
  out += "\"tempC\":"  + String(isnan(t)?0.0:t, 2) + ",";
  out += "\"humid\":"  + String(isnan(h)?0.0:h, 2) + ",";
  out += "\"feelC\":"  + String(isnan(f)?0.0:f, 2) + ",";

  out += "\"ldrPresent\":"  + String(isLDRPresent() ? "true":"false") + ",";
  out += "\"ldrRaw\":"      + String(ldrRaw) + ",";
  out += "\"isNight\":"     + String(night ? "true":"false") + ",";

  out += "\"rainPresent\":" + String(isRainPresent() ? "true":"false") + ",";
  out += "\"isRaining\":"   + String(rain ? "true":"false") + ",";

  out += "\"relay\":"       + String(relayOn ? "true":"false") + ",";
  out += "\"auto\":"        + String(autoMode ? "true":"false") + ",";

  out += "\"ssid\":\"" + ssid + "\",";
  out += "\"mode\":\"" + mode + "\",";
  out += "\"ip\":\""   + ip   + "\",";
  out += "\"rssi\":"   + String(rssi) + ",";
  out += "\"uptime\":" + String(upS) + ",";
  out += "\"heap\":"   + String(ESP.getFreeHeap());
  out += "}";
  server.send(200, "application/json; charset=utf-8", out);
}

void handleAct() {
  // POST: auto=on|off  /  relay=on|off
  if (server.hasArg("auto")) {
    String v = server.arg("auto");
    autoMode = (v == "on" || v == "true" || v == "1");
    // ถ้าเพิ่งเปิด auto → ให้รีเลย์ตาม LDR ทันที
    if (autoMode) {
      bool night = isNightByLDR();
      applyRelay(night);
    }
  }
  if (server.hasArg("relay")) {
    String v = server.arg("relay");
    bool wantOn = (v == "on" || v == "true" || v == "1");
    autoMode = false;               // เข้า manual เมื่อสั่งรีเลย์ตรง ๆ
    applyRelay(wantOn);
  }
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
}

// =================================
//          Relay Control
// =================================
void applyRelay(bool on) {
  relayOn = on;
  setRelay(on);  // จาก sensors.h (จัดการ active high/low ให้แล้ว)
}

// =================================
/*     Upload → PHP (GET)
 *  ส่งพารามิเตอร์:
 *    rainfall = 1/0   (จาก isRaining)
 *    ldr      = ค่า ldrRaw (หรือ -1 ถ้าไม่มี)
 *    temp     = องศาเซลเซียส (ทศนิยม 2)
 *    humi     = %RH (ทศนิยม 2)
 *    feel     = heat-index C (ทศนิยม 2)
 */
void periodicUploadToPHP() {
  if (PHP_ENDPOINT.length() == 0) return;
  if (WiFi.status() != WL_CONNECTED) return;

  float t = readTempC();
  float h = readHumid();
  float f = computeFeelLikeC(t, h);
  int   l = readLDRRaw();
  bool  r = isRaining();

  // แปลงค่าเป็นสตริง (ถ้า NaN ใช้ค่าว่าง)
  auto fmt2 = [](float x)->String {
    if (isnan(x)) return String("");
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", x);
    return String(buf);
  };

  String qs = "?rainfall=" + String(r ? 1 : 0);
  qs += "&ldr="  + String(l);
  qs += "&temp=" + fmt2(t);
  qs += "&humi=" + fmt2(h);
  qs += "&feel=" + fmt2(f);

  String url = PHP_ENDPOINT + qs;

  HTTPClient http;
  http.begin(url);
  int code = http.GET();
  String resp = http.getString();
  http.end();

  Serial.printf("[PHP] GET %s → %d\n", url.c_str(), code);
  if (resp.length()) {
    Serial.printf("[PHP] resp: %s\n", resp.c_str());
  }
}
