// Room_status.ino
// - Wi-Fi portal ตั้งค่า SSID/PASS (manager_wifi.h)
// - Firebase: เคลม owner, ส่ง readings ทุก 5s
// - Relay: GPIO26 (active-LOW) อ่านคำสั่งจาก /settings/<DEVICE_ID>/relayOn

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include "firebase.h"
#include "Sensor.h"
#include "manager_wifi.h"

#define AP_SSID            "RoomStatus-Setup"
#define AP_PASS            "pleuck0803"
#define CONNECT_TIMEOUT_MS 15000

WebServer   server(80);
Preferences prefs;

String saved_ssid, saved_pass;
bool   inAP = false;

// ---------- Firebase ----------
FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig fcfg;

unsigned long lastPushMs   = 0;
bool          ownerClaimed = false;
bool          tokenLogged  = false;

// ---------- Relay ----------
#define RELAY_PIN        26     // ต่อ IN รีเลย์ที่ GPIO26
#define RELAY_ACTIVE_LOW 1      // โมดูลส่วนมากเป็น active-LOW

inline void relayInit() {
  pinMode(RELAY_PIN, OUTPUT);
#if RELAY_ACTIVE_LOW
  digitalWrite(RELAY_PIN, HIGH);   // OFF
#else
  digitalWrite(RELAY_PIN, LOW);    // OFF
#endif
}
inline void relaySet(bool on) {
#if RELAY_ACTIVE_LOW
  digitalWrite(RELAY_PIN, on ? LOW : HIGH);
#else
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
#endif
}
bool relayState = false;
unsigned long lastRelayPoll = 0;

// ---------- fwd decl ----------
void initFirebase();
void pushReadingsOnce();
void claimOwnerOnce();
void startMDNSIfSTA();
void pollRelaySetting();

// ---------- HTTP helpers ----------
static String htmlWithIP(const char* rawHtml, IPAddress ip) {
  String page = FPSTR(rawHtml);
  page.replace("{{AP_IP}}", ip.toString());
  return page;
}
void handleRoot() {
  IPAddress ip = inAP ? WiFi.softAPIP() : WiFi.localIP();
  server.send(200, "text/html; charset=utf-8", htmlWithIP(MANAGER_WIFI_HTML, ip));
}
void handleScan() {
  int n = WiFi.scanNetworks();
  String out = "[";
  for (int i = 0; i < n; i++) {
    if (i) out += ",";
    out += "{\"ssid\":\"" + String(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  out += "]";
  server.send(200, "application/json; charset=utf-8", out);
}
bool connectSTA(const String& ssid, const String& pass, uint32_t timeoutMs) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) delay(200);
  return WiFi.status() == WL_CONNECTED;
}
void startAP() {
  inAP = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(120);
  Serial.println("[WiFi] AP mode started: " + String(AP_SSID));
  Serial.print("[WiFi] AP IP: "); Serial.println(WiFi.softAPIP());
}
void handleSave() {
  if (!server.hasArg("ssid")) { server.send(400, "text/plain; charset=utf-8", "ssid required"); return; }
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.end();

  WiFi.softAPdisconnect(true);
  bool ok = connectSTA(ssid, pass, CONNECT_TIMEOUT_MS);

  if (ok) {
    inAP = false;
    String ip = WiFi.localIP().toString();
    server.send(200, "application/json; charset=utf-8", String("{\"ok\":true,\"ip\":\"") + ip + "\"}");
    Serial.println("[WiFi] Connected. IP: " + ip);
    initFirebase();
    startMDNSIfSTA();
    lastPushMs = 0;
  } else {
    startAP();
    server.send(200, "application/json; charset=utf-8",
      String("{\"ok\":false,\"msg\":\"connect failed, back to AP\",\"ap\":\"") + WiFi.softAPIP().toString() + "\"}");
  }
}
void handleReset() {
  prefs.begin("wifi", false); prefs.clear(); prefs.end();
  startAP();
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true,\"cleared\":true}");
}
void setupServer() {
  server.on("/",      HTTP_GET,  handleRoot);
  server.on("/scan",  HTTP_GET,  handleScan);
  server.on("/save",  HTTP_POST, handleSave);
  server.on("/reset", HTTP_POST, handleReset);
  server.onNotFound([]{ server.send(404, "text/plain; charset=utf-8", "Not found"); });
  server.begin(); Serial.println("[HTTP] Server started");
}
void tryConnectFromNVS() {
  prefs.begin("wifi", true);
  saved_ssid = prefs.getString("ssid", "");
  saved_pass = prefs.getString("pass", "");
  prefs.end();

  if (saved_ssid.length() > 0) {
    Serial.println("[WiFi] Try connect saved SSID: " + saved_ssid);
    if (connectSTA(saved_ssid, saved_pass, CONNECT_TIMEOUT_MS)) {
      inAP = false;
      Serial.println("[WiFi] Connected. IP: " + WiFi.localIP().toString());
      initFirebase();
      startMDNSIfSTA();
      return;
    }
    Serial.println("[WiFi] Connect failed. Fallback to AP.");
  }
  startAP();
}

// ---------- Arduino ----------
void setup() {
  Serial.begin(115200);
  delay(200);

  relayInit();        // รีเลย์
  initSensors();      // DHT/LDR
  tryConnectFromNVS();
  setupServer();
  startMDNSIfSTA();
}
void loop() {
  server.handleClient();

  if (!inAP && WiFi.status() == WL_CONNECTED) {
    if (!tokenLogged && Firebase.ready() && auth.token.uid.length() > 0) {
      Serial.printf("[FB] token ready (uid: %s)\n", auth.token.uid.c_str());
      tokenLogged = true;
    }
    if (!ownerClaimed) claimOwnerOnce();

    pollRelaySetting(); // อ่านคำสั่งรีเลย์จาก /settings/<DEVICE_ID>/relayOn

    if (Firebase.ready() && millis() - lastPushMs > 5000) {
      lastPushMs = millis();
      pushReadingsOnce();
    }
  }
}

