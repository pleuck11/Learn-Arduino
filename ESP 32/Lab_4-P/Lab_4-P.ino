// ==== Slim Firebase + RTDB only ====
#define ENABLE_RTDB

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

#include <Firebase_ESP_Client.h>   // by Mobizt
// ไม่ต้องใช้ ArduinoJson.h

#include "index_html.h"
#include "manager_html.h"
#include "sensors.h"
#include "firebase_secrets.h"

// ====== Globals ======
Preferences prefs;
WebServer  server(80);
DNSServer  dnsServer;

const char* AP_SSID     = "ESP32-Setup";
const char* AP_PASSWORD = "12345678";
const byte  DNS_PORT    = 53;

bool autoMode = true;   // Auto = เปิดไฟตาม LDR
bool relayOn  = false;  // สถานะรีเลย์

// ====== Firebase Objects ======
FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig config;

unsigned long lastFbPushMs = 0;
const unsigned long FB_PUSH_INTERVAL = 10000; // push RTDB ทุก 10 วิ
unsigned long lastDesiredPollMs = 0;
const unsigned long FB_POLL_INTERVAL = 1000; // โพล desired ทุก 1 วิ

// ====== Fwd decls ======
bool tryConnectFromNVS();
void startPortal();
void stopPortal();
bool captivePortalRedirect();
void applyRelay() { setRelay(relayOn); }

String currentIP()   { return WiFi.isConnected() ? WiFi.localIP().toString()  : WiFi.softAPIP().toString(); }
String currentSSID() { return WiFi.isConnected() ? WiFi.SSID()                : String(AP_SSID); }
String currentMode() { return WiFi.isConnected() ? String("STA")              : String("AP"); }

String jsonStatus() {
  float t = readTempC();
  float h = readHumid();
  float f = 0.0f;
  if (isnan(t) || isnan(h)) { t = 0.0f; h = 0.0f; f = 0.0f; }
  else                      { f = computeFeelLikeC(t, h); }

  bool rainOk = isRainPresent();
  bool ldrOk  = isLDRPresent();
  int  ldr    = ldrOk ? readLDRRaw() : -1;
  bool night  = ldrOk ? isNightByLDR() : false;
  bool rain   = rainOk ? isRaining()   : false;

  char buf[560];
  snprintf(buf, sizeof(buf),
    "{\"tempC\":%.2f,\"humid\":%.2f,\"feelC\":%.2f,"
    "\"ldrRaw\":%d,\"isNight\":%s,\"isRaining\":%s,"
    "\"ldrPresent\":%s,\"rainPresent\":%s,"
    "\"auto\":%s,\"relay\":%s,"
    "\"rssi\":%d,\"uptime\":%lu,\"heap\":%u,"
    "\"ip\":\"%s\",\"ssid\":\"%s\",\"mode\":\"%s\"}",
    t, h, f,
    ldr,
    night?"true":"false",
    rain ?"true":"false",
    ldrOk ?"true":"false",
    rainOk?"true":"false",
    autoMode?"true":"false",
    relayOn ?"true":"false",
    (WiFi.isConnected()?WiFi.RSSI():0),
    (unsigned long)(millis()/1000),
    ESP.getFreeHeap(),
    currentIP().c_str(),
    currentSSID().c_str(),
    currentMode().c_str()
  );
  return String(buf);
}

// ====== Firebase Helpers ======
void firebaseBegin() {
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;

  if (String(USER_EMAIL).length() > 0 && String(USER_PASSWORD).length() > 0) {
    auth.user.email    = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
  } else {
    // Anonymous sign-up
    Serial.println("[FB ] Try anonymous sign-up");
    if (Firebase.signUp(&config, &auth, "", "")) {
      Serial.println("[FB ] Anonymous sign-up OK");
    } else {
      Serial.printf("[FB ] Anonymous sign-up failed: %s\n", config.signer.signupError.message.c_str());
    }
  }

  Firebase.reconnectWiFi(true);
  config.timeout.serverResponse = 10000; // 10s
  Firebase.begin(&config, &auth);
}

