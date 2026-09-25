// Host simulator. With HAVE_SDL: a window (mouse = finger). Always: a scripted headless mode that
// writes BMP snapshots, used to eyeball screens without hardware. Also a --serve mode (stdin/stdout
// protocol) that tools/webemu.py drives to serve the game in a browser.
//   sim                      interactive (SDL)
//   sim --script file.txt    headless; commands: tap X Y | hold X Y | down X Y | move X Y | up | wait MS | skip SEC | snap name | reset
//                            | newgame KID PET | debug | dbg CMD | ui | screen | echo WORD | watch MS | monkey N SEED
//   sim --serve              headless; stdin commands: down X Y | move X Y | up | tick MS | skip SEC | reset | frame
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "game.h"
#ifdef HAVE_SDL
#include <SDL.h>
#endif

static Game g_game;
static InputTracker g_tracker;
static uint32_t g_ms = 0, g_epoch = 0;
static uint32_t g_pal[TINT_COUNT][C_COUNT];
static const char* savePath(int slot) { static char p[64]; snprintf(p, sizeof p, "build/host/pets-club%d.sav", slot); return p; }
static int loadSaves(Save* out) {
  int n = 0; uint8_t blob[256];
  for (int slot = 0; slot < MAX_HOUSES; slot++) {
    FILE* f = fopen(savePath(slot), "rb"); if (!f) continue;
    size_t got = fread(blob, 1, sizeof blob, f); fclose(f);
    if (got && pet::loadBlob(blob, got, out[n])) n++;
  }
  return n;
}
static void storeSave(int slot, const Save& s) { FILE* f = fopen(savePath(slot), "wb"); if (f) { fwrite(&s, 1, sizeof s, f); fclose(f); } }
static void removeSaves() { for (int slot = 0; slot < MAX_HOUSES; slot++) remove(savePath(slot)); }

static void writeBMP(const char* path) {
  const int S = 3, W = gfx::W * S, H = gfx::H * S;
  FILE* f = fopen(path, "wb"); if (!f) { perror(path); return; }
  uint32_t rowBytes = (uint32_t)W * 3, fileSize = 54 + rowBytes * (uint32_t)H;
  uint8_t hdr[54] = {'B', 'M'}; memcpy(hdr + 2, &fileSize, 4); uint32_t off = 54; memcpy(hdr + 10, &off, 4);
  uint32_t dib = 40; memcpy(hdr + 14, &dib, 4); int32_t w = W, h = H; memcpy(hdr + 18, &w, 4); memcpy(hdr + 22, &h, 4);
  uint16_t planes = 1, bpp = 24; memcpy(hdr + 26, &planes, 2); memcpy(hdr + 28, &bpp, 2);
  fwrite(hdr, 1, 54, f);
  const uint32_t* pal = g_pal[g_game.tint()];
  std::vector<uint8_t> row(rowBytes);
  for (int y = H - 1; y >= 0; y--) {
    for (int x = 0; x < W; x++) {
      int lx = x / S, ly = y / S; int dx = x - 240, dy = y - 240;
      uint32_t c = dx * dx + dy * dy <= 240 * 240 ? pal[gfx::fb[ly * gfx::W + lx]] : 0x202020;  // outside the round glass: dark
      row[x * 3 + 0] = c & 255; row[x * 3 + 1] = (c >> 8) & 255; row[x * 3 + 2] = (c >> 16) & 255;
    }
    fwrite(row.data(), 1, rowBytes, f);
  }
  fclose(f);
}

static void frame(bool down, int x, int y) {
  Input in = g_tracker.step(down, x, y, g_ms);
  g_game.update(g_epoch + g_ms / 1000, g_ms, in);
  g_game.render();
  Save s; int slot;
  while (g_game.takeSave(&s, &slot, true)) storeSave(slot, s);
  while (g_game.takeErase(&slot)) remove(savePath(slot));
}

