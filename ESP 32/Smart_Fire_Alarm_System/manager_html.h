#pragma once
const char MANAGER_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="th">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Wi-Fi Manager</title>
<style>
:root{--card:#fff;--bg:#f6f7fb;--text:#111827;--muted:#6b7280;--accent:#2563eb}
body{margin:0;font-family:ui-sans-serif,system-ui,Segoe UI,Roboto,Arial;background:var(--bg);color:var(--text)}
.wrap{max-width:760px;margin:24px auto;padding:0 16px}
.card{background:var(--card);border-radius:16px;padding:16px;box-shadow:0 6px 18px rgba(0,0,0,.06)}
h1{font-size:22px;margin:0 0 12px}
.muted{color:var(--muted);font-size:13px}
input{width:100%;padding:10px 12px;border:1px solid #e5e7eb;border-radius:12px;outline:none}
button{appearance:none;border:0;background:var(--accent);color:#fff;padding:10px 14px;border-radius:12px;cursor:pointer;font-weight:600}
.row{display:flex;gap:12px;flex-wrap:wrap;align-items:center}
.list{margin-top:12px}
.item{background:#fff;border:1px solid #e5e7eb;border-radius:12px;padding:10px 12px;display:flex;justify-content:space-between;align-items:center;margin-bottom:8px}
.mono{font-family:ui-monospace,Menlo,Consolas,monospace}
a{color:var(--accent);text-decoration:none}
</style>
</head>
<body>
<div class="wrap">
  <div class="card">
    <h1>🔧 ตั้งค่า Wi-Fi</h1>
    <p class="muted">เลือกเครือข่ายหรือกรอกด้วยตนเอง แล้วกดบันทึก อุปกรณ์จะรีสตาร์ทและพยายามเชื่อมต่อ</p>
    <div class="row">
      <button onclick="scan()">สแกนเครือข่าย</button>
      <a href="/" style="margin-left:auto;">กลับหน้า Dashboard</a>
    </div>
    <div class="list" id="nets"></div>
    <form id="f" style="margin-top:12px" onsubmit="save(event)">
      <label>SSID</label>
      <input id="ssid" name="ssid" placeholder="ชื่อ Wi-Fi (SSID)" required />
      <div style="height:8px"></div>
      <label>รหัสผ่าน</label>
      <input id="pass" name="pass" placeholder="Password" type="password" required />
      <div style="height:12px"></div>
      <button type="submit">บันทึกและรีสตาร์ท</button>
    </form>
  </div>
</div>
<script>
async function scan(){
  const list = document.getElementById('nets');
  list.textContent = 'กำลังสแกน...';
  try{
    const nets = await (await fetch('/scan')).json();
    if (!Array.isArray(nets)||!nets.length){ list.innerHTML = '<div class="muted">ไม่พบเครือข่าย</div>'; return; }
    list.innerHTML = nets.map(n=>`
      <div class="item">
        <div>
          <div class="mono">${escapeHtml(n.ssid||'-')}</div>
          <div class="muted">RSSI: ${n.rssi}</div>
        </div>
        <button onclick="pick('${escapeAttr(n.ssid||'')}')">เลือก</button>
      </div>
    `).join('');
  }catch(e){ list.innerHTML = '<div class="muted">สแกนไม่สำเร็จ</div>'; }
}
function pick(ssid){ document.getElementById('ssid').value=ssid; document.getElementById('pass').focus(); }
async function save(ev){
  ev.preventDefault();
  const fd = new FormData(document.getElementById('f'));
  const r = await fetch('/savewifi',{method:'POST',body:fd});
  document.body.innerHTML = await r.text();
}
function escapeHtml(s){return (s||'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));}
function escapeAttr(s){return (s||'').replace(/['"\\]/g,c=>'\\'+c);}
</script>
</body>
</html>
)HTML";
