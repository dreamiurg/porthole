// Biscuit's Home skips drawing a frame its target buffer already holds (canvas::Frames with a look word). Drive Home
// through presses, fetch, naps and the scene's steps over the panel's two buffers, and check every frame against a
// whole repaint: a look that misses something Home draws shows up here as a differing frame. Run: make test
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "games/biscuit/game.h"
#include "games/biscuit/layout.h"
#include "gfx565.h"
#include "input.h"

static bool drawnRight(biscuit::Game& game, uint16_t* shown, uint16_t* whole) {
  gfx565::target(shown);
  game.render();
  biscuit::Game copy = game;   // a copy knows neither buffer of `whole`: it draws the whole frame
  gfx565::target(whole);
  copy.render();
  const bool same = !memcmp(shown, whole, gfx565::W * gfx565::H * 2);
  memcpy(shown, whole, gfx565::W * gfx565::H * 2);   // report each mistake once
  return same;
}

int main() {
  static uint16_t bufs[2][gfx565::W * gfx565::H], whole[gfx565::W * gfx565::H];
  for (auto& b : bufs) for (uint16_t& p : b) p = 0xF81F;
  static biscuit::Game game;
  const uint32_t now = 1790000000u;   // a weekday, 16:00 local
  biscuit::Save save = biscuit::create(now);
  biscuit::setPetName(save, "Pip", now);
  biscuit::seal(save);
  const Profile kid = {0, "Sam", 0, 7, 0};
  const SaveSlot slot = {&save, sizeof save};
  uint32_t ms = 1000;
  game.enter({&kid, &kid, &slot, 1, now, ms, false});
  InputTracker tracker;
  // Where a finger goes (logical px): the pup, Feed, Play, each fetch ball, the window (a nap and back), nowhere.
  const biscuit::Box& pup = biscuit::PUP;
  const int spots[][2] = {{pup.x + 20, pup.y + 20}, {40, 129}, {67, 129}, {47, 70}, {113, 78}, {79, 102}, {45, 105},
                          {111, 106}, {126, 62}, {126, 62}, {10, 10}};
  int bad = 0, frames = 0, home = 0;
  for (int f = 0; f < 3000; f++, frames++) {
    ms += f % 7 ? 17 : 120;   // mostly a frame, sometimes a stall: the scene steps at odd moments
    const int phase = f % 60, spot = f / 60 % (int)(sizeof spots / sizeof spots[0]);
    const bool down = phase >= 20 && phase < 24;   // a short press, held for a few frames
    Input in = tracker.step(down, spots[spot][0], spots[spot][1], ms);
    game.update(now + ms / 1000, ms, in);
    if (game.wantsHome()) { game.leave(); game.enter({&kid, &kid, &slot, 1, now + ms / 1000, ms, false}); }
    if (game.surface() != SURFACE_RGB565) continue;
    home += !strcmp(game.screenName(), "biscuit_home");
    if (!drawnRight(game, bufs[f & 1], whole) && bad++ < 5) printf("frame %d on %s differs from a whole repaint\n", f, game.screenName());
  }
  printf("biscuit home: %d of %d frames differ (%d on Home)\n", bad, frames, home);
  assert(!bad && home > 1000);
  return 0;
}