// Same upscale+round-mask+tint as writeBMP, but raw RGB triples (PPM order) to stdout.
static void writePPM() {
  const int S = 3, W = gfx::W * S, H = gfx::H * S;
  const uint32_t* pal = g_pal[g_game.tint()];
  std::vector<uint8_t> buf((size_t)W * H * 3);
  size_t i = 0;
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      int lx = x / S, ly = y / S; int dx = x - 240, dy = y - 240;
      uint32_t c = dx * dx + dy * dy <= 240 * 240 ? pal[gfx::fb[ly * gfx::W + lx]] : 0x202020;  // outside the round glass: dark
      buf[i++] = (c >> 16) & 255; buf[i++] = (c >> 8) & 255; buf[i++] = c & 255;
    }
  }
  printf("PPM %zu\n", buf.size());
  fwrite(buf.data(), 1, buf.size(), stdout);
  fflush(stdout);
}

// --serve mode: newline-delimited commands on stdin, one response on stdout per command (flushed).
// down/move/up/tick/skip/reset all print "ok"; frame prints a binary PPM (P6) preceded by "PPM <bytes>".
static void runServe() {
  bool down = false; int cx = 0, cy = 0;
  char line[256];
  while (fgets(line, sizeof line, stdin)) {
    char cmd[32] = {0}; char a[64] = {0}, b[64] = {0};
    if (sscanf(line, "%31s %63s %63s", cmd, a, b) < 1) continue;
    if (!strcmp(cmd, "down")) { down = true; cx = atoi(a); cy = atoi(b); frame(true, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "move")) { cx = atoi(a); cy = atoi(b); frame(down, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "up")) { down = false; frame(false, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "tick")) { int ms = atoi(a); for (int t = 0; t < ms; t += 33) { frame(down, cx, cy); g_ms += 33; } puts("ok"); }
    else if (!strcmp(cmd, "skip")) { g_epoch += (uint32_t)atol(a); frame(down, cx, cy); g_ms += 33; puts("ok"); }
    else if (!strcmp(cmd, "reset")) { removeSaves(); down = false; g_game.begin(g_epoch + g_ms / 1000, g_ms, nullptr, 0); puts("ok"); }
    else if (!strcmp(cmd, "frame")) writePPM();
    else if (!strcmp(cmd, "dbg")) { g_game.debugCmd(a); puts("ok"); }
    else puts("ok");
    fflush(stdout);
  }
}

// ---- playtest helpers (tools/playtest.py). step() advances one 40 ms frame exactly like `wait`, so runs replay identically.
static void step(bool down, int x, int y) { frame(down, x, y); g_ms += 40; }
static uint32_t g_rng = 1;  // own LCG: a monkey seed replays the same gestures on every platform
static int mrand(int n) { g_rng = g_rng * 1664525u + 1013904223u; return (int)((g_rng >> 8) % (uint32_t)n); }
static void monkeyPoint(int& x, int& y) { do { x = 2 + mrand(157); y = 2 + mrand(157); } while ((x - 80) * (x - 80) + (y - 80) * (y - 80) > 78 * 78); }
static void press(int x, int y, int ms) { for (int t = 0; t < ms; t += 40) step(true, x, y); step(false, x, y); }  // 80 ms = the script `tap`
// Chaos monkey: a kid mashing the glass. 65% taps, 10% double taps, 10% drags, 10% slow taps (30-200 ms), 5% long presses.
static void monkey(int n, uint32_t seed) {
  g_rng = seed;
  for (int i = 1; i <= n; i++) {
    int x, y, r = mrand(100); monkeyPoint(x, y);
    if (r < 65) press(x, y, 80);
    else if (r < 75) { press(x, y, 80); press(x, y, 80); }  // double tap: the second press starts 120 ms after the first
    else if (r < 85) {
      int ex, ey, k = 4 + mrand(5); monkeyPoint(ex, ey);
      step(true, x, y); for (int j = 1; j <= k; j++) step(true, x + (ex - x) * j / k, y + (ey - y) * j / k); step(false, ex, ey);
    }
    else if (r < 95) press(x, y, 30 + mrand(171));
    else press(x, y, 700);
    for (int t = 0, w = 40 + mrand(461); t < w; t += 40) step(false, x, y);
    if (i % 100 == 0) printf("monkey %d screen=%s\n", i, g_game.screenName());
  }
  puts("monkey done");
}
// Steps frames like `wait` and reports each trick-lesson glow: the orange (C_ORANGE) ring at the lit zone. Rows above
// y=26 are skipped (the orange home button); a dozen pixels minimum so stray confetti does not count.
static void watch(int ms, bool down, int x, int y) {
  bool was = false;
  for (int t = 0; t < ms; t += 40) {
    step(down, x, y);
    long sx = 0, sy = 0, n = 0;
    for (int py = 26; py < gfx::H; py++) for (int px = 0; px < gfx::W; px++) if (gfx::fb[py * gfx::W + px] == C_ORANGE) { sx += px; sy += py; n++; }
    if (n >= 12 && !was) printf("glow %ld %ld\n", (sx + n / 2) / n, (sy + n / 2) / n);
    was = n >= 12;
  }
}

