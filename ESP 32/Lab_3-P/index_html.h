#pragma once
const char INDEX_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="th">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Lab 3 – ESP32 Dashboard</title>
<style>
  :root{font-family:ui-sans-serif,system-ui,-apple-system,Segoe UI,Roboto,TH Sarabun New,sans-serif}
  body{margin:0;background:#0b1220;color:#e6edf3}
  header{padding:14px 16px;background:#111a2b;border-bottom:1px solid #21304a}
  main{max-width:980px;margin:18px auto;padding:0 14px}
  .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}
  .card{background:#0f1a2f;border:1px solid #21304a;border-radius:14px;padding:14px}
  .label{opacity:.8;font-size:12px}
  .value{font-size:34px;font-weight:800;margin-top:6px}
  .pill{display:inline-block;border:1px solid #2a3a5e;padding:4px 10px;border-radius:999px;margin-right:6px}
  .row{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
  .ok{color:#7ee787}.warn{color:#ffb86b}.bad{color:#ff7b72}
  .muted{opacity:.6}
  .ctl{margin-top:12px}
  .btn{font-size:14px;padding:8px 10px;border-radius:10px;border:1px solid #2b3d66;background:#1e2b48;color:#fff;cursor:pointer}
  .btn:disabled{opacity:.5;cursor:not-allowed}
</style>
</head>
<body>
<header>
  <div style="display:flex;align-items:center;justify-content:space-between;">
    <strong>Lab 3 – DHT11 / LDR / Rain / Relay</strong>
    <a class="btn" href="/manager">ตั้งค่า Wi-Fi</a>
  </div>
</header>
<main>
  <div class="grid">
    <div class="card">
      <div class="label">Temperature (°C)</div>
      <div id="temp" class="value">--</div>
      <div class="row"><span class="label">Feel like</span><span id="feel" class="pill">--</span></div>
    </div>
    <div class="card">
      <div class="label">Humidity (%)</div>
      <div id="humid" class="value">--</div>
    </div>
    <div class="card">
      <div class="label">Rain</div>
      <div id="rain" class="value">--</div>
      <div id="rainHint" class="muted"></div>
    </div>
    <div class="card">
      <div class="label">LDR (Day/Night)</div>
      <div id="dn" class="value">--</div>
      <div class="row"><span class="label">LDR Raw</span><span id="ldr" class="pill">--</span></div>
      <div class="row"><span class="label">Relay</span><span id="relay" class="pill">--</span></div>
      <div id="ldrHint" class="muted"></div>
    </div>
  </div>

  <div class="card ctl">
    <div class="row" style="gap:12px;">
      <span>Auto Mode: <b id="autoLabel">ON</b></span>
      <button id="autoToggle" class="btn">สลับ Auto/Manual</button>
      <button id="relayOn" class="btn">Relay ON</button>
      <button id="relayOff" class="btn">Relay OFF</button>
    </div>
    <div class="label" style="margin-top:6px;">*Auto จะเปิดรีเลย์เมื่อ “กลางคืน” ตามค่า LDR (ถ้ามีการต่อเซนเซอร์)</div>
  </div>

  <div class="card ctl">
    <div class="row">
      <span id="ssid" class="pill">SSID: -</span>
      <span id="mode" class="pill">Mode: -</span>
      <span id="ip"   class="pill">IP: -</span>
      <span id="rssi" class="pill">RSSI: -- dBm</span>
      <span id="uptime" class="pill">Uptime: --s</span>
      <span id="heap" class="pill">Heap: --</span>
    </div>
  </div>
</main>

<script>
async function callAct(body){
  const form=new URLSearchParams(); for(const k in body) form.append(k,body[k]);
  const r=await fetch('/act',{method:'POST',body:form}); return r.json();
}
function setManualEnabled(en){
  document.getElementById('relayOn').disabled=!en;
  document.getElementById('relayOff').disabled=!en;
}
const asBool=v=> (typeof v==='boolean') ? v : (String(v).toLowerCase()==='true');
const n=x=>Number.isFinite(Number(x))?Number(x).toFixed(2):'0.00';

async function refresh(){
  try{
    const r=await fetch('/status.json',{cache:'no-store'});
    const j=await r.json();

    document.getElementById('temp').textContent  = n(j.tempC);
    document.getElementById('humid').textContent = n(j.humid);
    document.getElementById('feel').textContent  = n(j.feelC);

    // Rain
    const rainPresent = asBool(j.rainPresent);
    const rainEl = document.getElementById('rain');
    const rainHint = document.getElementById('rainHint');
    if (!rainPresent){
      rainEl.textContent = 'ไม่ทราบค่า';
      rainEl.className   = 'value';
      rainHint.textContent = 'ยังไม่ได้ต่อเซนเซอร์ฝน (Rain sensor)';
    } else {
      rainHint.textContent = '';
      const raining = asBool(j.isRaining);
      rainEl.textContent = raining ? 'ฝนกำลังตก' : 'ฝนไม่ตก';
      rainEl.className   = 'value ' + (raining ? 'bad' : 'ok');
    }

    // LDR
    const ldrPresent = asBool(j.ldrPresent);
    const dnEl = document.getElementById('dn');
    const ldrRaw = document.getElementById('ldr');
    const ldrHint = document.getElementById('ldrHint');
    if (!ldrPresent || Number(j.ldrRaw)===-1){
      dnEl.textContent = 'ไม่ทราบค่า';
      dnEl.className   = 'value';
      ldrRaw.textContent = '-';
      ldrHint.textContent = 'ยังไม่ได้ต่อ LDR (Analog/DO)';
    } else {
      ldrHint.textContent = '';
      const night = asBool(j.isNight);
      dnEl.textContent = night ? 'กลางคืน' : 'กลางวัน';
      dnEl.className   = 'value ' + (night ? 'warn' : 'ok');
      ldrRaw.textContent = j.ldrRaw;
    }

    // Wi-Fi info
    document.getElementById('ssid').textContent = `SSID: ${j.ssid ?? '-'}`;
    document.getElementById('mode').textContent = `Mode: ${j.mode ?? '-'}`;
    document.getElementById('ip').textContent   = `IP: ${j.ip ?? '-'}`;

    // Relay & ระบบ
    document.getElementById('relay').textContent     = asBool(j.relay) ? 'ON' : 'OFF';
    document.getElementById('autoLabel').textContent = asBool(j.auto)  ? 'ON' : 'OFF';
    setManualEnabled(!asBool(j.auto));

    document.getElementById('rssi').textContent   = `RSSI: ${j.rssi} dBm`;
    document.getElementById('uptime').textContent = `Uptime: ${j.uptime}s`;
    document.getElementById('heap').textContent   = `Heap: ${j.heap}`;
  }catch(e){
    console.log(e);
    document.getElementById('temp').textContent='0.00';
  }
}
document.getElementById('autoToggle').onclick=async()=>{
  const on=document.getElementById('autoLabel').textContent.trim()==='ON';
  await callAct({auto:on?'off':'on'}); refresh();
};
document.getElementById('relayOn').onclick =async()=>{await callAct({relay:'on'});  refresh();};
document.getElementById('relayOff').onclick=async()=>{await callAct({relay:'off'}); refresh();};

refresh();
setInterval(refresh,5000);
</script>
</body>
</html>)HTML";
