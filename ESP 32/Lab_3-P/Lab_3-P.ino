#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

#include "index_html.h"
#include "manager_html.h"
#include "sensors.h"

Preferences prefs;
WebServer  server(80);
DNSServer  dnsServer;

const char* AP_SSID     = "ESP32-Setup";
const char* AP_PASSWORD = "12345678";
const byte  DNS_PORT    = 53;

bool autoMode = true;   // Auto = เปิดไฟตาม LDR
bool relayOn  = false;  // สถานะรีเลย์

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

void setup() {
  Serial.begin(115200);
  delay(200);

  initSensors();
  applyRelay();

  if (!tryConnectFromNVS()) {
    startPortal();
    Serial.print("[AP ] IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.print("[WiFi] Connected, ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WiFi] SSID: ");
    Serial.println(WiFi.SSID());
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

  // ===== Manual / Auto Control =====
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

  // Wi-Fi Manager
  server.on("/manager", HTTP_GET, [](){
    IPAddress ip = (WiFi.getMode()==WIFI_MODE_AP) ? WiFi.softAPIP() : WiFi.localIP();
    String page = String(MANAGER_HTML);
    page.replace("{{AP_IP}}", ip.toString());
    server.send(200, "text/html; charset=utf-8", page);
  });

  server.begin();
  Serial.println("[HTTP] server started");
}

void loop() {
  server.handleClient();
  if (WiFi.getMode()==WIFI_MODE_AP) dnsServer.processNextRequest();

  if (autoMode) {
      bool nextRelay = isLDRPresent() ? isNightByLDR() : false;
      relayOn = nextRelay;    // true=กลางคืน(เปิด), false=กลางวัน(ปิด)
      applyRelay();
  }

  static unsigned long t = 0;
  if (millis() - t > 5000) {
    refreshPresenceCheck();
    t = millis();
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
  Serial.printf("[AP ] SSID:%s PASS:%s IP:%s\n",AP_SSID,AP_PASSWORD,apIP.toString().c_str());
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
