#pragma once
const char MANAGER_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="th">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 – WiFi Manager</title>
<style>
  :root { font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, sans-serif; }
  body { margin: 0; background: #0b1220; color: #e6edf3; }
  .wrap { max-width: 720px; margin: 24px auto; padding: 0 16px; }
  .card { background:#121a2b; border:1px solid #1f2a44; border-radius:14px; padding:16px; margin-bottom:12px; }
  .title { font-size: 22px; font-weight: 800; margin-bottom: 10px; }
  .row { display:flex; gap:8px; align-items:center; flex-wrap: wrap; }
  label { display:block; font-size:12px; opacity:.8; margin-bottom:6px; }
  input, select, button { width:100%; padding:10px; border-radius:10px; border:1px solid #2a3a5e; background:#0f1729; color:#fff; }
  button { cursor:pointer; }
  .list { display:flex; flex-direction:column; gap:8px; margin-top:8px; }
  .ssid { padding:10px; border:1px dashed #2a3a5e; border-radius:10px; cursor:pointer; }
  .ok { color:#7ef; }
  .warn { color:#ffb86b; }
</style>
</head>
<body>
<div class="wrap">
  <div class="title">ตั้งค่า Wi-Fi ให้ ESP32</div>

  <div class="card">
    <div class="row">
      <button id="scanBtn">สแกนเครือข่าย</button>
    </div>
    <div id="scanList" class="list"></div>
  </div>

  <div class="card">
    <label>SSID</label>
    <input id="ssid" placeholder="ชื่อ Wi-Fi" />
    <label>รหัสผ่าน</label>
    <input id="pass" type="password" placeholder="รหัสผ่าน" />
    <div class="row" style="margin-top:8px">
      <button id="saveBtn">บันทึก & เชื่อมต่อ</button>
    </div>
    <div id="msg" style="margin-top:8px; font-size:13px; opacity:.9"></div>
  </div>

  <div class="card">
    <div class="row">
      <button id="toHomeBtn">ไปหน้า Home</button>
      <button id="reconfigBtn">บังคับเข้าโหมดตั้งค่า</button>
      <button id="resetBtn">ลบค่า Wi-Fi (NVS)</button>
    </div>
    <div style="margin-top:8px; font-size:12px; opacity:.7">
      AP IP: <span id="ip">{{AP_IP}}</span> · เข้าหน้านี้ได้ที่ <code>http://{{AP_IP}}/manager</code>
    </div>
  </div>
</div>

<script>
const scanBtn = document.getElementById('scanBtn');
const saveBtn = document.getElementById('saveBtn');
const resetBtn = document.getElementById('resetBtn');
const reconfigBtn = document.getElementById('reconfigBtn');
const toHomeBtn = document.getElementById('toHomeBtn');
const scanList = document.getElementById('scanList');
const msg = document.getElementById('msg');

scanBtn.onclick = async () => {
  scanList.innerHTML = 'กำลังสแกน...';
  try{
    const r = await fetch('/scan',{cache:'no-store'});
    const arr = await r.json();
    scanList.innerHTML = '';
    arr.sort((a,b)=>b.rssi-a.rssi).forEach(net=>{
      const div = document.createElement('div');
      div.className = 'ssid';
      div.textContent = `${net.ssid} (RSSI ${net.rssi} dBm)`;
      div.onclick = ()=>{
        document.getElementById('ssid').value = net.ssid;
      };
      scanList.appendChild(div);
    });
    if(!arr.length) scanList.textContent = 'ไม่พบเครือข่าย';
  }catch(e){
    scanList.innerHTML = '<span class="warn">สแกนไม่สำเร็จ</span>';
  }
};

saveBtn.onclick = async () => {
  const ssid = document.getElementById('ssid').value.trim();
  const pass = document.getElementById('pass').value;
  if(!ssid){ msg.innerHTML = '<span class="warn">กรุณาใส่ SSID</span>'; return; }
  msg.textContent = 'กำลังบันทึกและเชื่อมต่อ...';
  const body = new URLSearchParams(); body.append('ssid', ssid); body.append('pass', pass);
  try{
    const r = await fetch('/save', { method:'POST', body });
    const j = await r.json();
    if(j.ok){
      msg.innerHTML = '<span class="ok">เชื่อมต่อสำเร็จ! กำลังไปหน้า Home...</span>';
      setTimeout(()=>location.href = j.redirect || '/home', 1200);
    }else{
      msg.innerHTML = '<span class="warn">เชื่อมต่อไม่สำเร็จ ลองใหม่หรือเช็ครหัสผ่าน</span>';
    }
  }catch(e){ msg.textContent = 'ผิดพลาด'; }
};

resetBtn.onclick = async ()=>{
  await fetch('/reset',{method:'POST'});
  msg.innerHTML = '<span class="ok">ลบค่าแล้ว</span>';
};

reconfigBtn.onclick = async ()=>{
  await fetch('/reconfig',{method:'POST'});
  msg.textContent = 'เข้าโหมดตั้งค่าแล้ว (ถ้าอยู่โหมด AP อยู่แล้วจะไม่เห็นอะไรเปลี่ยน)';
};

toHomeBtn.onclick = ()=> location.href = '/home';
</script>
</body>
</html>)HTML";
