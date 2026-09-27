#!/usr/bin/env python3
"""Browser emulator for Porthole (the profile shell and its games). Wraps `build/host/snap --serve` behind a tiny
HTTP server so the games can be played with a mouse/touch, no hardware needed. The tilt pad under the screen is the
motion sensor (drag the ball: where gravity points in the glass's plane; the middle is lying flat); Shake jolts it.

Run: make webemu        (or: python3 tools/webemu.py)
Then open http://127.0.0.1:8765
"""

import http.server
import json
import math
import os
import struct
import subprocess
import sys
import threading
import time
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SNAP = os.path.join(ROOT, "build", "host", "snap")
HOST, PORT = "127.0.0.1", int(sys.argv[1]) if len(sys.argv) > 1 else int(os.environ.get("PORT", "8765"))


def _read_exact(f, n):
    buf = bytearray()
    while len(buf) < n:
        chunk = f.read(n - len(buf))
        if not chunk:
            raise EOFError("simulator closed stdout")
        buf += chunk
    return bytes(buf)


def _png_chunk(tag, data):
    return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)


def ppm_to_png(rgb, w, h):
    """Raw top-down RGB triples -> minimal PNG (8-bit truecolor, no filtering). zlib level 1: ~1-2 ms for a noisy
    480x480 frame (~50 KB) where the default level takes 8-13 ms; it is only ever sent over localhost."""
    stride = w * 3
    raw = b"".join(b"\x00" + rgb[y * stride : (y + 1) * stride] for y in range(h))  # filter byte 0: None
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + _png_chunk(b"IHDR", ihdr) + _png_chunk(b"IDAT", zlib.compress(raw, 1)) + _png_chunk(b"IEND", b"")


