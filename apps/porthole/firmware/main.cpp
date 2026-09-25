// Pets Club firmware entry point.
#include <Arduino.h>
#include "board.h"
#include "game.h"

static Game g_game;
static InputTracker g_input;
static uint16_t g_pal[TINT_COUNT][C_COUNT];
static uint32_t g_lastSaveMs = 0, g_lastTouchMs = 0;
static uint8_t g_backlight = 100;
static bool g_touchLog = false;
static uint32_t g_bootLocalEpoch = 0, g_bootMillis = 0;
// Pets Club's NVS namespace, one "s<n>" key per house. Never rename: it orphans every save on a device.
static const char* const NS = "crago";
static const char* slotKey(int slot) { static char k[4]; snprintf(k, sizeof k, "s%d", slot); return k; }

// Wall clock: RTC if it runs, else continue from the last save so the pet's day count keeps going.
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

static void playSound(int id) { g_game.platformSoundStart(id); }

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("\n[pets-club] boot");
  board::init();
  for (int t = 0; t < TINT_COUNT; t++) {
    uint32_t rgb[C_COUNT]; palette_build((Tint)t, rgb);
    for (int i = 0; i < C_COUNT; i++) g_pal[t][i] = rgb888_to_565(rgb[i]);
  }
  Save houses[MAX_HOUSES]; int nHouses = 0; uint8_t blob[256]; uint32_t lastSeen = 0;
  for (int slot = 0; slot < MAX_HOUSES; slot++) {
    size_t got = board::loadBlob(NS, slotKey(slot), blob, sizeof blob);
    if (got && pet::loadBlob(blob, got, houses[nHouses])) nHouses++;
  }
  if (nHouses == 0) {  // first boot after the multi-house update: adopt the old single "save" key as house 0
    size_t got = board::loadBlob(NS, "save", blob, sizeof blob);
    if (got && pet::loadBlob(blob, got, houses[0])) { nHouses = 1; board::saveBlob(NS, slotKey(0), &houses[0], sizeof(Save)); Serial.println("[pets-club] migrated save -> s0"); }
  }
  for (int i = 0; i < nHouses; i++) if (houses[i].lastSeen > lastSeen) lastSeen = houses[i].lastSeen;
  uint32_t now;
  if (board::rtcValid()) now = board::rtcNow();
  else {
    now = nHouses ? lastSeen + 60 : buildEpoch();
    if (now < buildEpoch()) now = buildEpoch();
    board::rtcSet(now);
    Serial.println("[pets-club] RTC was not running; clock restored");
  }
  g_bootLocalEpoch = now; g_bootMillis = millis();
  Serial.printf("[pets-club] houses=%d now=%lu heap=%lu\n", nHouses, (unsigned long)now, (unsigned long)board::freeHeap());
  g_game.begin(now, millis(), houses, nHouses);
  g_lastTouchMs = millis();
}

void loop() {
  uint32_t ms = millis();
  board::Touch t = board::readTouch();
  static bool swallow = false;  // the touch that wakes a dark screen is not a game input, until released
  if (t.down) {
    if (g_backlight == 0) swallow = true;
    g_lastTouchMs = ms;
  } else swallow = false;
  Input in = g_input.step(t.down && !swallow, t.x / 3, t.y / 3, ms);
  if (in.pressed && g_touchLog) Serial.printf("[touch] %d,%d\n", t.x, t.y);
  g_game.update(nowSec(), ms, in);
  g_game.render();
  board::present(gfx::fb, g_pal[g_game.tint()]);

  // sound: simple on/off pattern player for the active buzzer
  board::buzzer(g_game.soundOn(ms));

  // save when the game asks, at most once per 5 s per house
  Save out; int slot;
  if (g_game.takeSave(&out, &slot, ms - g_lastSaveMs > 5000)) { board::saveBlob(NS, slotKey(slot), &out, sizeof out); g_lastSaveMs = ms; }
  if (g_game.takeErase(&slot)) board::eraseBlob(NS, slotKey(slot));

  // idle dimming (no physical buttons: the screen is the only power control)
  uint32_t idle = ms - g_lastTouchMs;
  uint8_t want = g_game.asleep() ? (idle > 20000 ? 0 : 40) : (idle > 300000 ? 0 : idle > 60000 ? 30 : 100);
  if (want != g_backlight) { g_backlight = want; board::setBacklight(want); }

  // serial maintenance: "T<epoch>" sets the clock (local wall-clock seconds), "R" wipes every house, "S" prints stats,
  // "D" toggles touch logging, "P<n>" clears house n's secret code (a parent's escape hatch)
  while (Serial.available()) {
    int c = Serial.read();
    if (c == 'T') { uint32_t e = (uint32_t)Serial.parseInt(); if (e > 1600000000u) { board::rtcSet(e); g_bootLocalEpoch = e; g_bootMillis = millis(); Serial.println("[pets-club] clock set"); } }
    else if (c == 'R') { board::eraseAll(); Serial.println("[pets-club] all houses erased, rebooting"); delay(100); ESP.restart(); }
    else if (c == 'P') { int n = Serial.parseInt(); g_game.clearPin(n); Serial.printf("[pets-club] house %d code cleared\n", n); }
    else if (c == 'S') { g_game.debugPrint(); Serial.printf("heap=%lu now=%lu\n", (unsigned long)board::freeHeap(), (unsigned long)nowSec()); }
    else if (c == 'D') { g_touchLog = !g_touchLog; Serial.printf("[pets-club] touch log %s\n", g_touchLog ? "on" : "off"); }
  }
  (void)playSound;
}
