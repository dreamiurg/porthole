// Every game against saves it did not write, handed over the way the shell does (App::enter): junk of every size,
// a real save cut short, and a real save with bytes changed and its crc put right, which is what another firmware's
// save or one out-of-range field looks like to a loader. Each visit then plays, draws, sounds and saves.
// `make ci` runs it once more as test_saves-asan: a table read past its end only shows under the sanitizers.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/apps.h"
#include "crc32.h"
#include "gfx565.h"
#include "host/fixtures/kingdom_save_v1.h"
#include "host/fixtures/times_cards_save_v3.h"

static int checks = 0;
static uint32_t rngState = 4242, g_ms = 1000;
static uint32_t rnd() { rngState ^= rngState << 13; rngState ^= rngState >> 17; rngState ^= rngState << 5; return rngState; }
static uint16_t g_fb[gfx565::W * gfx565::H];
static const uint32_t NOW = 1790000000u;

struct Blob { uint8_t d[shell::BLOB_MAX]; size_t n; };

static void keep(App* a, Blob* out) {
  const void* d; size_t len;
  if (!a->takeSave(&d, &len, true)) return;
  CHECK(len <= shell::BLOB_MAX);
  if (out && len) { memcpy(out->d, d, len); out->n = len; }
}
static void frame(App* a, InputTracker& t, int f) {
  static bool down = false; static int x = 80, y = 80;
  g_ms += 33;
  if (rnd() % 3 == 0) { down = !down; x = rnd() % 8 ? rnd() % 160 : 80; y = x == 80 ? 12 : rnd() % 160; }
  Input in = t.step(down, x, y, g_ms);
  in.gx = (int16_t)(rnd() % 2001 - 1000); in.gy = (int16_t)(rnd() % 2001 - 1000); in.gz = -500;
  a->update(NOW + f / 30, g_ms, in);
  a->wantsHome();
  if (a->surface() == SURFACE_RGB565) gfx565::target(g_fb);
  a->render();
  a->tint(); a->asleep(); a->soundOn(g_ms); a->screenName();
}
// One visit: all four profiles' slots hold the blob (some games read the others'), one of them plays.
static void visit(App* a, const Blob& b, int frames, Blob* saved = nullptr) {
  Profile all[4]; SaveSlot saves[4];
  for (int i = 0; i < 4; i++) {
    all[i] = {(uint8_t)i, "Sam", (uint8_t)i, (uint8_t)(5 + i * 2), 0};
    saves[i] = {b.d, b.n};
  }
  uint32_t stars = 50;
  a->enter({&all[rnd() % 4], all, saves, 4, NOW, g_ms, false, &stars});
  InputTracker t;
  for (int f = 0; f < frames; f++) {
    frame(a, t, f);
    if (f % 10 == 9) keep(a, saved);
  }
  a->leave();
  keep(a, saved);
}

template <typename T, typename Seal> static void put(Blob& out, T s, Seal seal) { seal(s); memcpy(out.d, &s, sizeof s); out.n = sizeof s; }
// A save the game would accept: built from its header where a few seconds of random play do not write one.
static void base(App* a, Blob& out) {
  const char* ns = a->store();
  out.n = 0;
  if (!strcmp(ns, "crago")) { Save s; pet::adopt(s, NOW - 86400 * 40, "Sam", "Rex"); put(out, s, pet::seal); }
  if (!strcmp(ns, biscuit::STORE)) put(out, biscuit::create(NOW - 86400), biscuit::seal);
  if (!strcmp(ns, marble::STORE)) put(out, marble::Save{}, marble::seal);
  if (!strcmp(ns, fc::STORE)) put(out, fc::Save{}, fc::seal);
  if (!strcmp(ns, "radar")) put(out, radar::Save{}, radar::seal);
  if (!strcmp(ns, "racing")) put(out, racing::Save{}, racing::seal);
  if (!strcmp(ns, "toytown")) put(out, toytown::fresh(), toytown::seal);
  if (!out.n) visit(a, out, 300, &out);
}

static void junk(App* a) {
  static const size_t SIZES[] = {1, 3, 4, 8, 12, 16, 20, 44, 48, 52, 100, 184, 212, 520, 935, 1020, 2048};
  for (size_t n : SIZES) for (int kind = 0; kind < 3; kind++) {
    Blob b; b.n = n;
    for (size_t i = 0; i < n; i++) b.d[i] = kind == 0 ? 0 : kind == 1 ? 0xFF : (uint8_t)rnd();
    visit(a, b, 10);
  }
}
// Every save here ends in a crc32 of all that comes before it, after an 8-byte magic and version.
static void mutated(App* a, const Blob& b0, int trials) {
  uint32_t crc; memcpy(&crc, b0.d + b0.n - 4, 4);
  CHECK(crc == os::crc32(b0.d, b0.n - 4));
  for (size_t n = 0; n <= b0.n + 8; n += b0.n / 24 + 1) { Blob b = b0; b.n = n; visit(a, b, 6); }   // cut short, and too long
  static const uint8_t EDGE[] = {0xFF, 0x80, 0x7F, 0xFE, 0x00, 0x01, 0x40, 0x20};
  const size_t body = b0.n - 12;
  for (int t = 0; t < trials; t++) {
    Blob b = b0;
    const size_t count = t % 5 ? (size_t)1 << (t % 5) : body;   // 2, 4, 8 or 16 bytes, or every one
    for (size_t i = 0; i < count; i++) b.d[8 + (t % 5 ? rnd() % body : i)] = rnd() % 2 ? EDGE[rnd() % 8] : (uint8_t)rnd();
    crc = os::crc32(b.d, b.n - 4); memcpy(b.d + b.n - 4, &crc, 4);
    visit(a, b, 40);
  }
}

// A game that packs its save now still reads the whole struct it stored before: the same treatment for that version.
static void older(App* a) {
  Blob b; b.n = 0;
  if (!strcmp(a->store(), cards::STORE)) { b.n = sizeof TIMES_CARDS_SAVE_V3; memcpy(b.d, TIMES_CARDS_SAVE_V3, b.n); }
  if (!strcmp(a->store(), "kingdom")) { b.n = sizeof KINGDOM_SAVE_V1; memcpy(b.d, KINGDOM_SAVE_V1, b.n); }
  if (b.n) mutated(a, b, 40);
}

int main() {
  gfx565::target(g_fb);
  int saving = 0;
  for (int k = 0; k < N_APPS; k++) {
    Blob b0;
    base(APPS[k], b0);
    junk(APPS[k]);
    if (b0.n < 12) continue;
    saving++;
    mutated(APPS[k], b0, 40);
    older(APPS[k]);
  }
  CHECK(saving == 9);   // a game that starts to save gets a base() here, or proof that random play writes one
  printf("test_saves: %d checks passed\n", checks);
  return 0;
}