bool fbSetJson(const String& path, FirebaseJson& json) {
  if (!Firebase.ready()) return false;
  if (!Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
    Serial.printf("[FB ] setJSON fail [%s] %s\n", path.c_str(), fbdo.errorReason().c_str());
    return false;
  }
  return true;
}

bool fbGetBool(const String& path, bool &outVal) {
  if (!Firebase.ready()) return false;
  if (Firebase.RTDB.getBool(&fbdo, path.c_str())) {
    outVal = fbdo.boolData();
    return true;
  }
  return false;
}

void fbPushState() {
  float t = readTempC();
  float h = readHumid();
  float f = (isnan(t)||isnan(h)) ? 0.0f : computeFeelLikeC(t,h);

  bool rainOk = isRainPresent();
  bool ldrOk  = isLDRPresent();
  int  ldr    = ldrOk ? readLDRRaw() : -1;
  bool night  = ldrOk ? isNightByLDR() : false;
  bool rain   = rainOk ? isRaining()   : false;

  FirebaseJson js;
  js.set("tempC", isnan(t)?0.0:t);
  js.set("humid", isnan(h)?0.0:h);
  js.set("feelC", f);
  js.set("ldrRaw", ldr);
  js.set("isNight", night);
  js.set("isRaining", rain);
  js.set("ldrPresent", ldrOk);
  js.set("rainPresent", rainOk);
  js.set("auto", autoMode);
  js.set("relay", relayOn);
  js.set("wifi/ssid", currentSSID());
  js.set("wifi/ip", currentIP());
  js.set("wifi/rssi", WiFi.isConnected()?WiFi.RSSI():0);
  js.set("mode", currentMode());
  js.set("uptime", (uint32_t)(millis()/1000));
  js.set("heap", (uint32_t)ESP.getFreeHeap());

  String path = "/devices/" + String(DEVICE_ID) + "/state";
  fbSetJson(path, js);
}

void fbPollDesiredAndApply() {
  String base = "/devices/" + String(DEVICE_ID) + "/desired";
  bool v;

  if (fbGetBool(base + "/auto", v)) {
    autoMode = v;
    if (autoMode) {
      relayOn = isLDRPresent() ? isNightByLDR() : false;
      applyRelay();
    }
  }
  if (fbGetBool(base + "/relay", v)) {
    if (!autoMode) {
      relayOn = v;
      applyRelay();
    }
  }
}

