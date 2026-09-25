#!/usr/bin/env python3
"""Browser emulator for Pets Club. Wraps `build/host/snap --serve` behind a tiny
HTTP server so the game can be played with a mouse/touch, no hardware needed.

Run: make webemu        (or: python3 tools/webemu.py)
Then open http://127.0.0.1:8765
"""

import http.server
import json
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
    """Raw top-down RGB triples -> minimal PNG (8-bit truecolor, no filtering)."""
    stride = w * 3
    raw = bytearray()
    for y in range(h):
        raw.append(0)  # filter: None
        raw += rgb[y * stride : (y + 1) * stride]
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + _png_chunk(b"IHDR", ihdr) + _png_chunk(b"IDAT", zlib.compress(bytes(raw))) + _png_chunk(b"IEND", b"")


class Sim:
    """Owns the `snap --serve` child process. All access serialized by self.lock."""

    def __init__(self):
        if not os.path.exists(SNAP):
            print("build/host/snap missing, running make snap...", file=sys.stderr)
            subprocess.run(["make", "snap"], cwd=ROOT, check=True)
        self.proc = subprocess.Popen([SNAP, "--serve"], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        self.lock = threading.Lock()
        self.last_frame_time = time.time()

    def _cmd(self, line):
        """Send one command, consume its one-line response. Caller holds self.lock."""
        self.proc.stdin.write((line + "\n").encode())
        self.proc.stdin.flush()
        return self.proc.stdout.readline()

    def frame_png(self):
        with self.lock:
            now = time.time()
            elapsed_ms = max(0, min(int((now - self.last_frame_time) * 1000), 100))
            self.last_frame_time = now
            self._cmd(f"tick {elapsed_ms}")
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

    def control(self, cmd):
        secs = {"hour": 3600, "night": 8 * 3600, "day": 86400}
        with self.lock:
            if cmd in secs:
                self._cmd(f"skip {secs[cmd]}")
            elif cmd == "reset":
                self._cmd("reset")
            elif cmd.startswith("dbg:") and cmd[4:].isalpha():
                self._cmd("dbg " + cmd[4:])


PAGE = """<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Pets Club</title>
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
</style>
</head>
<body>
<h1>Pets Club</h1>
<canvas id="screen" width="480" height="480"></canvas>
<div id="buttons">
  <button data-cmd="hour">+1 hour</button>
  <button data-cmd="night">+8 hours</button>
  <button data-cmd="day">+1 day</button>
  <button data-cmd="reset">Reset game</button>
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

for (const btn of document.querySelectorAll('#buttons button')) {
  btn.addEventListener('click', () => post('/control', { cmd: btn.dataset.cmd }));
}

async function pollFrame() {
  try {
    const res = await fetch('/frame.png?t=' + Date.now());
    const bmp = await createImageBitmap(await res.blob());
    ctx.drawImage(bmp, 0, 0);
  } catch (e) { /* transient network hiccup, next poll will recover */ }
}
setInterval(pollFrame, 83);  // ~12/s
pollFrame();
</script>
</body>
</html>
"""


class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        pass  # quiet; /frame.png polls at ~12/s and would otherwise flood the console

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
    print(f"Pets Club web emulator: http://{HOST}:{PORT}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        sim.proc.terminate()


if __name__ == "__main__":
    main()