class Sim:
    """Owns the `snap --serve` child process. All access serialized by self.lock."""

    def __init__(self):
        if not os.path.exists(SNAP):
            print("build/host/snap missing, running make snap...", file=sys.stderr)
            subprocess.run(["make", "snap"], cwd=ROOT, check=True)
        self.proc = subprocess.Popen([SNAP, "--serve"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.lock = threading.Lock()
        self.last_frame_time = time.time()
        self.pending_ms = 0  # real time not yet simulated: the sim steps in whole 33 ms frames

    def _cmd(self, line):
        """Send one command, consume its one-line response. Caller holds self.lock."""
        self.proc.stdin.write((line + "\n").encode())
        self.proc.stdin.flush()
        return self.proc.stdout.readline()

    def frame_png(self):
        with self.lock:
            now = time.time()
            self.pending_ms += max(0, min(int((now - self.last_frame_time) * 1000), 100))
            self.last_frame_time = now
            frames, self.pending_ms = divmod(self.pending_ms, 33)
            if frames:
                self._cmd(f"tick {frames * 33}")
            self.proc.stdin.write(b"frame\n")
            self.proc.stdin.flush()
            header = self.proc.stdout.readline().decode().strip()
            n = int(header.split()[1])
            payload = _read_exact(self.proc.stdout, n)
        return ppm_to_png(payload, 480, 480)

    def input(self, kind, x, y):
        if kind not in ("down", "move", "up"):
            return
        lx, ly = max(0, min(159, int(x / 3))), max(0, min(159, int(y / 3)))
        with self.lock:
            self._cmd("up" if kind == "up" else f"{kind} {lx} {ly}")

    def tilt(self, x, y):
        """In-plane gravity (x right, y down, in g, at most 1): the rest of 1 g points into the glass, face up."""
        if not (math.isfinite(x) and math.isfinite(y)):  # json.loads takes NaN and Infinity; round() would raise
            return
        m = math.hypot(x, y)
        if m > 1:
            x, y, m = x / m, y / m, 1.0
        with self.lock:
            self._cmd(f"tilt {round(x * 1000)} {round(y * 1000)} {round(-math.sqrt(1 - m * m) * 1000)}")

    def control(self, cmd):
        secs = {"hour": 3600, "night": 8 * 3600, "day": 86400}
        with self.lock:
            if cmd in secs:
                self._cmd(f"skip {secs[cmd]}")
            elif cmd in ("reset", "shake"):
                self._cmd(cmd)
            elif cmd.startswith("dbg:") and cmd[4:].isalpha():
                self._cmd("dbg " + cmd[4:])


PAGE = """<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Porthole</title>
<style>
  html, body { background:#111; color:#ccc; font-family:-apple-system,Helvetica,sans-serif;
               margin:0; padding:24px 12px; display:flex; flex-direction:column; align-items:center; }
  h1 { font-size:15px; font-weight:600; color:#888; letter-spacing:.04em; text-transform:uppercase; margin:0 0 16px; }
  #screen { width:480px; height:480px; max-width:92vw; max-height:92vw; border-radius:50%;
            background:#202020; box-shadow:0 0 0 6px #333, 0 10px 30px rgba(0,0,0,.6); touch-action:none; }
  #buttons { margin-top:18px; display:flex; gap:8px; flex-wrap:wrap; justify-content:center; }
  button { background:#2a2a2a; color:#ddd; border:1px solid #444; border-radius:6px;
           padding:9px 14px; font-size:14px; cursor:pointer; }
  button:hover { background:#3a3a3a; }
  button:active { background:#222; }
  #tilt { margin-top:18px; display:flex; gap:16px; align-items:center; }
  #pad { width:180px; height:180px; cursor:grab; touch-action:none; }
  #tilt .col { display:flex; flex-direction:column; gap:8px; }
  #tiltRead { font-size:12px; color:#888; min-width:9em; }
</style>
</head>
<body>
<h1>Porthole</h1>
<canvas id="screen" width="480" height="480"></canvas>
<div id="tilt">
  <canvas id="pad" width="180" height="180" title="Where gravity points: drag the ball. The middle is lying flat. Arrow keys turn it, space shakes."></canvas>
  <div class="col">
    <button data-tilt="0,1">Upright</button>
    <button data-tilt="0,0">Flat</button>
    <button data-cmd="shake">Shake</button>
    <span id="tiltRead"></span>
  </div>
</div>
<div id="buttons">
  <button data-cmd="hour">+1 hour</button>
  <button data-cmd="night">+8 hours</button>
  <button data-cmd="day">+1 day</button>
  <button data-cmd="reset">Wipe all profiles</button>
  <span style="width:100%"></span>
  <button data-cmd="dbg:hungry">make hungry</button>
  <button data-cmd="dbg:sleepy">make sleepy</button>
  <button data-cmd="dbg:dirty">make muddy</button>
  <button data-cmd="dbg:poop">poop</button>
  <button data-cmd="dbg:hearts">7 hearts</button>
  <button data-cmd="dbg:books">unlock books</button>
  <button data-cmd="dbg:hats">unlock hats</button>
  <button data-cmd="dbg:dog">age: dog</button>
  <button data-cmd="dbg:grown">age: grown</button>
</div>
<script>
const canvas = document.getElementById('screen');
const ctx = canvas.getContext('2d');
let pressed = false, lastMove = 0;

function post(url, body) {
  fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) })
    .catch(() => {});
}

function canvasXY(e) {
  const r = canvas.getBoundingClientRect();
  const t = e.touches && e.touches.length ? e.touches[0] : e.changedTouches && e.changedTouches[0];
  const cx = t ? t.clientX : e.clientX, cy = t ? t.clientY : e.clientY;
  return { x: (cx - r.left) * canvas.width / r.width, y: (cy - r.top) * canvas.height / r.height };
}

function onDown(e) {
  e.preventDefault();
  pressed = true;
  const p = canvasXY(e);
  post('/input', { type: 'down', x: p.x, y: p.y });
}
function onMove(e) {
  if (!pressed) return;
  e.preventDefault();
  const now = performance.now();
  if (now - lastMove < 33) return;  // throttle to ~30/s
  lastMove = now;
  const p = canvasXY(e);
  post('/input', { type: 'move', x: p.x, y: p.y });
}
function onUp(e) {
  if (!pressed) return;
  e.preventDefault();
  pressed = false;
  post('/input', { type: 'up', x: 0, y: 0 });
}

canvas.addEventListener('mousedown', onDown);
window.addEventListener('mousemove', onMove);
window.addEventListener('mouseup', onUp);
canvas.addEventListener('mouseleave', (e) => { if (pressed) onUp(e); });
canvas.addEventListener('touchstart', onDown, { passive: false });
canvas.addEventListener('touchmove', onMove, { passive: false });
canvas.addEventListener('touchend', onUp, { passive: false });
canvas.addEventListener('touchcancel', onUp, { passive: false });

for (const btn of document.querySelectorAll('button[data-cmd]')) {
  btn.addEventListener('click', () => post('/control', { cmd: btn.dataset.cmd }));
}

// The motion sensor: the ball is gravity in the glass's plane (the ring is 1 g, the middle lying flat).
const pad = document.getElementById('pad'), pctx = pad.getContext('2d'), tiltRead = document.getElementById('tiltRead');
const PAD_C = 90, PAD_R = 72;
let tilt = { x: 0, y: 1 }, lastTilt = 0;
function drawPad() {
  pctx.clearRect(0, 0, 180, 180);
  pctx.fillStyle = '#1a1d24'; pctx.beginPath(); pctx.arc(PAD_C, PAD_C, 88, 0, 7); pctx.fill();
  pctx.strokeStyle = '#353b48'; pctx.lineWidth = 2;
  for (const r of [PAD_R, PAD_R / 2]) { pctx.beginPath(); pctx.arc(PAD_C, PAD_C, r, 0, 7); pctx.stroke(); }
  const bx = PAD_C + tilt.x * PAD_R, by = PAD_C + tilt.y * PAD_R;
  pctx.strokeStyle = 'rgba(240,183,94,.5)'; pctx.lineWidth = 4;
  pctx.beginPath(); pctx.moveTo(PAD_C, PAD_C); pctx.lineTo(bx, by); pctx.stroke();
  pctx.fillStyle = '#e9b86a'; pctx.beginPath(); pctx.arc(bx, by, 13, 0, 7); pctx.fill();
  const m = Math.hypot(tilt.x, tilt.y), deg = Math.round(Math.atan2(tilt.x, tilt.y) * 180 / Math.PI);
  tiltRead.textContent = m < 0.05 ? 'lying flat' : `down ${deg === 0 ? '' : Math.abs(deg) + (deg > 0 ? ' deg right' : ' deg left')}, ${Math.round(m * 100)}% g`;
}
function setTilt(x, y, force) {
  const m = Math.hypot(x, y);
  if (m > 1) { x /= m; y /= m; }
  tilt = { x, y };
  drawPad();
  const now = performance.now();
  if (force || now - lastTilt > 33) { lastTilt = now; post('/tilt', tilt); }
}
function padTilt(e) {
  const r = pad.getBoundingClientRect();
  setTilt(((e.clientX - r.left) * 180 / r.width - PAD_C) / PAD_R, ((e.clientY - r.top) * 180 / r.height - PAD_C) / PAD_R, false);
}
pad.addEventListener('pointerdown', (e) => { pad.setPointerCapture(e.pointerId); padTilt(e); });
pad.addEventListener('pointermove', (e) => { if (pad.hasPointerCapture(e.pointerId)) padTilt(e); });
pad.addEventListener('pointerup', () => setTilt(tilt.x, tilt.y, true));
for (const btn of document.querySelectorAll('[data-tilt]')) {
  btn.addEventListener('click', () => { const [x, y] = btn.dataset.tilt.split(',').map(Number); setTilt(x, y, true); });
}
window.addEventListener('keydown', (e) => {
  if (e.target.matches('button')) return;
  let a = Math.atan2(tilt.x, tilt.y), m = Math.hypot(tilt.x, tilt.y);
  if (e.key === 'ArrowLeft') a -= 0.15; else if (e.key === 'ArrowRight') a += 0.15;
  else if (e.key === 'ArrowUp') m = Math.max(0, m - 0.1); else if (e.key === 'ArrowDown') m = Math.min(1, m + 0.1);
  else if (e.key === ' ') { post('/control', { cmd: 'shake' }); e.preventDefault(); return; } else return;
  e.preventDefault();
  setTilt(Math.sin(a) * m, Math.cos(a) * m, true);
});
drawPad();

async function pollFrame() {
  try {
    const res = await fetch('/frame.png?t=' + Date.now());
    const bmp = await createImageBitmap(await res.blob());
    ctx.drawImage(bmp, 0, 0);
  } catch (e) { /* transient network hiccup, next poll will recover */ }
}
async function frameLoop() {  // ~25 frames a second, never two requests at once
  const start = performance.now();
  await pollFrame();
  setTimeout(frameLoop, Math.max(0, 40 - (performance.now() - start)));
}
frameLoop();
</script>
</body>
</html>
"""


class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        pass  # quiet; /frame.png polls at ~25/s and would otherwise flood the console

    def _send(self, code, content_type, body):
        self.send_response(code)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        if content_type == "image/png":
            self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            self._send(200, "text/html; charset=utf-8", PAGE.encode())
        elif self.path.startswith("/frame.png"):
            try:
                self._send(200, "image/png", sim.frame_png())
            except Exception as e:
                self.send_error(500, str(e))
        else:
            self.send_error(404)

    def do_POST(self):
        n = int(self.headers.get("Content-Length", 0) or 0)
        try:
            data = json.loads(self.rfile.read(n) or b"{}")
        except json.JSONDecodeError:
            self.send_error(400, "bad json")
            return
        if self.path == "/input":
            sim.input(data.get("type"), data.get("x", 0), data.get("y", 0))
            self._send(200, "application/json", b"{}")
        elif self.path == "/tilt":
            sim.tilt(float(data.get("x", 0)), float(data.get("y", 1)))
            self._send(200, "application/json", b"{}")
        elif self.path == "/control":
            sim.control(data.get("cmd", ""))
            self._send(200, "application/json", b"{}")
        else:
            self.send_error(404)


sim = None


def main():
    global sim
    sim = Sim()
    httpd = http.server.ThreadingHTTPServer((HOST, PORT), Handler)
    print(f"Porthole web emulator: http://{HOST}:{PORT}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        sim.proc.terminate()


if __name__ == "__main__":
    main()
