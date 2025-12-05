#ifndef WEB_APP_H
#define WEB_APP_H

// A single-page web app served at /
// Manual mode track with draggable knob:
// - horizontal drag = direction/position
// - vertical drag distance = speed (%)

static const char WEB_INDEX[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html><head><meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no"/>
<title>SlidePilot Web App</title>
<style>
:root{--bg:#101418;--surface:#0e1419;--surface2:#121a22;--pri:#00e5ff;--text:#fff;--dim:#8fa1b3;--sep:#24313c;}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html,body{margin:0;height:100%;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,Segoe UI,Roboto}
.app{max-width:960px;margin:0 auto;padding:12px}
.card{background:var(--surface);border:1px solid var(--sep);border-radius:12px;padding:12px;margin-bottom:12px}
h1{margin:6px 0 12px;font-size:18px}
.row{display:flex;gap:12px;flex-wrap:wrap;align-items:center}
.btn{background:var(--surface2);border:1px solid var(--sep);color:var(--text);border-radius:10px;padding:10px 14px;cursor:pointer}
.btn.primary{background:var(--pri);border-color:transparent;color:#00313a;font-weight:700}
.pill{display:inline-block;background:var(--pri);color:#00313a;border-radius:999px;padding:6px 10px;font-weight:800}
.muted{color:var(--dim)}
/* Manual track */
.trackWrap{position:relative;height:120px}
.track{position:absolute;left:24px;right:24px;top:50%;height:2px;background:#27323d;transform:translateY(-50%)}
.knob{position:absolute;width:36px;height:36px;border-radius:50%;background:var(--pri);border:2px solid #0a2630;box-shadow:0 2px 8px rgba(0,0,0,.4);touch-action:none}
.speedBadge{position:absolute;right:10px;top:10px}
</style></head>
<body>
<div class="app">
  <div class="card">
    <h1>SlidePilot <span class="pill">Web App</span></h1>
    <div class="muted">Connect to the camera slider AP, then visit <b>http://192.168.4.1</b>.</div>
  </div>

  <div class="card">
    <h1>Manual Mode</h1>
    <div class="trackWrap" id="tw">
      <div class="track" id="track"></div>
      <div class="knob" id="knob"></div>
      <div class="pill speedBadge" id="spd">Speed: 40%</div>
    </div>
    <div class="muted">Drag knob left/right to move. Drag up for faster, down for slower.</div>
    <div class="row" style="margin-top:10px">
      <button class="btn" onclick="stop()">Stop</button>
      <button class="btn primary" onclick="center()">Center Knob</button>
    </div>
  </div>

  <div class="card">
    <h1>Quick Actions</h1>
    <div class="row">
      <button class="btn" onclick="go('/api/jog?mm=-10')">◀ Jog -10mm</button>
      <button class="btn" onclick="go('/api/jog?mm=10')">Jog +10mm ▶</button>
      <button class="btn" onclick="go('/api/setSpeed?p=50')">Speed 50%</button>
      <button class="btn" onclick="go('/api/setSpeed?p=100')">Speed 100%</button>
    </div>
  </div>
</div>

<script>
const knob = document.getElementById('knob');
const track = document.getElementById('track');
const spd   = document.getElementById('spd');
let dragging=false, startX=0, startY=0, baseX=0, baseSpeed=40, speed=40;

function layoutKnob(x){
  const rect = track.getBoundingClientRect();
  const minX = rect.left-18 + window.scrollX;
  const maxX = rect.right-18 + window.scrollX;
  if (x<minX) x=minX; if (x>maxX) x=maxX;
  knob.style.top = (rect.top - 18 + window.scrollY) + 'px';
  knob.style.left = x + 'px';
}
function center(){
  const rect = track.getBoundingClientRect();
  layoutKnob(rect.left + (rect.width/2) - 18 + window.scrollX);
  go('/api/stop');
}
function stop(){ go('/api/stop'); }

knob.addEventListener('pointerdown',(e)=>{
  dragging=true; knob.setPointerCapture(e.pointerId);
  startX=e.clientX; startY=e.clientY; const r=knob.getBoundingClientRect(); baseX=r.left+18+window.scrollX; baseSpeed=speed;
});
knob.addEventListener('pointermove',(e)=>{
  if (!dragging) return;
  const dx=e.clientX - startX;
  const dy=e.clientY - startY;
  const rect=track.getBoundingClientRect();
  const x = baseX + dx;
  layoutKnob(x);
  // dir from center:
  const centerX = rect.left + rect.width/2;
  const dir = (x+18 - centerX) >= 0 ? 1 : -1;
  // speed from vertical drag (up = faster, down = slower):
  let p = baseSpeed - dy*0.4;
  if (p<5) p=5; if (p>100) p=100;
  speed = Math.round(p);
  spd.textContent = 'Speed: ' + speed + '%';
  // send drive command (debounced)
  drive(dir, speed);
});
knob.addEventListener('pointerup',()=>{ dragging=false; });

function drive(dir, p){
  // dir: 1 right, -1 left; p: 5..100
  fetch('/api/drive?dir='+(dir>0?1:-1)+'&p='+p).catch(()=>{});
}
function go(path){ fetch(path).catch(()=>{}); }

window.addEventListener('resize', ()=>center());
document.addEventListener('DOMContentLoaded', ()=>center());
</script>
</body></html>
)HTML";

#endif
