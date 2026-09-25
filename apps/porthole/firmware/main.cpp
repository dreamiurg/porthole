// Porthole firmware entry point: the shell (profiles, launcher) hosting the games in APPS.
#include <Arduino.h>
#include "board.h"
#include "game.h"
#include "shell.h"

// The shell's storage is the board's NVS, addressed by (namespace, key).
struct NvsStore : shell::Store {
  size_t load(const char* ns, const char* key, void* buf, size_t max) override { return board::loadBlob(ns, key, buf, max); }
  void save(const char* ns, const char* key, const void* data, size_t len) override { board::saveBlob(ns, key, data, len); }
  void erase(const char* ns, const char* key) override { board::eraseBlob(ns, key); }
};

static Game g_pets;
static App* const APPS[] = {&g_pets};
static const int N_APPS = sizeof APPS / sizeof APPS[0];
static NvsStore g_store;
static Shell g_shell;
static InputTracker g_input;
static uint16_t g_pal[TINT_COUNT][C_COUNT];
static uint32_t g_lastTouchMs = 0;
static uint8_t g_backlight = 100;
static bool g_touchLog = false;
static uint32_t g_bootLocalEpoch = 0, g_bootMillis = 0;
// Test harness (tools/device.py): a synthetic finger held at logical (x, y) until `until`, then released.
static struct { bool on; int x, y; uint32_t until; } g_fake = {};

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
    Serial.println("[porthole] RTC was not running; clock restored");
  }
  g_bootLocalEpoch = now; g_bootMillis = millis();
  Serial.printf("[porthole] profiles=%d now=%lu heap=%lu\n", g_shell.profileCount(), (unsigned long)now, (unsigned long)board::freeHeap());
  g_lastTouchMs = millis();
}

// Idle dimming (no physical buttons: the screen is the only power control).
static void dimWhenIdle(uint32_t ms) {
  uint32_t idle = ms - g_lastTouchMs;
  uint8_t want = g_shell.asleep() ? (idle > 20000 ? 0 : 40) : (idle > 300000 ? 0 : idle > 60000 ? 30 : 100);
  if (want != g_backlight) { g_backlight = want; board::setBacklight(want); }
}

// Serial maintenance: "T<epoch>" sets the clock (local wall-clock seconds), "R" wipes every profile and every game's
// saves, "S" prints stats, "D" toggles touch logging, "P<n>" clears profile n's secret code (a parent's escape hatch),
// "X<x>,<y>,<ms>" presses the screen and "F" dumps the frame (both for tools/device.py).
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
  else if (c == 'F') {  // F: dump the frame as "FB <w> <h>\n", w*h palette indices, then that tint's 32 RGB565 colours
    Serial.printf("FB %d %d\n", gfx::W, gfx::H);
    Serial.write(gfx::fb, gfx::W * gfx::H);
    Serial.write((const uint8_t*)g_pal[g_shell.tint()], sizeof g_pal[0]);
    Serial.flush();
  }
  else if (c == 'D') { g_touchLog = !g_touchLog; Serial.printf("[porthole] touch log %s\n", g_touchLog ? "on" : "off"); }
}

void loop() {
  uint32_t ms = millis();
  board::Touch t = board::readTouch();
  if (g_fake.on) {  // a harness press behaves like a finger (physical px), including waking the screen
    t = {ms < g_fake.until, g_fake.x * 3, g_fake.y * 3};
    if (!t.down) g_fake.on = false;
  }
  static bool swallow = false;  // the touch that wakes a dark screen is not a game input, until released
  if (t.down) {
    if (g_backlight == 0) swallow = true;
    g_lastTouchMs = ms;
  } else swallow = false;
  Input in = g_input.step(t.down && !swallow, t.x / 3, t.y / 3, ms);
  if (in.pressed && g_touchLog) Serial.printf("[touch] %d,%d\n", t.x, t.y);
  g_shell.update(nowSec(), ms, in);   // also saves: the open game at most every 5 s, profiles when they change
  g_shell.render();
  board::present(gfx::fb, g_pal[g_shell.tint()]);
  board::buzzer(g_shell.soundOn(ms));
  dimWhenIdle(ms);
  while (Serial.available()) serialCommand(Serial.read());
}
