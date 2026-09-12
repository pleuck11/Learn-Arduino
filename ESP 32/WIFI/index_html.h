#pragma once
const char index_html[] PROGMEM = R"rawliteral(
<!doctype html><html lang="th">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Wi-Fi Setup</title>
<style>
 :root{--bg:#f8fafc;--card:#fff;--border:#e5e7eb;--shadow:0 2px 10px rgba(2,6,23,.06);--primary:#2563eb}
 body{margin:0;background:var(--bg);font-family:system-ui,Segoe UI,Arial,sans-serif;color:#0f172a}
 .container{max-width:460px;margin:24px auto;padding:0 16px}
 h1{text-align:center;margin:0 0 18px}
 .card{background:var(--card);border:1px solid var(--border);border-radius:16px;box-shadow:var(--shadow);padding:18px;margin-bottom:20px}
 .label{display:block;width:90%;margin:8px auto 6px;color:#64748b}
 .input{display:block;width:90%;margin:0 auto 10px;padding:10px;border:1px solid var(--border);border-radius:10px}
 .btn{display:block;width:90%;margin:8px auto;padding:12px 14px;border-radius:12px;background:var(--primary);color:#fff;text-decoration:none;text-align:center;font-weight:500}
 .small{color:#64748b;text-align:center;margin-top:10px}
</style>
</head>
<body>
  <div class="container">
    <h1>ตั้งค่า Wi-Fi (Portal)</h1>
    <div class="card">
      <form method="POST" action="/save">
        <label class="label">SSID</label>
        <input class="input" name="ssid" placeholder="เช่น MyHomeWiFi" />
        <label class="label">รหัสผ่าน (ถ้าเป็น Open ให้เว้นว่าง)</label>
        <input class="input" name="pass" type="password" placeholder="Wi-Fi password" />
        <button class="btn" type="submit">บันทึก & เชื่อมต่อ</button>
      </form>
    </div>
    <a class="btn" href="/scan">🔍 ค้นหา Wi-Fi รอบ ๆ</a>
    <a class="btn" href="/status">📶 ดูสถานะ</a>
    <a class="btn" href="/manager">🧭 Manager</a>
    <a class="btn" href="/reconfig">🔄 เปลี่ยนเครือข่าย (AP+STA)</a>
  </div>
</body>
</html>
)rawliteral";
