#pragma once
const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="th">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Smart Fire Alarm - Dashboard</title>
<style>
/* ===== Dark theme palette ===== */
:root{
  --bg:#0b1220; --header:#0d1423; --card:#0f172a; --line:rgba(255,255,255,.10);
  --text:#e5e7eb; --muted:#94a3b8; --accent:#3b82f6; --accent-weak:rgba(59,130,246,.15);
  --danger:#f87171; --ok:#22c55e;
}
*{box-sizing:border-box}
body{margin:0;font-family:ui-sans-serif,system-ui,Segoe UI,Roboto,Arial;background:var(--bg);color:var(--text)}
header{padding:16px 20px;background:var(--header);border-bottom:1px solid var(--line);position:sticky;top:0;z-index:10}
h1{margin:0;font-size:20px}
.wrap{max-width:980px;margin:28px auto;padding:0 16px}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(260px,1fr));gap:36px}
.card{background:var(--card);border:1px solid var(--line);border-radius:16px;padding:16px;box-shadow:0 10px 28px rgba(0,0,0,.45);margin-block:12px}
.title{font-size:13px;color:var(--muted);margin-bottom:6px}
.value{font-size:32px;font-weight:700}
.row{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
button,.btn{appearance:none;border:0;background:var(--accent);color:#fff;padding:10px 14px;border-radius:12px;cursor:pointer;font-weight:600}
.btn-outline,.btn-ghost{background:transparent;color:var(--text);border:1px solid var(--line)}
button:hover,.btn:hover{filter:brightness(1.05)} .btn-outline:hover,.btn-ghost:hover{border-color:rgba(255,255,255,.2)}
.badge{display:inline-block;padding:4px 10px;border-radius:999px;font-size:12px;background:var(--accent-weak);color:#93c5fd;border:1px solid rgba(59,130,246,.2)}
.mono{font-family:ui-monospace,Menlo,Consolas,monospace}
footer{text-align:center;color:var(--muted);padding:32px 0 44px;font-size:12px;border-top:1px solid var(--line);margin-top:28px}
.ok{color:var(--ok)} .danger{color:var(--danger)}
.colorbox{width:28px;height:28px;border-radius:8px;border:1px solid var(--line);background:#000}
.note{font-size:12px;color:var(--muted)}
.group{border:1px dashed var(--line);border-radius:12px;padding:12px}
.switch{display:inline-flex;align-items:center;gap:8px}
.switch input{width:18px;height:18px;accent-color:var(--accent)}
.small{font-size:12px}
#btn-buzzer,#btn-relay{display:inline-flex;align-items:center;justify-content:center;white-space:nowrap;min-width:160px;padding:10px 14px;border-radius:12px}
.card .note{margin-top:8px}
/* (คงสไตล์ของ range ไว้เผื่อใช้ที่อื่น แม้ไม่มีสไลเดอร์ในหน้านี้) */
input[type="range"]{-webkit-appearance:none;appearance:none;height:6px;border-radius:999px;background:rgba(255,255,255,.12);outline:none}
input[type="range"]::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:16px;height:16px;border-radius:50%;background:var(--accent);border:2px solid rgba(0,0,0,.35);margin-top:-5px}
input[type="range"]::-moz-range-thumb{width:16px;height:16px;border-radius:50%;background:var(--accent);border:2px solid rgba(0,0,0,.35)}
</style>
</head>
<body>
<header>
  <div class="row" style="justify-content:space-between">
    <h1>🔥 Smart Fire Alarm</h1>
    <a class="btn-outline btn" href="/manager">ตั้งค่า Wi-Fi</a>
  </div>
</header>

<div class="wrap">

  <!-- โหมด Manual / Auto -->
  <div class="grid">
    <div class="card">
      <div class="title">โหมดการทำงาน</div>
      <div class="row">
        <div>โหมด: <span id="mode" class="badge">Manual</span></div>
      </div>
      <div class="row" style="margin-top:8px">
        <button id="btn-man" class="btn" onclick="setMode('manual')">Manual</button>
        <button id="btn-auto" class="btn-ghost" onclick="setMode('auto')">Auto</button>
      </div>
      <div class="note">* เริ่มต้นระบบจะอยู่ที่ Manual</div>
    </div>

    <!-- เปิด/ปิดกลุ่มอุปกรณ์ -->
    <div class="card">
      <div class="title">Enable/Disable กลุ่มอุปกรณ์ (Manual Mode)</div>
      <div class="group">
        <div class="switch"><input id="en-smoke"  type="checkbox" onchange="toggleEnable('smoke', this.checked)"><label for="en-smoke">Smoke</label></div>
        <div class="switch"><input id="en-pump"   type="checkbox" onchange="toggleEnable('pump', this.checked)"><label for="en-pump">Pump</label></div>
        <div class="switch"><input id="en-buzzer" type="checkbox" onchange="toggleEnable('buzzer', this.checked)"><label for="en-buzzer">Buzzer</label></div>
        <div class="switch"><input id="en-rgb"    type="checkbox" onchange="toggleEnable('rgb', this.checked)"><label for="en-rgb">RGB</label></div>
      </div>
      <div class="note">* ใช้ได้เมื่อโหมดเป็น Manual เท่านั้น (Auto จะไม่สนใจการตั้งค่านี้)</div>
    </div>
  </div>

  <!-- สถานะเซ็นเซอร์/อุปกรณ์ -->
  <div class="grid">
    <div class="card">
      <div class="title">สถานะควัน (DO @ GPIO23)</div>
      <div id="smoke" class="value">--</div>
    </div>

    <div class="card">
      <div class="title">อุณหภูมิ/ความชื้น (DHT11 @ GPIO22)</div>
      <div id="tempC" class="value">--°C</div>
      <div class="small note">ความชื้น: <span id="tempRaw" class="mono">--</span>%RH</div>
    </div>

    <div class="card">
      <div class="title">ปั๊มน้ำ / รีเลย์</div>
      <button id="btn-relay" onclick="toggleRelay()">รีเลย์: --</button>
      <div class="small note">Manual เท่านั้น (ถ้า disable ไว้จะสั่งไม่ได้)</div>
    </div>

    <div class="card">
      <div class="title">บัซเซอร์</div>
      <button id="btn-buzzer" onclick="toggleBuzzer()">บัซเซอร์: --</button>
      <div class="small note">Manual เท่านั้น (ถ้า disable ไว้จะสั่งไม่ได้)</div>
    </div>

    <div class="card">
      <div class="title">ไฟ RGB</div>
      <div class="row">
        <div class="colorbox" id="cb"></div>
        <div class="mono" id="rgbtxt">r0 g0 b0</div>
      </div>
      <div class="row" style="margin-top:8px">
        <button class="btn-outline" onclick="onRGB()">เปิดไฟ</button>
        <button class="btn-outline" onclick="offRGB()">ปิดไฟ</button>
      </div>
    </div>
  </div>

  <!-- เครือข่าย -->
  <div class="grid">
    <div class="card">
      <div class="title">เครือข่าย</div>
      <div class="row">
        <div>Wi-Fi: <span id="wconn" class="badge">--</span></div>
        <div>SSID: <span id="wssid" class="mono">--</span></div>
        <div>IP:   <span id="wip"   class="mono">--</span></div>
        <div>RSSI: <span id="wrssi" class="mono">--</span></div>
      </div>
      <div class="title" style="margin-top:8px">อัปเดตทุก ~0.5s</div>
    </div>
  </div>

</div>

<footer>© Smart Fire Alarm System</footer>

<script>
let state = {
  auto:false,
  groups:{smoke:true,pump:true,buzzer:true,rgb:true},
  smoke:false, relay:false, buzzer:false, rgb:{r:0,g:0,b:0}
};

// แสดงสถานะสีบนการ์ดจาก state.rgb
function updateRgbText(){
  const r = (state.rgb?.r|0), g = (state.rgb?.g|0), b = (state.rgb?.b|0);
  document.getElementById('rgbtxt').textContent = `r${r} g${g} b${b}`;
  document.getElementById('cb').style.background = `rgb(${r},${g},${b})`;
}

function reflectMode(){
  document.getElementById('mode').textContent = state.auto ? 'Auto' : 'Manual';
  document.getElementById('btn-man').className  = state.auto ? 'btn-ghost' : 'btn';
  document.getElementById('btn-auto').className = state.auto ? 'btn' : 'btn-ghost';
}
function reflectGroups(){
  document.getElementById('en-smoke').checked  = !!state.groups.smoke;
  document.getElementById('en-pump').checked   = !!state.groups.pump;
  document.getElementById('en-buzzer').checked = !!state.groups.buzzer;
  document.getElementById('en-rgb').checked    = !!state.groups.rgb;
}

async function fetchStatus(){
  try{
    const j = await (await fetch('/api/status')).json();
    state = j;

    reflectMode(); reflectGroups();

    document.getElementById('smoke').textContent = j.smoke ? 'พบควัน' : 'ปกติ';
    document.getElementById('smoke').className = 'value ' + (j.smoke ? 'danger' : 'ok');

    document.getElementById('btn-relay').textContent  = 'รีเลย์: '  + (j.relay ? 'ON':'OFF');
    document.getElementById('btn-buzzer').textContent = 'บัซเซอร์: ' + (j.buzzer? 'ON':'OFF');

    // RGB preview จากสถานะจริง
    updateRgbText();

    // Network
    document.getElementById('wconn').textContent = j.wifi_connected ? 'Connected' : 'AP Mode';
    document.getElementById('wssid').textContent = j.ssid || '-';
    document.getElementById('wip').textContent   = j.ip || '-';
    document.getElementById('wrssi').textContent = j.rssi;

    // Temperature/Humidity
    const tC   = (j.temp_c  ?? j.tempC  ?? null);
    const hPct = (j.temp_raw?? j.temperatureRaw ?? null); // ใช้เป็น %RH
    const tempCEl = document.getElementById('tempC');
    const tempRawEl = document.getElementById('tempRaw');
    if (tempCEl)  tempCEl.textContent  = (tC==null||Number.isNaN(+tC)) ? '--°C' : ((+tC).toFixed(2)+'°C');
    if (tempRawEl) tempRawEl.textContent= (hPct==null||Number.isNaN(+hPct)) ? '--' : (+hPct).toFixed(0);
  }catch(e){ console.log(e); }
}

async function setMode(m){ await fetch('/api/mode?m='+m); fetchStatus(); }
async function toggleEnable(device, on){ await fetch(`/api/enable?device=${encodeURIComponent(device)}&on=${on?1:0}`); fetchStatus(); }
async function toggleRelay(){ await fetch('/api/relay?on='+(state.relay?0:1));   fetchStatus(); }
async function toggleBuzzer(){ await fetch('/api/buzzer?on='+(state.buzzer?0:1)); fetchStatus(); }

// เปิดไฟ: ใช้สีตามสถานะจริง (ควัน=แดง, ปกติ=เขียว) ทำงานได้ทุกโหมด
async function onRGB(){
  const r = state.smoke ? 255 : 0;
  const g = state.smoke ? 0   : 255;
  const b = 0;

  state.rgb = { r, g, b }; // อัปเดตหน้าทันที
  updateRgbText();

  await fetch(`/api/rgb?r=${r}&g=${g}&b=${b}`);
  fetchStatus();
}

// ปิดไฟ (ดำ)
async function offRGB(){
  state.rgb = { r:0, g:0, b:0 };
  updateRgbText();

  await fetch('/api/rgb?r=0&g=0&b=0');
  fetchStatus();
}

// อัปเดตถี่ขึ้นเป็น 500 ms
setInterval(fetchStatus, 500);
fetchStatus();
</script>
</body>
</html>
)HTML";
