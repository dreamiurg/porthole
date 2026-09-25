// Browser host (Emscripten). Same loop as sim.cpp/main.cpp: pointer -> InputTracker -> Game, fb -> palette -> canvas,
// saves -> localStorage ("pets-club.s0".."s2", hex), buzzer pattern -> Web Audio square wave. Built by `make play`.
#include <emscripten.h>
#include <stdint.h>
#include "game.h"

static Game g_game;
static InputTracker g_tracker;
static uint32_t g_pal[TINT_COUNT][C_COUNT];  // RGBA bytes as little-endian words (0xAABBGGRR), ready for ImageData
static uint32_t g_pix[gfx::W * gfx::H];
static bool g_down = false, g_latch = false; static int g_x = 0, g_y = 0;
static uint32_t g_lastMs = 0, g_soundUntil = 0; static bool g_beep = false;

EM_JS(void, js_init, (), {
  const cv = document.getElementById('screen');
  Module.ctx2d = cv.getContext('2d');
  Module.img = Module.ctx2d.createImageData(160, 160);
  const at = (e) => { const r = cv.getBoundingClientRect();
    return [Math.min(159, Math.max(0, Math.floor((e.clientX - r.left) * 160 / r.width))),
            Math.min(159, Math.max(0, Math.floor((e.clientY - r.top) * 160 / r.height)))]; };
  let down = false;
  cv.addEventListener('pointerdown', (e) => {
    if (!e.isPrimary || e.button !== 0) return;  // right/middle click is not a finger
    e.preventDefault(); cv.setPointerCapture(e.pointerId); down = true;
    if (!Module.audio) {  // a pointerdown is a user gesture, so the context is allowed to start here
      const a = new (window.AudioContext || window.webkitAudioContext)(), o = a.createOscillator(), g = a.createGain();
      o.type = 'square'; o.frequency.value = 2000; g.gain.value = 0; o.connect(g).connect(a.destination); o.start();
      Module.audio = a; Module.gain = g;
    } else if (Module.audio.state !== 'running') Module.audio.resume();
    const p = at(e); _web_touch(1, p[0], p[1]);
  });
  cv.addEventListener('pointermove', (e) => { if (down && e.isPrimary) { const p = at(e); _web_touch(1, p[0], p[1]); } });
  const up = (e) => { if (!e.isPrimary) return; down = false; const p = at(e); _web_touch(0, p[0], p[1]); };
  cv.addEventListener('pointerup', up); cv.addEventListener('pointercancel', up);
  cv.addEventListener('contextmenu', (e) => e.preventDefault());  // a long press is a belly rub, not a menu
});

// Local wall-clock seconds, the same convention as the board's RTC (no timezone handling in the game).
EM_JS(double, js_local_epoch, (), { const d = new Date(); return Math.floor(d.getTime() / 1000) - d.getTimezoneOffset() * 60; });
// Never hand the game an earlier second than last time: a DST fall-back or a clock change must not run game time backwards.
static uint32_t g_epoch = 0;
static uint32_t epochNow() { uint32_t e = (uint32_t)js_local_epoch(); if (e > g_epoch) g_epoch = e; return g_epoch; }

EM_JS(void, js_present, (const uint32_t* pix), {
  Module.img.data.set(HEAPU8.subarray(pix, pix + 160 * 160 * 4)); Module.ctx2d.putImageData(Module.img, 0, 0);
});

EM_JS(int, js_load, (int slot, uint8_t* out, int max), {
  let h = ""; try { h = localStorage.getItem('pets-club.s' + slot) || ""; } catch (e) {}  // storage off: no save
  const n = Math.min(max, h.length >> 1);
  for (let i = 0; i < n; i++) HEAPU8[out + i] = parseInt(h.substr(i * 2, 2), 16);
  return n;
});
EM_JS(void, js_store, (int slot, const uint8_t* p, int len), {
  if (window.petsClubResetting) return;  // the reset link cleared storage and is reloading; do not write it back
  let h = ""; for (let i = 0; i < len; i++) h += HEAPU8[p + i].toString(16).padStart(2, '0');
  try { localStorage.setItem('pets-club.s' + slot, h); } catch (e) {}
});
EM_JS(void, js_erase, (int slot), { try { localStorage.removeItem('pets-club.s' + slot); } catch (e) {} });

// Schedule the buzzer on/off `inSec` seconds from now. Silent until the first tap has unlocked audio.
EM_JS(void, js_beep, (int on, double inSec), {
  if (Module.gain) Module.gain.gain.setValueAtTime(on ? 0.03 : 0, Module.audio.currentTime + 0.02 + inSec);
});

extern "C" EMSCRIPTEN_KEEPALIVE void web_touch(int down, int x, int y) {
  if (down && !g_down) g_latch = true;  // a click shorter than one frame still reaches the game as a press
  g_down = down; g_x = x; g_y = y;
}

static void tick() {
  uint32_t ms = (uint32_t)emscripten_get_now();
  // Particles and the fetch drops move per frame, tuned for the ~30 fps the device and the webemu host run at.
  if (ms - g_lastMs < 30) return;
  g_lastMs = ms;
  Input in = g_tracker.step(g_down || g_latch, g_x, g_y, ms); g_latch = false;
  g_game.update(epochNow(), ms, in);
  g_game.render();
  const uint32_t* pal = g_pal[g_game.tint()];
  for (int i = 0; i < gfx::W * gfx::H; i++) g_pix[i] = pal[gfx::fb[i]];  // the round glass is the canvas's CSS border-radius
  js_present(g_pix);

  // soundOn() is the firmware's own pattern player (mute included). Its beeps are 20-70 ms, shorter than a frame,
  // so sample it per millisecond one frame ahead and schedule the edges instead of polling once per frame.
  uint32_t from = g_soundUntil > ms && g_soundUntil - ms < 1000 ? g_soundUntil : ms;
  for (uint32_t t = from; t < ms + 33; t++) {
    bool on = g_game.soundOn(t);
    if (on != g_beep) { g_beep = on; js_beep(on, (t - ms) / 1000.0); }
  }
  g_soundUntil = ms + 33;

  // localStorage has no flash wear, so unlike main.cpp every dirty house is written straight away, as sim.cpp does.
  Save s; int slot;
  while (g_game.takeSave(&s, &slot, true)) js_store(slot, (const uint8_t*)&s, sizeof s);
  while (g_game.takeErase(&slot)) js_erase(slot);
}

int main() {
  for (int t = 0; t < TINT_COUNT; t++) {
    uint32_t rgb[C_COUNT]; palette_build((Tint)t, rgb);
    for (int i = 0; i < C_COUNT; i++) g_pal[t][i] = 0xFF000000u | (rgb[i] & 0xFF) << 16 | (rgb[i] & 0xFF00) | (rgb[i] >> 16 & 0xFF);
  }
  Save houses[MAX_HOUSES]; int n = 0; uint8_t blob[256];
  for (int slot = 0; slot < MAX_HOUSES; slot++) {
    int got = js_load(slot, blob, sizeof blob);
    if (got && pet::loadBlob(blob, (size_t)got, houses[n])) n++;
  }
  js_init();
  g_lastMs = (uint32_t)emscripten_get_now();
  g_game.begin(epochNow(), g_lastMs, houses, n);
  emscripten_set_main_loop(tick, 0, false);  // requestAnimationFrame; tick() gates it down to the game's frame rate
  return 0;
}
