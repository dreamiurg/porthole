// Self-check of the App contract (os/app.h) for every game in games/apps.h, so a new game is held to it without writing
// a test: a store name the device's key store takes, an RGB565 render() that paints the whole round screen when it
// cannot know what the buffers hold, and a save the shell can keep and hand back. Run: make test
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include "games/apps.h"
#include "gfx565.h"
#include "profiles.h"

static const uint16_t UNKNOWN = 0xF81F;   // the sim's magenta: a pixel no render() has written
static uint16_t g_bufs[2][gfx565::W * gfx565::H];
static bool g_unknown[2];
static uint32_t g_rs;
static uint32_t rnd() { g_rs ^= g_rs << 13; g_rs ^= g_rs >> 17; g_rs ^= g_rs << 5; return g_rs; }

static void forget() {   // what enter() and an indexed present leave: both panel buffers unknown
  for (auto& b : g_bufs) for (uint16_t& p : b) p = UNKNOWN;
  g_unknown[0] = g_unknown[1] = true;
}

// One present. An RGB565 render() into a buffer it cannot know must leave no pixel of the round screen unwritten.
static void present(App& app, int f) {
  if (app.surface() != SURFACE_RGB565) { app.render(); forget(); return; }
  uint16_t* shown = g_bufs[f & 1];
  gfx565::target(shown);
  app.render();
  if (!g_unknown[f & 1]) return;
  g_unknown[f & 1] = false;
  for (int y = 0; y < gfx565::H; y++)
    for (int x = 0; x < gfx565::W; x++)
      if (shown[y * gfx565::W + x] == UNKNOWN && gfx565::inCircle(x, y)) {
        printf("%s, screen %s, frame %d: render() left %d,%d unpainted\n", app.name(), app.screenName(), f, x, y);
        assert(false);
      }
}

// A stay in the game under seeded taps, holds and tilts; a game that asks for home is left and entered again.
static void play(App& app, const AppEnter& e, int frames) {
  InputTracker tracker;
  uint32_t ms = e.ms;
  int hold = 0, tx = 80, ty = 80;
  app.enter(e);
  forget();
  for (int f = 0; f < frames; f++) {
    ms += 17;
    if (hold > 0) hold--;
    else if (rnd() % 20 == 0) { hold = 1 + (int)(rnd() % 6); tx = 20 + (int)(rnd() % 120); ty = 8 + (int)(rnd() % 144); }
    Input in = tracker.step(hold > 0, tx, ty, ms);
    in.gx = (int16_t)((int)(rnd() % 600) - 300); in.gy = (int16_t)((int)(rnd() % 600) - 300); in.gz = -900;
    app.update(e.nowSec + (ms - e.ms) / 1000, ms, in);
    if (app.wantsHome()) { app.leave(); app.enter(e); forget(); tracker = InputTracker(); continue; }
    present(app, f);
  }
  app.leave();
}

int main() {
  const Profile kid = {0, "Sam", 0, 7, 0};
  static uint8_t blob[shell::BLOB_MAX];
  for (int i = 0; i < N_APPS; i++) {
    App& app = *APPS[i];
    const size_t ns = strlen(app.store());
    assert(strlen(app.name()) > 0 && ns > 0 && ns <= 15);   // NVS takes a namespace of at most 15 characters
    for (int j = 0; j < i; j++) assert(strcmp(app.store(), APPS[j]->store()) != 0);   // or two games share saves
    g_rs = 777;
    uint32_t stars = 50;
    const SaveSlot none = {nullptr, 0};
    play(app, {&kid, &kid, &none, 1, 1790000000u, 1000, false, &stars}, 400);
    const void* data = nullptr; size_t len = 0;
    SaveSlot kept = {nullptr, 0};
    if (app.takeSave(&data, &len, true)) {
      assert(len <= shell::BLOB_MAX);   // the shell drops a larger save without a word (shell.cpp, Shell::flushApp)
      memcpy(blob, data, len);
      kept = {blob, len};
    }
    play(app, {&kid, &kid, &kept, 1, 1790000100u, 20000, false, &stars}, 200);   // back in, with what it saved
  }
  printf("test_app_contract: %d games hold the App contract\n", N_APPS);
  return 0;
}
