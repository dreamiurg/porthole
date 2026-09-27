// Porthole firmware entry point: the shell (profiles, launcher) hosting the games in APPS.
#include <Arduino.h>
#include "board.h"
#include "esp_heap_caps.h"
#include "games/biscuit/game.h"
#include "games/marble-kick/game.h"
#include "games/pets-club/game.h"
#include "gfx565.h"
#include "shell.h"

// The shell's storage is the board's NVS, addressed by (namespace, key).
struct NvsStore : shell::Store {
  size_t load(const char* ns, const char* key, void* buf, size_t max) override { return board::loadBlob(ns, key, buf, max); }
  void save(const char* ns, const char* key, const void* data, size_t len) override { board::saveBlob(ns, key, data, len); }
  void erase(const char* ns, const char* key) override { board::eraseBlob(ns, key); }
};

static Game g_pets;
static biscuit::Game g_biscuit;
static marble::Game g_marble;
static App* const APPS[] = {&g_pets, &g_biscuit, &g_marble};
static const int N_APPS = sizeof APPS / sizeof APPS[0];
static NvsStore g_store;
static Shell g_shell;
static InputTracker g_input;
static uint16_t g_pal[TINT_COUNT][C_COUNT];
static ActivityTracker g_activity;   // raw touch (even a swallowed one) or a move of the board: wakes and keeps it lit
static uint8_t g_backlight = 100;
static bool g_touchLog = false;
static bool g_hires = false;   // the last frame was an RGB565 app's
static uint32_t g_bootLocalEpoch = 0, g_bootMillis = 0;
// Test harness (tools/devctl.py): a synthetic finger held at logical (x, y) until `until`, then released.
static struct { bool on; int x, y; uint32_t until; } g_fake = {};
// Gravity for Input (milli-g, screen frame): the last good sensor reading, or the harness's "G" override while it is on.
static int16_t g_grav[3] = {0, 0, -1000};
static bool g_gravFake = false;
// Frame metrics for "M": per frame, in us, the loop period and the time spent in shell update, render and present, over
// the last MT_N frames.
enum { MT_FRAME, MT_UPDATE, MT_RENDER, MT_PRESENT, MT_COUNT };
static const int MT_N = 64;
static uint32_t g_mt[MT_N][MT_COUNT];
static int g_mtAt = 0;

// Wall clock: RTC if it runs, else continue from the last play time so the pets' day counts keep going.
static uint32_t nowSec() {
  return g_bootLocalEpoch + (millis() - g_bootMillis) / 1000u;
}

// Compile-time fallback clock: the time this firmware was built (local time of the build machine).
static uint32_t buildEpoch() {
  static const char* months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char mon[4] = {0}; int d, y, hh, mm, ss;
  sscanf(__DATE__, "%3s %d %d", mon, &d, &y); sscanf(__TIME__, "%d:%d:%d", &hh, &mm, &ss);
  int m = (int)((strstr(months, mon) - months) / 3) + 1;
  // days from civil
  int yy = y - (m <= 2); int era = yy / 400; unsigned yoe = (unsigned)(yy - era * 400);
  unsigned doy = (153u * (unsigned)(m + (m > 2 ? -3 : 9)) + 2u) / 5u + (unsigned)d - 1u;
  unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
  uint32_t days = (uint32_t)(era * 146097 + (int)doe - 719468);
  return days * 86400u + (uint32_t)(hh * 3600 + mm * 60 + ss);
}

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("\n[porthole] boot");
  board::init();
  for (int t = 0; t < TINT_COUNT; t++) {
    uint32_t rgb[C_COUNT]; palette_build((Tint)t, rgb);
    for (int i = 0; i < C_COUNT; i++) g_pal[t][i] = rgb888_to_565(rgb[i]);
  }
  g_shell.begin(g_store, APPS, N_APPS);   // first boot after Pets Club: its houses become profiles here
  uint32_t lastSeen = g_shell.lastSeen(), now;
  if (board::rtcValid()) now = board::rtcNow();
  else {
    now = lastSeen ? lastSeen + 60 : buildEpoch();
    if (now < buildEpoch()) now = buildEpoch();
    board::rtcSet(now);
    g_shell.clockRestored();   // else a kid capped today stays on "Back tomorrow": the guess is always that same day
    Serial.println("[porthole] RTC was not running; clock restored");
  }
  g_bootLocalEpoch = now; g_bootMillis = millis();
  Serial.printf("[porthole] profiles=%d now=%lu heap=%lu\n", g_shell.profileCount(), (unsigned long)now, (unsigned long)board::freeHeap());
  g_activity.step(Input{}, true, millis());   // boot counts as activity
}

