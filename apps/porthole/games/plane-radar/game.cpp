#include "game.h"
#include <stdio.h>
#include <string.h>
#include "scope.h"

namespace radar {
using namespace paint;
namespace {
bool within(int x, int y, int cx, int cy, int r) { return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r; }
bool on(const ui::Box& b, int x, int y) { return x >= b.x && y >= b.y && x < b.x + b.w && y < b.y + b.h; }
}  // namespace

void Game::enter(const AppEnter& e) {
  in_ = Input{};
  home_ = dirty_ = false;
  snap_ = air::Snapshot{};
  save_ = Save{};
  save_.range = START_RANGE;
  for (int i = 0; i < e.n; i++)   // only this kid's save; a bad one keeps the defaults
    if (e.all[i].id == e.who->id && e.saves[i].len) loadBlob(e.saves[i].data, e.saves[i].len, save_);
  if (save_.range >= RANGES) save_.range = START_RANGE;
  save_.up %= 4;
  ms_ = fetchMs_ = e.ms;
  gen_ = 0;   // whatever the feed holds counts as just fetched: no glide from a visit ago
  go(P_SCOPE);
  setRange(save_.range);
}

void Game::go(Page p) {
  page_ = p;
  double lat = 0, lon = 0;   // home can't change while the menu is open: its words once, on the way in
  air::Feed* f = air::feed();
  if (p == P_MENU) spotText(f && f->spot(&lat, &lon), lat, lon, spot_, sizeof spot_);
  frames_.reset();
  gate_.shown(ms_);
}

void Game::setRange(int r) {
  if (r != save_.range) dirty_ = true;
  save_.range = (uint8_t)r;
  if (air::Feed* f = air::feed()) f->start(fetchKm(r));
}

void Game::leave() {
  if (air::Feed* f = air::feed()) f->stop();
}

bool Game::clicked(const ui::Box& b) const { return ui::clicked(in_, b); }

// Home, the menu, or anywhere else on the glass for the next range (the rim dots and N/E/S/W invite a poke).
void Game::scope() {
  if (clicked(HOME)) { home_ = true; return; }
  UiAudit::add(MENU_CX - MENU_HIT, MENU_CY - MENU_HIT, 2 * MENU_HIT, 2 * MENU_HIT);
  if (!in_.released) return;
  const bool onMenu = within(in_.downX, in_.downY, MENU_CX, MENU_CY, MENU_HIT);
  if (onMenu && within(in_.x, in_.y, MENU_CX, MENU_CY, MENU_HIT)) { go(P_MENU); return; }
  const bool onHome = on(HOME, in_.downX, in_.downY);
  const int dx = in_.x - in_.downX, dy = in_.y - in_.downY;
  if (!onMenu && !onHome && dx * dx + dy * dy <= 36) setRange((save_.range + 1) % RANGES);
}

void Game::menu() {
  if (clicked(DONE)) { go(P_SCOPE); return; }
  for (int i = 0; i < 4; i++)
    if (clicked(upBox(i)) && save_.up != i) { save_.up = (uint8_t)i; dirty_ = true; }
  for (int i = 0; i < RANGES; i++)
    if (clicked(rangeBox(i))) setRange(i);
  uint8_t* hide[SHOWS] = {&save_.hideCall, &save_.hideRoute, &save_.hideAlt};
  for (int i = 0; i < SHOWS; i++)
    if (clicked(showBox(i))) { *hide[i] = !*hide[i]; dirty_ = true; }
}

void Game::update(uint32_t, uint32_t ms, const Input& in) {
  ms_ = ms;
  in_ = in;
  gate_.filter(in_, ms);
  if (air::Feed* f = air::feed()) f->read(snap_);
  else snap_.state = air::S_OFF;
  if (snap_.gen != gen_) { gen_ = snap_.gen; fetchMs_ = ms; }
  switch (page_) {
    case P_SCOPE: scope(); break;
    case P_MENU: menu(); break;
  }
}

void Game::render() {
  const bool homeDown = page_ == P_SCOPE && in_.down && on(HOME, in_.x, in_.y);
  // Repaint only when what a page shows changed: the settings on the menu; on the scope the state, new planes and,
  // while there are planes to glide, each second.
  struct Look { uint8_t page, range, up, call, route, alt, homeDown, state, age; uint32_t gen; } look = {};
  look = {page_, save_.range, save_.up, save_.hideCall, save_.hideRoute, save_.hideAlt, homeDown, 0, 0, 0};
  if (page_ == P_SCOPE) {
    look.state = snap_.state;
    look.gen = snap_.gen;
    look.age = snap_.state == air::S_LIVE && snap_.n ? (uint8_t)ageSec() : 0;
  }
  const View v = {page_, &snap_, save_.range, save_.up, {!save_.hideCall, !save_.hideRoute, !save_.hideAlt}, homeDown,
                  ageSec(), spot_};
  frames_.render(nullptr, 0, os::crc32(&look, sizeof look), [&] { paint::draw(v); });
}

// Capped at the feed's stale time: past it the feed reports lost, and a plane never flies far off its last fix.
int Game::ageSec() const {
  const uint32_t s = (ms_ - fetchMs_) / 1000;
  return (int)(s < air::STALE_MS / 1000 ? s : air::STALE_MS / 1000);
}

bool Game::takeSave(const void** data, size_t* len, bool allowed) {
  if (!allowed || !dirty_) return false;
  seal(save_);
  *data = &save_; *len = sizeof save_;   // the shell copies it out before the next update
  dirty_ = false;
  return true;
}

const char* Game::screenName() const {
  static const char* N[] = {"radar_off", "radar_nospot", "radar_nowifi", "radar_joining", "radar_live", "radar_lost",
                            "radar_menu"};   // a name per air::State, then the menu
  static_assert(sizeof N / sizeof N[0] == air::S_COUNT + 1, "a name per state, then the menu");
  if (page_ == P_MENU) return N[air::S_COUNT];
  return N[snap_.state < air::S_COUNT ? snap_.state : air::S_OFF];
}

void Game::debugPrint() {
  printf("[radar state=%d planes=%d range=%d up=%d call=%d route=%d alt=%d gen=%lu age=%d]\n", (int)snap_.state,
         snap_.n, save_.range, save_.up, !save_.hideCall, !save_.hideRoute, !save_.hideAlt, (unsigned long)snap_.gen,
         ageSec());
  printf("screen=%s\n", screenName());
}

}  // namespace radar