// ====== Arduino ======
void setup() {
  Serial.begin(115200);
  delay(200);

  initSensors();
  applyRelay();

  if (!tryConnectFromNVS()) {
    startPortal();
    Serial.printf("[AP ] SSID:%s PASS:%s IP:%s\n", AP_SSID, AP_PASSWORD, WiFi.softAPIP().toString().c_str());
  } else {
    Serial.printf("[WiFi] Connected, ESP32 IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("[WiFi] SSID: %s\n", WiFi.SSID().c_str());
    firebaseBegin();   // เริ่ม Firebase เมื่อ Wi-Fi พร้อม
  }

  // --- Routes ---
  server.on("/", HTTP_GET, [](){
    if (WiFi.status()==WL_CONNECTED)
      { server.sendHeader("Location","/home"); server.send(302,"text/plain",""); }
    else
      { server.sendHeader("Location","/manager"); server.send(302,"text/plain",""); }
  });

  server.on("/home", HTTP_GET, [](){
    if (captivePortalRedirect()) return;
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
  });

  server.on("/status.json", HTTP_GET, [](){
    if (captivePortalRedirect()) return;
    server.send(200, "application/json; charset=utf-8", jsonStatus());
  });

  // POST /act  (auto=on|off, relay=on|off)
  server.on("/act", HTTP_POST, [](){
    bool changed = false;

    if (server.hasArg("auto")) {
      String v = server.arg("auto");
      autoMode = (v == "on" || v == "1" || v == "true");
      changed = true;
      if (autoMode) { 
        relayOn = isLDRPresent() ? isNightByLDR() : false;
        applyRelay();
      }
    }

    if (!autoMode && server.hasArg("relay")) {
      String v = server.arg("relay");
      relayOn = (v=="on" || v=="1" || v=="true");
      changed = true;
    }

    if (changed) applyRelay();
    server.send(200, "application/json; charset=utf-8",
                "{\"ok\":true,\"status\":"+jsonStatus()+"}");
  });

  // Manager
  server.on("/manager", HTTP_GET, [](){
    IPAddress ip = (WiFi.getMode()==WIFI_MODE_AP) ? WiFi.softAPIP() : WiFi.localIP();
    String page = String(MANAGER_HTML);
    page.replace("{{AP_IP}}", ip.toString());
    server.send(200, "text/html; charset=utf-8", page);
  });

  // Scan Wi-Fi (ย่อ JSON)
  server.on("/scan", HTTP_GET, [](){
    int n = WiFi.scanNetworks(false, true);
    String out = "[";
    for (int i = 0; i < n; ++i) {
      if (i) out += ",";
      out += "{\"ssid\":\"" + String(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    out += "]";
    WiFi.scanDelete();
    server.send(200, "application/json; charset=utf-8", out);
  });

  // Save Wi-Fi
  server.on("/save", HTTP_POST, [](){
    String ssid = server.hasArg("ssid") ? server.arg("ssid") : "";
    String pass = server.hasArg("pass") ? server.arg("pass") : "";

    if (ssid == "") {
      server.send(400, "text/plain; charset=utf-8", "ssid required");
      return;
    }

    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();

    stopPortal();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
      firebaseBegin();
      String ipStr = WiFi.localIP().toString();
      String resp = String("{\"ok\":true,\"ip\":\"") + ipStr + "\",\"redirect\":\"http://" + ipStr + "/home\"}";
      server.send(200, "application/json; charset=utf-8", resp);
    } else {
      startPortal();
      IPAddress apIP = WiFi.softAPIP();
      String resp = String("{\"ok\":true,\"ip\":\"") + apIP.toString() + "\",\"redirect\":\"/manager\"}";
      server.send(200, "application/json; charset=utf-8", resp);
    }
  });

  server.begin();
  Serial.printf("[HTTP] server started at http://%s\n", currentIP().c_str());
}

void loop() {
  server.handleClient();
  if (WiFi.getMode()==WIFI_MODE_AP) dnsServer.processNextRequest();

  if (autoMode) {
      bool nextRelay = isLDRPresent() ? isNightByLDR() : false;
      relayOn = nextRelay;
      applyRelay();
  }

  static unsigned long t = 0;
  if (millis() - t > 5000) {
    refreshPresenceCheck();
    t = millis();
  }

  if (Firebase.ready() && WiFi.status() == WL_CONNECTED) {
    if (millis() - lastDesiredPollMs >= FB_POLL_INTERVAL) {
      lastDesiredPollMs = millis();
      fbPollDesiredAndApply();
    }
    if (millis() - lastFbPushMs >= FB_PUSH_INTERVAL) {
      lastFbPushMs = millis();
      fbPushState();
    }
  }
}

// ===== Wi-Fi Helpers =====
bool tryConnectFromNVS() {
  WiFi.mode(WIFI_STA);
  prefs.begin("wifi", true);
  String ssid=prefs.getString("ssid","");
  String pass=prefs.getString("pass","");
  prefs.end();
  if(ssid=="") return false;

  WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long t0=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t0<12000){ delay(500); Serial.print('.'); }
  Serial.println();
  return WiFi.status()==WL_CONNECTED;
}

void startPortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress apIP=WiFi.softAPIP();
  dnsServer.start(DNS_PORT,"*",apIP);
}

void stopPortal() {
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
}

bool captivePortalRedirect() {
  if (WiFi.getMode()==WIFI_MODE_AP) {
    IPAddress apIP=WiFi.softAPIP();
    if (server.hostHeader()!=apIP.toString()) {
      server.sendHeader("Location","http://"+apIP.toString()+"/manager",true);
      server.send(302,"text/plain","");
      return true;
    }
  }
  return false;
}
