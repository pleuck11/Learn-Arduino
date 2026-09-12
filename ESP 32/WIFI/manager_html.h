#pragma once
const char manager_html[] PROGMEM = R"rawliteral(
<!doctype html><html lang="th"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Manager</title>
<style>
 :root{--bg:#f8fafc;--card:#fff;--border:#e5e7eb;--shadow:0 2px 10px rgba(2,6,23,.06);--primary:#2563eb}
 body{margin:0;background:var(--bg);font-family:system-ui,Segoe UI,Arial,sans-serif;color:#0f172a}
 .container{max-width:460px;margin:24px auto;padding:0 16px}
 h1{text-align:center;margin:0 0 18px}
 .card{background:var(--card);border:1px solid var(--border);border-radius:16px;box-shadow:var(--shadow);padding:18px;margin-bottom:20px;text-align:center}
 .btn{display:block;width:90%;margin:8px auto;padding:12px 14px;border-radius:12px;background:var(--primary);color:#fff;text-decoration:none;text-align:center;font-weight:500}
 .small{color:#64748b;text-align:center;margin-top:10px}
</style>
</head><body>
  <div class="container">
    <h1>ESP32 Manager</h1>
    <div class="card">
      <a class="btn" href="/status">📶 ดูสถานะ</a>
      <a class="btn" href="/">⚙️ ตั้งค่า Wi-Fi</a>
      <a class="btn" href="/scan">🔍 ค้นหา Wi-Fi</a>
      <a class="btn" href="/reconfig">🔄 เปลี่ยนเครือข่าย (AP+STA)</a>
      <a class="btn" href="/reset">🧹 ล้างค่า</a>
    </div>
    <p class="small">ใช้ “เปลี่ยนเครือข่าย (AP+STA)” เมื่อบอร์ดต่อเน็ตอยู่ แต่ต้องการเปิดพอร์ทัลชั่วคราวโดยไม่หลุดจากเครือข่ายเดิม</p>
  </div>
</body></html>
)rawliteral";
