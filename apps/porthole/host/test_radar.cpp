// Plane Radar's self-check: the scope geometry, tag placement, the save, and the pages driven through the App
// interface with a stub feed (menu, settings saved, slow presses, taps on the rim), every page drawn.
#include "host/check.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "games/plane-radar/game.h"
#include "games/plane-radar/scope.h"
#include "gfx565.h"

using namespace radar;
static int checks = 0;

static air::Plane at(float east, float north) {
  air::Plane p{};
  p.eastKm = east; p.northKm = north; p.knots = 300; p.altFt = 12345; p.kind = air::K_AIRLINER;
  snprintf(p.call, sizeof p.call, "TST1");
  return p;
}

static void geometry() {
  const int r = 2;   // 12 mi
  const Placed north = place(0, rangeKm(r) / 2, 0, r);
  CHECK(north.inside && north.x == CX && abs(north.y - (CY - RING / 2)) <= 1);
  const Placed southUp = place(0, rangeKm(r) / 2, 2, r);   // south at the top: north is straight down
  CHECK(southUp.inside && southUp.x == CX && abs(southUp.y - (CY + RING / 2)) <= 1);
  const Placed eastSouthUp = place(rangeKm(r) / 2, 0, 2, r);   // ...and east on the left
  CHECK(eastSouthUp.x < CX);
  const Placed far = place(rangeKm(r) * 3, 0, 0, r);   // past the ring: a rim dot at its bearing
  CHECK(!far.inside && abs(far.x - (CX + RIM)) <= 1 && far.y == CY);
  CHECK(fabsf(screenDeg(90, 2) - 270) < 0.01f && fabsf(screenDeg(370, 0) - 10) < 0.01f);
  CHECK(fetchKm(r) > rangeKm(r));   // the request reaches past the ring, for the rim dots
}

static void tags() {
  Tag t = tagFor({CX - 60, CY, 80, 40, -1}, {nullptr, 0});   // west of the middle: the tag goes on its right
  CHECK(t.ok && t.x > CX - 60);
  t = tagFor({CX + 60, CY, 80, 40, -1}, {nullptr, 0});       // east: on its left
  CHECK(t.ok && t.x + 80 < CX + 60);
  t = tagFor({CX + 200, CY, 80, 40, -1}, {nullptr, 0});      // near the east rim: the left side is the only one on the glass
  CHECK(t.ok && t.x + 80 < CX + 200);
  const canvas::Box block = {CX - 40, CY - 30, CX + 20, CY + 30};   // its right side taken: it moves elsewhere
  t = tagFor({CX - 60, CY, 30, 20, -1}, {&block, 1});
  CHECK(t.ok && !overlap({t.x, t.y, t.x + 30, t.y + 20}, block));
  const canvas::Box all = {0, 0, 480, 480};
  CHECK(!tagFor({CX, CY, 30, 20, -1}, {&all, 1}).ok);   // nowhere free: no tag
}

static void words() {
  char b[24];
  altText(12345, b, sizeof b); CHECK(!strcmp(b, "12300 ft"));
  altText(air::NO_ALT, b, sizeof b); CHECK(!b[0]);
  altText(-120, b, sizeof b); CHECK(!strcmp(b, "-100 ft"));
  rangeText(3, b, sizeof b); CHECK(!strcmp(b, "25 mi"));
  for (int s = 0; s < air::S_COUNT; s++) CHECK(s == air::S_LIVE ? !stateText((air::State)s)[0] : stateText((air::State)s)[0]);
  spotText(true, 47.621, -122.349, b, sizeof b); CHECK(!strcmp(b, "Home 47.62,-122.35"));
}

static void saves() {
  Save s{}; s.range = 3; s.up = 2; s.hideRoute = 1; seal(s);
  Save t{};
  CHECK(loadBlob(&s, sizeof s, t) && t.range == 3 && t.up == 2 && t.hideRoute == 1);
  Save bad = s; bad.crc ^= 1;
  Save u{}; u.range = 9;
  CHECK(!loadBlob(&bad, sizeof bad, u) && u.range == 9);   // a broken blob leaves the save alone
  CHECK(!loadBlob(&s, 8, u));
}