// Idle dimming (no physical buttons: the screen is the only power control).
static const uint32_t DIM_MS = 60000;   // first dim step; the same minute and activity rule as shell::IDLE_MS
static void dimWhenIdle(uint32_t ms) {
  uint32_t idle = g_activity.idleMs(ms);
  uint8_t want = g_shell.asleep() ? (idle > 20000 ? 0 : 40) : (idle > 300000 ? 0 : idle > DIM_MS ? 30 : 100);
  if (want != g_backlight) { g_backlight = want; board::setBacklight(want); }
}

// The frame on the glass for tools/devctl.py. Indexed: "FB 160 160 idx\n", 160*160 palette indices, then the tint's 32
// RGB565 colours. RGB565: "FB 480 480 565 <runs>\n", then <runs> (count, color) uint16 pairs, little endian: the raw
// 460 KB would take 40 s at 115200 baud, flat UI art compresses to a few seconds.
static void dumpFrame() {
  if (!g_hires) {
    Serial.printf("FB %d %d idx\n", gfx::W, gfx::H);
    Serial.write(gfx::fb, gfx::W * gfx::H);
    Serial.write((const uint8_t*)g_pal[g_shell.tint()], sizeof g_pal[0]);
    Serial.flush();
    return;
  }
  const uint16_t* px = board::frontBuffer();
  const int n = gfx565::W * gfx565::H;
  for (int pass = 0, runs = 0; pass < 2; pass++) {   // count the runs, then send them
    if (pass) Serial.printf("FB %d %d 565 %d\n", gfx565::W, gfx565::H, runs);
    for (int i = 0; i < n;) {
      int j = i + 1;
      while (j < n && j - i < 65535 && px[j] == px[i]) j++;
      if (pass) { uint16_t run[2] = {(uint16_t)(j - i), px[i]}; Serial.write((const uint8_t*)run, sizeof run); }
      else runs++;
      i = j;
    }
  }
  Serial.flush();
}

