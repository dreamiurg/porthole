// Plane Radar's look: a night-navy scope with sea-green rings, B612 Mono lettering (the cockpit typeface), planes as
// amber, cyan or coral arrowheads by kind. paint::draw paints a whole page from a View; the game repaints only when
// the View changes (canvas::Frames). Layout boxes are logical px (the game hit-tests them); drawing is physical.
#pragma once
#include "air.h"
#include "gfx.h"
#include "ui.h"

namespace radar::paint {

enum Page : uint8_t { P_SCOPE, P_MENU };

// The scope: home low on the left, the menu button in the middle.
constexpr ui::Box HOME = {24, 113, 24, 22};
constexpr int MENU_CX = 80, MENU_CY = 80, MENU_R = 7, MENU_HIT = 14;   // drawn radius, touch radius
// The menu: Done on top, which way is up, the range, what a plane's label shows (flight, route, altitude), then where
// home is (read only: a grown-up sets it).
constexpr ui::Box DONE = {68, 6, 24, 22};
inline ui::Box upBox(int i) { return {26 + 28 * i, 38, 24, 22}; }
inline ui::Box rangeBox(int i) { return {26 + 28 * i, 72, 24, 22}; }
enum { SHOW_CALL, SHOW_ROUTE, SHOW_ALT, SHOWS };
inline ui::Box showBox(int i) { return {26 + 36 * i, 106, 34, 22}; }

struct View {
  Page page;
  const air::Snapshot* snap;
  int range, up;            // scope.h RANGE_MI index; Save::up (which way is at the top)
  bool show[SHOWS];         // what a plane's label shows, by SHOW_*
  bool homeDown;
  int ageSec;               // whole seconds since the planes were fetched: they are drawn flown on that far
  const char* spot;         // the menu's "Home 47.62, -122.35"
};

void draw(const View& v);
const gfx::Sprite& icon();   // the launcher's, in the shell's indexed palette

}  // namespace radar::paint