struct StubFeed : air::Feed {
  air::State state = air::S_LIVE; bool on = false; float km = 0;
  double lat = 0, lon = 0; bool has = false;
  void start(float r) override { on = true; km = r; }
  void stop() override { on = false; }
  void read(air::Snapshot& out) override {
    out = air::Snapshot{}; out.state = state; out.gen = 1;
    if (state != air::S_LIVE) return;
    out.planes[out.n++] = at(-5, 3);
    out.planes[out.n++] = at(4, -2);
    out.planes[out.n++] = at(60, 60);   // a rim dot
  }
  bool spot(double* a, double* b) override { *a = lat; *b = lon; return has; }
};

static uint16_t panel[gfx565::W * gfx565::H];
static uint32_t now = 1000;
static void frame(Game& g, Input in = Input{}) {
  g.update(0, now, in);
  gfx565::target(panel);
  g.render();
  now += 40;
}
static void tapAt(Game& g, int x, int y) {   // a finger down, then lifted: a tap
  Input in; in.down = in.pressed = true; in.x = in.downX = x; in.y = in.downY = y;
  frame(g, in);
  in.down = in.pressed = false; in.released = in.tap = true;
  frame(g, in);
}
static void tapBox(Game& g, const ui::Box& b) { tapAt(g, b.x + b.w / 2, b.y + b.h / 2); }
static void settle(Game& g) { for (int i = 0; i < 15; i++) frame(g); }   // past the fresh-screen tap gate

static void pages() {
  StubFeed feed;
  air::setFeed(&feed);
  Game g;
  Profile who = {1, "Sam", 0, 7, 0};
  AppEnter e = {&who, &who, nullptr, 0, 0, now, true};
  SaveSlot none = {nullptr, 0};
  e.saves = &none; e.n = 1;
  g.enter(e);
  settle(g);
  CHECK(feed.on && !strcmp(g.screenName(), "radar_live") && g.save().range == START_RANGE);
  tapAt(g, 60, 60);   // the scope: the next range
  CHECK(g.save().range == START_RANGE + 1 && fabsf(feed.km - fetchKm(START_RANGE + 1)) < 0.01f);
  const void* data; size_t len;
  CHECK(g.takeSave(&data, &len, true) && len == sizeof(Save));
  CHECK(!g.takeSave(&data, &len, true));   // nothing new
  tapAt(g, paint::MENU_CX, paint::MENU_CY);
  CHECK(!strcmp(g.screenName(), "radar_menu"));
  settle(g);
  tapBox(g, paint::upBox(2)); tapBox(g, paint::rangeBox(0)); tapBox(g, paint::showBox(paint::SHOW_ALT));
  CHECK(g.save().up == 2 && g.save().range == 0 && g.save().hideAlt == 1 && !g.save().hideCall && !g.save().hideRoute);
  CHECK(g.takeSave(&data, &len, true));
  Save saved; memcpy(&saved, data, sizeof saved);
  tapBox(g, paint::DONE);
  CHECK(!strcmp(g.screenName(), "radar_live"));
  settle(g);
  Input slow; slow.down = slow.pressed = true; slow.x = slow.downX = 60; slow.y = slow.downY = 60;
  frame(g, slow);
  slow.pressed = false;
  for (int i = 0; i < 30; i++) frame(g, slow);   // held 1.2 s: a slow press still steps the range
  slow.down = false; slow.released = true; frame(g, slow);
  CHECK(g.save().range == 1);
  tapAt(g, 150, 80);   // out on the rim (N/E/S/W, the dots): a tap there counts too
  CHECK(g.save().range == 2);
  feed.state = air::S_NO_SPOT; settle(g);
  CHECK(!strcmp(g.screenName(), "radar_nospot"));
  feed.state = air::S_LIVE; settle(g);
  tapBox(g, paint::HOME);
  CHECK(g.wantsHome() && !g.wantsHome());
  g.leave();
  CHECK(!feed.on);
  // Back again: the saved settings come back.
  SaveSlot slot = {&saved, sizeof saved};
  e.saves = &slot;
  now += 60000;   // a minute away; the feed still holds the same fetch
  e.ms = now;
  g.enter(e);
  frame(g);
  CHECK(g.ageSec() == 0);   // no glide carried over from the last visit
  CHECK(g.save().up == 2 && g.save().range == 0 && g.save().hideAlt == 1);
  air::setFeed(nullptr);
  settle(g);
  CHECK(!strcmp(g.screenName(), "radar_off"));
  g.debugPrint();
}

int main() {
  geometry();
  tags();
  words();
  saves();
  pages();
  printf("test_radar: %d checks ok\n", checks);
  return 0;
}
