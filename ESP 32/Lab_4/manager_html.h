#pragma once
const char MANAGER_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="th"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Wi-Fi Manager</title>
<style>
 body{font-family:system-ui,Segoe UI,Roboto,Arial;background:#0b1220;color:#e6edf3;margin:0}
 .wrap{max-width:720px;margin:24px auto;padding:16px}
 .card{background:#121a2b;border:1px solid #1f2a44;border-radius:14px;padding:16px}
 input,button{padding:10px;border-radius:10px;border:1px solid #2b3d66;background:#0f172a;color:#fff}
 button{cursor:pointer}
 .row{display:flex;gap:8px;flex-wrap:wrap}
 .pill{display:inline-block;padding:6px 10px;border:1px solid #2a3a5e;border-radius:999px;margin:4px 6px}
</style>
</head><body>
<div class="wrap">
  <h2>ESP32 Wi-Fi Manager</h2>
  <div class="card">
    <div class="row">
      <input id="ssid" placeholder="SSID">
      <input id="pass" placeholder="Password">
      <button id="save">Save & Connect</button>
      <a href="/home"><button>Go Home</button></a>
    </div>
    <p id="msg"></p>
  </div>
  <div class="card" style="margin-top:12px;">
    <button id="scan">Scan Wi-Fi</button>
    <div id="nets" style="margin-top:10px;"></div>
  </div>
  <p style="opacity:.7;margin-top:10px;">AP IP: {{AP_IP}}</p>
</div>
<script>
document.getElementById('scan').onclick=async()=>{
  const r=await fetch('/scan'); const arr=await r.json();
  const div=document.getElementById('nets'); div.innerHTML='';
  arr.forEach(n=>{
    const b=document.createElement('button');
    b.textContent=`${n.ssid} (RSSI ${n.rssi})`;
    b.className='pill'; b.onclick=()=>{ssid.value=n.ssid;};
    div.appendChild(b);
  });
};
document.getElementById('save').onclick=async()=>{
  const fd=new URLSearchParams(); fd.append('ssid',ssid.value); fd.append('pass',pass.value);
  const r=await fetch('/save',{method:'POST',body:fd}); const t=await r.text();
  try{const j=JSON.parse(t); if(j.ok&&j.redirect){ location.href=j.redirect; return; }}catch(e){}
  document.getElementById('msg').textContent = t.includes('ok')?'Saved.':t;
};
</script>
</body></html>)HTML";
