// manager_wifi.h
#pragma once
// หน้าเว็บตั้งค่า Wi-Fi อย่างเดียว (ไม่มีหน้า Dashboard อื่น)
// จะมีปุ่มสแกน SSID / กดเลือก / ใส่รหัส / Save & Connect
// ช่อง AP IP แสดง IP ปัจจุบัน (ตอนเป็น AP จะเป็น 192.168.4.1)

const char MANAGER_WIFI_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="th"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Room Status – Wi-Fi Setup</title>
<style>
  :root{color-scheme:dark light}
  body{font-family:system-ui,Segoe UI,Roboto,Arial;margin:0;background:#0b1220;color:#e6edf3}
  .wrap{max-width:720px;margin:24px auto;padding:16px}
  .card{background:#111a2b;border:1px solid #21304a;border-radius:14px;padding:14px}
  .row{display:flex;gap:8px;flex-wrap:wrap}
  input,button{padding:10px;border-radius:10px;border:1px solid #2b3d66;background:#0f172a;color:#fff}
  button{cursor:pointer}
  .pill{display:inline-block;padding:6px 10px;border:1px solid #2a3a5e;border-radius:999px;margin:4px 6px}
  a{color:#7ee787}
</style>
</head>
<body>
<div class="wrap">
  <h2>Room Status – ตั้งค่า Wi-Fi</h2>

  <div class="card">
    <div class="row">
      <input id="ssid" placeholder="SSID" autocomplete="off">
      <input id="pass" placeholder="Password" type="password" autocomplete="off">
      <button id="save">Save & Connect</button>
    </div>
    <p id="msg" class="pill" style="display:inline-block;"></p>
  </div>

  <div class="card" style="margin-top:12px;">
    <button id="scan">Scan Wi-Fi</button>
    <div id="nets" style="margin-top:10px;"></div>
  </div>

  <div class="card" style="margin-top:12px;">
    <button id="reset">ล้างค่าที่บันทึกไว้</button>
  </div>

  <p style="opacity:.7;margin-top:10px;">AP IP: <b id="apip">{{AP_IP}}</b></p>
</div>

<script>
const $ = (id)=>document.getElementById(id);

$('scan').onclick=async()=>{
  try{
    const r=await fetch('/scan'); const arr=await r.json();
    const div=$('nets'); div.innerHTML='';
    arr.forEach(n=>{
      const b=document.createElement('button');
      b.textContent=`${n.ssid} (RSSI ${n.rssi})`;
      b.className='pill'; b.onclick=()=>{$('ssid').value=n.ssid;};
      div.appendChild(b);
    });
    if(arr.length===0){ div.textContent='ไม่พบเครือข่าย'; }
  }catch{ $('nets').textContent='สแกนไม่สำเร็จ'; }
};

$('save').onclick=async()=>{
  const fd=new URLSearchParams(); 
  fd.append('ssid',$('ssid').value||''); 
  fd.append('pass',$('pass').value||'');
  try{
    const r=await fetch('/save',{method:'POST',body:fd}); 
    const j=await r.json();
    if(j.ok){
      $('msg').innerHTML = j.ip 
        ? `เชื่อมต่อสำเร็จ IP: <b>${j.ip}</b>`
        : 'บันทึกแล้ว กำลังเชื่อมต่อ...';
      if(j.ip){ setTimeout(()=>{ location.href=`http://${j.ip}/`; }, 1200); }
    }else{
      $('msg').textContent = (j && j.msg) ? j.msg : 'บันทึกไม่สำเร็จ';
    }
  }catch(e){
    $('msg').textContent='ส่งข้อมูลไม่สำเร็จ';
  }
};

$('reset').onclick=async()=>{
  try{
    const r=await fetch('/reset',{method:'POST'}); 
    const j=await r.json();
    $('msg').textContent = j && j.ok ? 'ล้างค่าแล้ว กลับสู่โหมด AP' : 'ล้างค่าไม่สำเร็จ';
  }catch{ $('msg').textContent='ล้างค่าไม่สำเร็จ'; }
};
</script>
</body></html>)HTML";
