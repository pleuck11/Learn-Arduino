#pragma once
const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="th">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 – Home Dashboard</title>
<style>
  :root { font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, sans-serif; }
  body { margin:0; background:#0b1220; color:#e6edf3; }
  .wrap { max-width:980px; margin:24px auto; padding:0 16px; }
  .title { font-size:22px; font-weight:800; margin-bottom:12px; display:flex; align-items:center; justify-content:space-between; }
  .btn { background:#1e2b48; color:#fff; border:1px solid #2b3d66; border-radius:10px; padding:8px 12px; text-decoration:none; }
  .grid { display:grid; grid-template-columns:repeat(auto-fit,minmax(220px,1fr)); gap:12px; }
  .card { background:#121a2b; border:1px solid #1f2a44; border-radius:14px; padding:14px; text-align:center; }
  .label { opacity:.8; font-size:12px; }
  .value { font-size:36px; font-weight:800; margin-top:6px; }
  .row { display:flex; gap:8px; align-items:center; flex-wrap:wrap; justify-content:center; }
  .pill { font-size:12px; border:1px solid #2a3a5e; padding:4px 8px; border-radius:999px; }
  .footer { opacity:.7; font-size:12px; margin-top:16px; text-align:center; }
</style>
</head>
<body>
  <div class="wrap">
    <div class="title">
      ESP32 – Sensors Dashboard
      <a class="btn" href="/manager">ตั้งค่า Wi-Fi</a>
    </div>

    <div class="grid">
      <div class="card">
        <div class="label">Temperature (°C)</div>
        <div id="temp" class="value">--</div>
      </div>

      <div class="card">
        <div class="label">LDR (0–4095)</div>
        <div id="ldr" class="value">--</div>
      </div>

      <div class="card">
        <div class="label">Day/Night</div>
        <div id="daynight" class="value">--</div>
      </div>

      <div class="card">
        <div class="label">Wi-Fi</div>
        <div class="row">
          <div id="wifi" class="pill">RSSI: -- dBm</div>
          <div id="ip" class="pill">IP: --</div>
          <div id="uptime" class="pill">Uptime: --s</div>
          <div id="heap" class="pill">Heap: --</div>
        </div>
      </div>
    </div>

    <div class="footer">อัปเดตทุก 2 วินาที | ถ้าไม่ขึ้นตรวจสอบสาย/พิน/ไลบรารี</div>
  </div>

<script>
async function refresh(){
  try{
    const r = await fetch('/status.json',{cache:'no-store'});
    const j = await r.json();

    document.getElementById('temp').textContent     = j.tempC?.toFixed?.(2) ?? j.tempC ?? '--';
    document.getElementById('ldr').textContent      = j.ldr ?? '--';
    document.getElementById('daynight').textContent = j.dayNight ?? '--';

    document.getElementById('wifi').textContent   = `RSSI: ${j.rssi} dBm`;
    document.getElementById('uptime').textContent = `Uptime: ${j.uptime}s`;
    document.getElementById('heap').textContent   = `Heap: ${j.heap}`;
    if(location.hostname) document.getElementById('ip').textContent = `IP: ${location.hostname}`;
  }catch(e){ console.log(e); }
}
refresh();
setInterval(refresh,2000);
</script>
</body>
</html>)HTML";
