#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

#include "index_html.h"
#include "manager_html.h"

// ===== Global objects / constants =====
Preferences prefs;
WebServer server(80);
DNSServer dnsServer;

const char* AP_SSID     = "ESP32-Setup";
const char* AP_PASSWORD = "12345678";
const byte  DNS_PORT    = 53;
bool shouldStartPortal  = false;

// ===== forward declarations =====
void handleRoot();
void handleManager();
void handleStatus();
void handleStatusJson();
void handleSave();
void handleReset();
void handleScan();
void handleSet();
void handleReconfig();
void startReconfigPortal();
void startPortal();
bool tryConnectFromNVS();

// ===== Helpers =====
String baseHead(const String& title) {
  String h;
  h  = "<!doctype html><html lang='th'><head><meta charset='utf-8'>";
  h += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  h += "<title>" + title + "</title>";
  h += R"STYLE(
<style>
:root{
  --bg:#f8fafc; --card:#ffffff; --text:#0f172a; --muted:#64748b;
  --border:#e5e7eb; --shadow:0 2px 10px rgba(2,6,23,.06);
  --primary:#2563eb; --success:#10B981; --danger:#ef4444;
}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font-family:system-ui,Segoe UI,Arial,sans-serif}
.container{max-width:460px;margin:24px auto;padding:0 16px}
.container.wide{max-width:760px}          /* ⬅ ใช้เฉพาะหน้าที่ต้องการความกว้าง */
h1{text-align:center;margin:0 0 18px}
.card{
  background:var(--card);border:1px solid var(--border);border-radius:16px;
  box-shadow:var(--shadow);padding:18px;margin-bottom:20px
}
.card.scan{max-width:720px;margin-left:auto;margin-right:auto}  /* ⬅ การ์ดหน้า /scan ให้กว้างขึ้น */