// ---------- Helpers ----------
void startMDNSIfSTA() {
  if (!inAP && WiFi.status() == WL_CONNECTED) {
    if (MDNS.begin("roomstatus")) Serial.println("[mDNS] http://roomstatus.local/");
  }
}

// ---------- Firebase ----------
void initFirebase() {
  fcfg.api_key               = FIREBASE_API_KEY;
  fcfg.database_url          = FIREBASE_DATABASE_URL;
  fcfg.token_status_callback = tokenStatusCallback;

  if (String(USER_EMAIL).length() == 0) {
    if (!Firebase.signUp(&fcfg, &auth, "", "")) {
      Serial.printf("[FB] signUp error: %s\n", fcfg.signer.signupError.message.c_str());
    } else {
      Serial.println("[FB] Anonymous sign-in OK");
    }
  } else {
    auth.user.email    = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
  }

  Firebase.begin(&fcfg, &auth);
  Firebase.reconnectNetwork(true);
  ownerClaimed = false;
  tokenLogged  = false;
  Serial.println("[FB] Firebase started");
}

void pushReadingsOnce() {
  SensorData s;
  bool dhtOK = readSensors(s);

  Serial.printf("[SENS] lightRaw=%u lightPct=%.0f%% state=%s, temp=%s, hum=%s\n",
    s.lightRaw, s.lightPct, s.lightState,
    isnan(s.temperature) ? "NaN" : String(s.temperature,1).c_str(),
    isnan(s.humidity)    ? "NaN" : String(s.humidity,1).c_str()
  );

  String base = String("/readings/") + DEVICE_ID;
  auto SET = [&](bool ok, const char* key){
    if (!ok) Serial.printf("[FB] write error @%s: %s\n", key, fbdo.errorReason().c_str());
    return ok;
  };

  SET(Firebase.RTDB.setInt   (&fbdo, base + "/lightRaw",   s.lightRaw),   "lightRaw");
  SET(Firebase.RTDB.setFloat (&fbdo, base + "/lightPct",   s.lightPct),   "lightPct");
  SET(Firebase.RTDB.setString(&fbdo, base + "/lightState", s.lightState), "lightState");

  if (dhtOK && !isnan(s.temperature)) {
    String tempText = String(s.temperature, 1) + " °C";
    SET(Firebase.RTDB.setFloat (&fbdo, base + "/temp",     s.temperature), "temp");
    SET(Firebase.RTDB.setString(&fbdo, base + "/tempText", tempText),      "tempText");
    SET(Firebase.RTDB.setString(&fbdo, base + "/tempStatus","ok"),         "tempStatus");
  } else {
    SET(Firebase.RTDB.setString(&fbdo, base + "/tempStatus","sensor_offline"), "tempStatus");
    SET(Firebase.RTDB.deleteNode(&fbdo, base + "/temp"),     "temp(del)");
    SET(Firebase.RTDB.deleteNode(&fbdo, base + "/tempText"), "tempText(del)");
    Serial.println("[DHT] read failed (NaN) – check wiring/pull-up and wait ≥2s after begin");
  }

  // (ออปชัน) โชว์สถานะรีเลย์ด้วย
  SET(Firebase.RTDB.setBool(&fbdo, base + "/relayOn", relayState), "relayOn");

  String summary = String("แสง: ") + s.lightState + " (" + String(s.lightPct,0) + "%)"
                 + (dhtOK && !isnan(s.temperature) ? String(", อุณหภูมิ: ") + String(s.temperature,1) + " °C"
                                                    : String(", อุณหภูมิ: ไม่ทราบ"));
  SET(Firebase.RTDB.setString(&fbdo, base + "/summary", summary), "summary");

  SET(Firebase.RTDB.setString(&fbdo, base + "/ip", WiFi.localIP().toString()), "ip");
  SET(Firebase.RTDB.setInt   (&fbdo, base + "/ts", millis()),                  "ts");

  Serial.println("[FB] pushed");
}

void claimOwnerOnce() {
  if (!Firebase.ready()) return;
  String uid = auth.token.uid.c_str();
  if (uid.length() == 0) return;

  String path = String("/owners/") + DEVICE_ID;
  String cur;

  if (!Firebase.RTDB.getString(&fbdo, path)) {
    Serial.printf("[FB] owners/get error: %s\n", fbdo.errorReason().c_str());
  } else {
    cur = fbdo.stringData();
    if (cur == uid) { ownerClaimed = true; return; }
  }
  if (Firebase.RTDB.setString(&fbdo, path, uid)) {
    ownerClaimed = true;
    Serial.println("[FB] owner claimed: " + uid);
  } else {
    Serial.printf("[FB] claim failed: %s\n", fbdo.errorReason().c_str());
  }
}

void pollRelaySetting() {
  if (!Firebase.ready()) return;
  if (millis() - lastRelayPoll < 2000) return;  // เช็คทุก 2s
  lastRelayPoll = millis();

  String path = String("/settings/") + DEVICE_ID + "/relayOn";
  if (Firebase.RTDB.getBool(&fbdo, path)) {
    bool desired = fbdo.boolData();
    if (desired != relayState) {
      relaySet(desired);
      relayState = desired;
      Serial.printf("[RELAY] set to %s\n", desired ? "ON" : "OFF");
      Firebase.RTDB.setBool(&fbdo, String("/readings/") + DEVICE_ID + "/relayOn", relayState);
    }
  } else {
    Serial.printf("[RELAY] read failed: %s\n", fbdo.errorReason().c_str());
  }
}