static void runScript(const char* path) {
  FILE* f = fopen(path, "r"); if (!f) { perror(path); exit(1); }
  setvbuf(stdout, nullptr, _IOLBF, 0);  // keep every line written before a sanitizer abort
  char line[256]; bool down = false; int cx = 0, cy = 0;
  while (fgets(line, sizeof line, f)) {
    char cmd[32] = {0}; char a[64] = {0}, b[64] = {0};
    if (sscanf(line, "%31s %63s %63s", cmd, a, b) < 1 || cmd[0] == '#') continue;
    if (!strcmp(cmd, "tap")) { int x = atoi(a), y = atoi(b); frame(true, x, y); g_ms += 40; frame(true, x, y); g_ms += 40; frame(false, x, y); g_ms += 40; }
    else if (!strcmp(cmd, "hold")) { int x = atoi(a), y = atoi(b); for (int i = 0; i < 25; i++) { frame(true, x, y); g_ms += 40; } frame(false, x, y); g_ms += 40; }
    else if (!strcmp(cmd, "down")) { down = true; cx = atoi(a); cy = atoi(b); frame(true, cx, cy); g_ms += 40; }
    else if (!strcmp(cmd, "move")) { cx = atoi(a); cy = atoi(b); frame(down, cx, cy); g_ms += 40; }
    else if (!strcmp(cmd, "up")) { down = false; frame(false, cx, cy); g_ms += 40; }
    else if (!strcmp(cmd, "wait")) { int ms = atoi(a); for (int t = 0; t < ms; t += 40) { frame(down, cx, cy); g_ms += 40; } }
    else if (!strcmp(cmd, "skip")) { g_epoch += (uint32_t)atol(a); frame(down, cx, cy); g_ms += 40; }
    else if (!strcmp(cmd, "snap")) { char p[256]; snprintf(p, sizeof p, "build/host/%s.bmp", a[0] ? a : "snap"); frame(down, cx, cy); writeBMP(p); printf("wrote %s\n", p); }
    else if (!strcmp(cmd, "reset")) { removeSaves(); g_game.begin(g_epoch + g_ms / 1000, g_ms, nullptr, 0); }
    else if (!strcmp(cmd, "newgame")) { Save s; pet::adopt(s, g_epoch + g_ms / 1000, a, b); s.kidAge = 8; pet::seal(s); removeSaves(); storeSave(0, s); g_game.begin(g_epoch + g_ms / 1000, g_ms, &s, 1); }
    else if (!strcmp(cmd, "debug")) { g_game.debugPrint(); }
    else if (!strcmp(cmd, "ui")) {  // one audited frame: every hit region and text box the game touched
      UiAudit::enabled = true; gfx::textLogEnabled = true; UiAudit::reset(); gfx::textLogReset();
      frame(down, cx, cy); g_ms += 40;
      printf("ui screen=%s regions=%d texts=%d\n", g_game.screenName(), UiAudit::count, gfx::textLogCount);
      for (int i = 0; i < UiAudit::count; i++) printf("region %d %d %d %d\n", UiAudit::regions[i].x, UiAudit::regions[i].y, UiAudit::regions[i].w, UiAudit::regions[i].h);
      for (int i = 0; i < gfx::textLogCount; i++) printf("text %d %d %d %d color=%d bg=%d\n", gfx::textLog[i].x, gfx::textLog[i].y, gfx::textLog[i].w, gfx::textLog[i].h, gfx::textLog[i].color, gfx::textLog[i].bg);
      UiAudit::enabled = false; gfx::textLogEnabled = false;
    }
    else if (!strcmp(cmd, "dbg")) { g_game.debugCmd(a); }
    else if (!strcmp(cmd, "screen")) { printf("screen=%s\n", g_game.screenName()); }
    else if (!strcmp(cmd, "echo")) { puts(a); }
    else if (!strcmp(cmd, "watch")) { watch(atoi(a), down, cx, cy); }
    else if (!strcmp(cmd, "monkey")) { monkey(atoi(a), (uint32_t)strtoul(b, nullptr, 10)); down = false; }
    else fprintf(stderr, "unknown command: %s\n", cmd);
  }
  fclose(f);
}