/* ปุ่ม */
.btn{display:block;width:90%;margin:8px auto;padding:12px 14px;border-radius:12px;background:var(--primary);color:#fff;text-decoration:none;text-align:center;font-weight:500}
.btn.success{background:var(--success)} .btn.danger{background:var(--danger)}
.btn.sm{display:inline-block;width:auto;margin:0;padding:8px 12px;border-radius:10px}

/* ฟอร์ม */
.label{display:block;width:90%;margin:8px auto 6px;color:var(--muted)}
.input{display:block;width:90%;margin:0 auto 10px;padding:10px;border:1px solid var(--border);border-radius:10px}

/* ข้อความย่อย */
.small{color:var(--muted);text-align:center;margin-top:10px}

/* ตารางหน้า /scan – ขนาดพอดีการ์ด + ร่องซ้ายขวา */
.tablewrap{padding:0 8px;}                 /* ร่องซ้าย/ขวาภายในการ์ด */
.table{
  width:100%;                              /* ใช้เต็มพื้นที่ .tablewrap */
  border-collapse:separate;
  border-spacing:0 10px;                   /* ช่องว่างระหว่างแถว */
}
th,td{
  padding:12px 14px;
  text-align:left;
  vertical-align:middle;
}
tr.item{
  background:#fff;
  border:1px solid var(--border);
  border-radius:12px;
  box-shadow:var(--shadow);
  overflow:hidden;                         /* กันล้นมุมโค้ง */
}
td.action{
  width:1%;
  white-space:nowrap;
  padding-right:14px;                      /* กันปุ่มชนขอบ */
}

/* สัญญาณแท่ง */
.sig{display:inline-block;width:22px;height:16px;position:relative;vertical-align:middle;margin-right:8px}
.sig i{position:absolute;bottom:0;width:3px;border-radius:2px;background:#9CA3AF}
.sig i.b1{left:1px;height:4px}.sig i.b2{left:6px;height:7px}.sig i.b3{left:11px;height:10px}.sig i.b4{left:16px;height:13px}
.sig.lv1 i.b1,.sig.lv1 i.b2{background:#6B7280}.sig.lv1 i.b3,.sig.lv1 i.b4{background:#E5E7EB}
.sig.lv2 i.b1,.sig.lv2 i.b2,.sig.lv2 i.b3{background:#6B7280}.sig.lv2 i.b4{background:#E5E7EB}
.sig.lv3 i{background:#4B5563}.sig.lv3 i.b4{background:#D1D5DB}
.sig.lv4 i{background:#111827}
.lock{font-size:14px;margin-left:6px;color:#6B7280}

/* สปินเนอร์ */
.spin{width:18px;height:18px;border:3px solid #e5e7eb;border-top-color:var(--primary);border-radius:50%;display:inline-block;animation:sp 1s linear infinite;vertical-align:middle;margin-right:8px}
@keyframes sp{to{transform:rotate(360deg)}}
</style>
)STYLE";
  h += "</head><body><div class='container'>";
  return h;
}
String baseFoot(){ return "</div></body></html>"; }

void readSavedCreds(String &ssid, String &pass) {
  Preferences p; p.begin("wifi", true);
  ssid = p.getString("ssid", ""); pass = p.getString("pass", "");
  p.end();
}
String maskPass(const String &pass) {
  if (pass.length()==0) return "(empty)";
  String m; for (size_t i=0;i<pass.length(); ++i) m += "•"; return m;
}
String urlEncode(const String &s) {
  const char *hex = "0123456789ABCDEF";
  String out; out.reserve(s.length()*3);
  for (size_t i=0;i<s.length(); ++i) {
    unsigned char c = (unsigned char)s[i];
    bool safe = (c=='-'||c=='_'||c=='.'||c=='~'||
                 (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9'));
    if (safe) out += (char)c;
    else { out += '%'; out += hex[(c>>4)&0xF]; out += hex[c&0xF]; }
  }
  return out;
}

// ===== Handlers =====
void handleRoot()     { server.send(200, "text/html", String(index_html)); }
void handleManager()  { server.send(200, "text/html", String(manager_html)); }

void handleStatus() {
  String s = baseHead("สถานะ");
  if (WiFi.isConnected()) {
    s += "<div class='card'><p>เชื่อมต่อแล้วกับ <b>" + WiFi.SSID() + "</b><br>";
    s += "IP: " + WiFi.localIP().toString() + "</p></div>";
  } else {
    s += "<div class='card'><p>ยังไม่เชื่อมต่อ</p></div>";
  }
  s += "<a class='btn' href='/manager'>กลับ</a>";
  s += baseFoot();
  server.send(200,"text/html",s);
}

void handleStatusJson() {
  String json = "{\"connected\":" + String(WiFi.isConnected() ? "true" : "false");
  if (WiFi.isConnected()) {
    json += ",\"ssid\":\"" + WiFi.SSID() + "\"";
    json += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
  }
  json += "}";
  server.send(200,"application/json",json);
}

void handleSave() {
  String ssid=server.arg("ssid");
  String pass=server.arg("pass");
  prefs.begin("wifi",false);
  prefs.putString("ssid",ssid);
  prefs.putString("pass",pass);
  prefs.end();

  String s=baseHead("บันทึกแล้ว");
  s+="<div class='card'><p>บันทึกค่า SSID/Password เรียบร้อย กำลังเชื่อมต่อ...</p></div>";
  s+="<a class='btn' href='/manager'>กลับหน้า Manager</a>";
  s+=baseFoot();
  server.send(200,"text/html",s);

  delay(1000);
  WiFi.begin(ssid.c_str(),pass.c_str());
}

void handleReset() {
  prefs.begin("wifi",false);
  prefs.clear();
  prefs.end();
  String s=baseHead("ล้างค่า");
  s+="<div class='card'><p>ล้างค่า Wi-Fi แล้ว</p></div>";
  s+="<a class='btn' href='/manager'>กลับ</a>";
  s+=baseFoot();
  server.send(200,"text/html",s);
  delay(1000);
  ESP.restart();
}

void handleScan() {
  int n = WiFi.scanNetworks(false,true);
  int *idx=nullptr; if(n>0){ idx=new int[n]; for(int i=0;i<n;++i) idx[i]=i; }
  for(int i=0;i<n-1;++i) for(int j=i+1;j<n;++j)
    if(WiFi.RSSI(idx[j]) > WiFi.RSSI(idx[i])){ int t=idx[i]; idx[i]=idx[j]; idx[j]=t; }

  String h = baseHead("ค้นหา Wi-Fi");
  // ทำให้ container ของหน้านี้กว้างขึ้น
  h.replace("class='container'","class='container wide'");
  h += "<h1>ค้นหา Wi-Fi รอบ ๆ</h1><div class='card scan'>";
  if(n<=0){
    h += "<p>ไม่พบเครือข่าย</p></div><a class='btn' href='/scan'>สแกนอีกครั้ง</a>";
    h += baseFoot(); server.send(200,"text/html",h); WiFi.scanDelete(); return;
  }

  // ตารางผลลัพธ์
  h += "<div class='tablewrap'><table class='table'><thead><tr>"
       "<th>SSID</th><th>สัญญาณ</th><th>ช่อง</th><th>ความปลอดภัย</th><th></th>"
       "</tr></thead><tbody>";

  for(int k=0;k<n;++k){
    int i=idx[k];
    String ssid=WiFi.SSID(i);
    int rssi=WiFi.RSSI(i);
    int ch=WiFi.channel(i);
    bool enc=(WiFi.encryptionType(i)!=WIFI_AUTH_OPEN);
    int lvl=1; if(rssi>=-50)lvl=4; else if(rssi>=-60)lvl=3; else if(rssi>=-70)lvl=2;

    h+="<tr class='item'><td>"+(ssid.length()?ssid:"(hidden)")+(enc?"<span class='lock'>🔒</span>":"<span class='lock'>🔓</span>")+"</td>";
    h+="<td><div class='sig lv"+String(lvl)+"'><i class='b1'></i><i class='b2'></i><i class='b3'></i><i class='b4'></i></div> "+String(rssi)+" dBm</td>";
    h+="<td>"+String(ch)+"</td>";
    h+="<td>"+String(enc?"WPA/WPA2/3":"Open")+"</td>";
    h+="<td class='action'><a class='btn sm' href='/set?ssid="+urlEncode(ssid)+"'>เลือก</a></td></tr>";
  }

  // ปิด table + tablewrap + card
  h += "</tbody></table></div></div>";
  h += "<a class='btn' href='/scan'>สแกนอีกครั้ง</a>";
  h += "<a class='btn' href='/'>กลับหน้าตั้งค่า</a>";
  h += baseFoot();
  server.send(200,"text/html",h);

  WiFi.scanDelete(); if(idx) delete[] idx;
}

void handleSet() {
  String ssid=server.arg("ssid");
  String s=baseHead("ตั้งค่า Wi-Fi");
  s+="<h1>กรอกรหัสผ่าน</h1><div class='card'><form method='POST' action='/save'>";
  s+="<label class='label'>SSID</label><input class='input' name='ssid' value='"+ssid+"' readonly>";
  s+="<label class='label'>รหัสผ่าน</label><input class='input' name='pass' type='password' placeholder='Wi-Fi password'>";
  s+="<button class='btn' type='submit'>บันทึก & เชื่อมต่อ</button></form></div>";
  s+="<a class='btn' href='/scan'>กลับไปเลือกเครือข่าย</a>";
  s+="<a class='btn' href='/'>กลับหน้าตั้งค่า</a>";
  s+=baseFoot();
  server.send(200,"text/html",s);
}

void startReconfigPortal() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  shouldStartPortal = true;
}
void handleReconfig() {
  if (!(WiFi.getMode() & WIFI_MODE_AP)) startReconfigPortal();
  String ap = WiFi.softAPIP().toString();
  String s = baseHead("โหมดเปลี่ยนเครือข่าย");
  s += "<div class='card'><h1>โหมดตั้งค่าพร้อมแล้ว</h1>";
  s += "<p>เชื่อมต่อ SSID <b>"+String(AP_SSID)+"</b> แล้วเปิด <b>http://"+ap+"/</b></p></div>";
  s += "<a class='btn' href='/scan'>สแกนจากที่นี่</a>";
  s += "<a class='btn' href='/'>กลับหน้าตั้งค่า</a>";
  s += baseFoot();
  server.send(200,"text/html",s);
}

// ===== Bootstraps =====
void startPortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/",            HTTP_GET,  handleRoot);
  server.on("/manager",     HTTP_GET,  handleManager);
  server.on("/status",      HTTP_GET,  handleStatus);
  server.on("/status.json", HTTP_GET,  handleStatusJson);
  server.on("/save",        HTTP_POST, handleSave);
  server.on("/reset",       HTTP_GET,  handleReset);
  server.on("/scan",        HTTP_GET,  handleScan);
  server.on("/set",         HTTP_GET,  handleSet);
  server.on("/reconfig",    HTTP_GET,  handleReconfig);
  server.begin();

  shouldStartPortal = true;
  Serial.println(String("[Portal] http://") + WiFi.softAPIP().toString() + "/");
}

bool tryConnectFromNVS() {
  String ssid, pass; readSavedCreds(ssid, pass);
  if (ssid=="") return false;
  WiFi.mode(WIFI_STA); WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long t0=millis();
  while (WiFi.status()!=WL_CONNECTED && millis()-t0<20000) delay(250);
  return WiFi.status()==WL_CONNECTED;
}

void setup() {
  Serial.begin(115200);
  WiFi.setHostname("esp32-setup");

  if (!tryConnectFromNVS()) startPortal();
  else {
    server.on("/",            HTTP_GET,  handleManager);
    server.on("/manager",     HTTP_GET,  handleManager);
    server.on("/status",      HTTP_GET,  handleStatus);
    server.on("/status.json", HTTP_GET,  handleStatusJson);
    server.on("/scan",        HTTP_GET,  handleScan);
    server.on("/set",         HTTP_GET,  handleSet);
    server.on("/save",        HTTP_POST, handleSave);
    server.on("/reset",       HTTP_GET,  handleReset);
    server.on("/reconfig",    HTTP_GET,  handleReconfig);
    server.begin();
    Serial.print("[WiFi] Connected IP: "); Serial.println(WiFi.localIP());
  }
}

void loop() {
  if (shouldStartPortal) dnsServer.processNextRequest();
  server.handleClient();
}