// Serial maintenance: "T<epoch>" sets the clock (local wall-clock seconds), "R" wipes every profile and every game's
// saves, "S" prints stats, "D" toggles touch logging, "P<n>" clears profile n's secret code (a parent's escape hatch),
// "X<x>,<y>,<ms>" presses the screen and "F" dumps the frame (both for tools/devctl.py), "A" prints the accelerometer,
// "G<x>,<y>,<z>" holds gravity at that vector (milli-g, screen frame) until a bare "G", "M" prints frame metrics.
static void printMetrics() {
  uint32_t sum[MT_COUNT] = {}, worst = 0;
  for (auto& f : g_mt) { for (int k = 0; k < MT_COUNT; k++) sum[k] += f[k]; if (f[MT_FRAME] > worst) worst = f[MT_FRAME]; }
  auto ms = [&](int k) { return sum[k] / 1000.0 / MT_N; };
  Serial.printf("[metrics] frames=%d frame=%.2fms fps=%.1f max=%.2fms update=%.2fms render=%.2fms present=%.2fms "
                "heap=%lu psram=%lu largest_internal=%lu\n",
                MT_N, ms(MT_FRAME), ms(MT_FRAME) > 0 ? 1000.0 / ms(MT_FRAME) : 0.0, worst / 1000.0, ms(MT_UPDATE),
                ms(MT_RENDER), ms(MT_PRESENT), (unsigned long)board::freeHeap(),
                (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
}
static void gravityCommand() {  // "G" alone clears the override; parseInt would read it as 0,0,0 after a 1 s timeout
  String arg = Serial.readStringUntil('\n'); arg.trim();
  int x, y, z;
  if (!arg.length()) { g_gravFake = false; Serial.println("[porthole] gravity from sensor"); }
  else if (sscanf(arg.c_str(), "%d,%d,%d", &x, &y, &z) == 3) {
    g_gravFake = true; g_grav[0] = (int16_t)x; g_grav[1] = (int16_t)y; g_grav[2] = (int16_t)z;
    Serial.printf("[porthole] gravity held at %d,%d,%d\n", x, y, z);
  } else Serial.println("[porthole] usage: G<x>,<y>,<z> or G");
}
static void accelCommand() {
  int16_t raw[3], g[3];
  if (!board::readAccel(g[0], g[1], g[2], raw)) { Serial.println("[accel] no sensor reading"); return; }
  Serial.printf("[accel] raw=%d,%d,%d g=%d,%d,%d%s\n", raw[0], raw[1], raw[2], g[0], g[1], g[2], g_gravFake ? " (G override on)" : "");
}
static void motionCommand(int c) {   // A, G, M: split out of serialCommand for the complexity gate
  if (c == 'A') accelCommand();
  else if (c == 'G') gravityCommand();
  else if (c == 'M') printMetrics();
}
static void serialCommand(int c) {
  if (c == 'T') { uint32_t e = (uint32_t)Serial.parseInt(); if (e > 1600000000u) { board::rtcSet(e); g_bootLocalEpoch = e; g_bootMillis = millis(); Serial.println("[porthole] clock set"); } }
  else if (c == 'R') {
    board::eraseNamespace(shell::NS);
    for (App* a : APPS) board::eraseNamespace(a->store());
    Serial.println("[porthole] all profiles erased, rebooting"); delay(100); ESP.restart();
  }
  else if (c == 'P') {  // a digit is required: parseInt reads a bare "P" as 0 and would clear profile 0's code
    String arg = Serial.readStringUntil('\n'); arg.trim();
    if (!arg.length() || !isDigit(arg[0])) { Serial.println("[porthole] usage: P<n>"); return; }
    int n = arg.toInt(); g_shell.clearPin(n); Serial.printf("[porthole] profile %d code cleared\n", n);
  }
  else if (c == 'S') { g_shell.debugPrint(); Serial.printf("heap=%lu now=%lu\n", (unsigned long)board::freeHeap(), (unsigned long)nowSec()); }
  else if (c == 'X') {  // X<x>,<y>,<ms>: press at logical (x, y) for ms (the harness's tap/hold)
    int x = Serial.parseInt(), y = Serial.parseInt(), dur = Serial.parseInt();
    g_fake = {true, x, y, millis() + (uint32_t)(dur > 0 ? dur : 80)};
  }
  else if (c == 'F') dumpFrame();
  else if (c == 'D') { g_touchLog = !g_touchLog; Serial.printf("[porthole] touch log %s\n", g_touchLog ? "on" : "off"); }
  else motionCommand(c);
}

void loop() {
  static uint32_t prevUs = micros();
  uint32_t* mt = g_mt[g_mtAt];
  uint32_t t0 = micros();
  mt[MT_FRAME] = t0 - prevUs; prevUs = t0;
  uint32_t ms = millis();
  board::Touch t = board::readTouch();
  if (g_fake.on) {  // a harness press behaves like a finger (physical px), including waking the screen
    t = {ms < g_fake.until, g_fake.x * 3, g_fake.y * 3};
    if (!t.down) g_fake.on = false;
  }
  static bool swallow = false;  // the touch that wakes a dark screen is not a game input, until released
  if (!t.down) swallow = false;
  else if (g_backlight == 0) swallow = true;
  Input in = g_input.step(t.down && !swallow, t.x / 3, t.y / 3, ms);
  if (in.pressed && g_touchLog) Serial.printf("[touch] %d,%d\n", t.x, t.y);
  if (!g_gravFake) board::readAccel(g_grav[0], g_grav[1], g_grav[2]);   // keeps the last good value on a failed read
  in.gx = g_grav[0]; in.gy = g_grav[1]; in.gz = g_grav[2];
  g_activity.step(in, t.down, ms);   // picking up a dark board lights it; only a touch gets swallowed
  uint32_t t1 = micros();
  g_shell.update(nowSec(), ms, in);   // also saves: the open game at most every 5 s, profiles when they change
  uint32_t t2 = micros();
  g_hires = g_shell.surface() == SURFACE_RGB565;
  if (g_hires) gfx565::target(board::backBuffer());   // an RGB565 app draws straight into the panel's back buffer
  g_shell.render();
  uint32_t t3 = micros();
  if (g_hires) board::presentHires(); else board::present(gfx::fb, g_pal[g_shell.tint()]);
  mt[MT_UPDATE] = t2 - t1; mt[MT_RENDER] = t3 - t2; mt[MT_PRESENT] = micros() - t3;
  g_mtAt = (g_mtAt + 1) % MT_N;
  board::buzzer(g_shell.soundOn(ms));
  dimWhenIdle(ms);
  while (Serial.available()) serialCommand(Serial.read());
}