int main(int argc, char** argv) {
  for (int t = 0; t < TINT_COUNT; t++) palette_build((Tint)t, g_pal[t]);
  // Default clock: a fixed Tuesday 16:00 local so snapshots are deterministic; --now overrides.
  g_epoch = 1790000000u - (1790000000u % 86400u) + 16 * 3600;
  const char* script = nullptr; bool serve = false;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--script") && i + 1 < argc) script = argv[++i];
    else if (!strcmp(argv[i], "--serve")) serve = true;
    else if (!strcmp(argv[i], "--now") && i + 1 < argc) g_epoch = (uint32_t)atol(argv[++i]);
    else if (!strcmp(argv[i], "--fresh")) removeSaves();
  }
  Save houses[MAX_HOUSES]; int n = loadSaves(houses);
  g_game.begin(g_epoch, g_ms, houses, n);
  if (script) { runScript(script); return 0; }
  if (serve) { runServe(); return 0; }
#ifdef HAVE_SDL
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* win = SDL_CreateWindow("Pets Club", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480, 480, SDL_WINDOW_ALLOW_HIGHDPI);
  SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, gfx::W, gfx::H);
  bool run = true, down = false; int mx = 0, my = 0; uint32_t start = SDL_GetTicks();
  static uint32_t pix[gfx::W * gfx::H];
  while (run) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) run = false;
      if (e.type == SDL_MOUSEBUTTONDOWN) { down = true; mx = e.button.x / 3; my = e.button.y / 3; }
      if (e.type == SDL_MOUSEBUTTONUP) down = false;
      if (e.type == SDL_MOUSEMOTION) { mx = e.motion.x / 3; my = e.motion.y / 3; }
      if (e.type == SDL_KEYDOWN) {
        SDL_Keycode k = e.key.keysym.sym;
        if (k == SDLK_q || k == SDLK_ESCAPE) run = false;
        if (k == SDLK_h) g_epoch += 3600;            // +1 hour
        if (k == SDLK_d) g_epoch += 86400;           // +1 day
        if (k == SDLK_n) g_epoch += 8 * 3600;        // +8 hours (a night)
        if (k == SDLK_s) { writeBMP("build/host/shot.bmp"); printf("wrote build/host/shot.bmp\n"); }
        if (k == SDLK_r) { removeSaves(); g_game.begin(g_epoch + g_ms / 1000, g_ms, nullptr, 0); }
        if (k == SDLK_p) g_game.debugPrint();
      }
    }
    g_ms = SDL_GetTicks() - start;
    frame(down, mx, my);
    const uint32_t* pal = g_pal[g_game.tint()];
    for (int i = 0; i < gfx::W * gfx::H; i++) {
      int x = i % gfx::W - 80, y = i / gfx::W - 80;
      pix[i] = (x * x + y * y <= 80 * 80) ? (0xFF000000u | pal[gfx::fb[i]]) : 0xFF202020u;
    }
    SDL_UpdateTexture(tex, nullptr, pix, gfx::W * 4);
    SDL_RenderClear(ren); SDL_RenderCopy(ren, tex, nullptr, nullptr); SDL_RenderPresent(ren);
  }
  SDL_Quit();
#else
  fprintf(stderr, "built without SDL; use --script\n");
#endif
  return 0;
}
