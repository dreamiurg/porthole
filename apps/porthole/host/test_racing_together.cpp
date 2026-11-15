// Rally's race together: two Games on an in-memory network, from the invite to the results and the ways a race
// together ends early. The wire protocol on its own first.
#include "host/check.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <deque>
#include <vector>
#include "gfx565.h"
#include "games/racing/game.h"

using namespace racing;
using together::Stage;
namespace {
// The household wifi: every radio that is on hears a broadcast; `cut` drops everything.
struct MemRadio;
struct Air { std::vector<MemRadio*> radios; bool cut = false; };
struct MemRadio final : lan::Radio {
  struct Pkt { uint32_t from; std::vector<uint8_t> data; };
  Air* air; uint32_t addr; bool on = false;
  std::deque<Pkt> inbox;
  MemRadio(Air* a, uint32_t ad) : air(a), addr(ad) { a->radios.push_back(this); }
  void start() override { on = true; }
  void stop() override { on = false; inbox.clear(); }
  lan::State state() override { return on ? lan::S_READY : lan::S_OFF; }
  uint32_t self() override { return addr; }
  void send(const void* d, size_t len, uint32_t to) override {
    if (air->cut || !on) return;
    const uint8_t* b = (const uint8_t*)d;
    for (MemRadio* r : air->radios)
      if (r != this && r->on && (!to || to == r->addr)) r->inbox.push_back({addr, std::vector<uint8_t>(b, b + len)});
  }
  size_t recv(void* buf, size_t cap, uint32_t* from) override {
    if (inbox.empty()) return 0;
    Pkt p = inbox.front(); inbox.pop_front();
    const size_t n = p.data.size() < cap ? p.data.size() : cap;
    memcpy(buf, p.data.data(), n);
    *from = p.from;
    return n;
  }
};

uint8_t AVATAR_PX[20 * 20];
const gfx::Sprite AVATARS[2] = {{20, 20, AVATAR_PX}, {20, 20, AVATAR_PX}};

struct Kid {
  Game g;
  InputTracker tracker;
  uint32_t ms = 1000, wallet = 0;
  Profile who;
  std::vector<uint16_t> fb = std::vector<uint16_t>(480 * 480);
  Kid(const char* name, uint8_t avatar, lan::Radio* r) : who{avatar, {}, avatar, 8, 0} {
    snprintf(who.name, sizeof who.name, "%s", name);
    lan::setRadio(r);
    gfx565::target(fb.data());
    g.enter(AppEnter{&who, &who, nullptr, 1, 1000000, ms, false, &wallet, AVATARS, 2});
    lan::setRadio(nullptr);   // each Game keeps the radio it entered with
  }
  void frame(bool down = false, int x = 0, int y = 0) {
    ms += 40;
    Input in = tracker.step(down, x, y, ms);
    in.gx = 0; in.gy = 1000; in.gz = 0;   // held upright, like a wheel
    gfx565::target(fb.data());
    g.update(1000000 + ms / 1000, ms, in);
    g.render();
  }
};
struct Pair {
  Air air;
  MemRadio ra{&air, 11}, rb{&air, 22};
  Kid sam{"Sam", 0, &ra}, zoe{"Zoe", 1, &rb};
  void wait(uint32_t t) { for (uint32_t e = 0; e < t; e += 40) { sam.frame(); zoe.frame(); } }
  void tap(Kid& k, const ui::Box& b) {
    wait(500);
    Kid& other = &k == &sam ? zoe : sam;
    for (int i = 0; i < 2; i++) { k.frame(true, b.x + b.w / 2, b.y + b.h / 2); other.frame(); }
    k.frame(); other.frame();
  }
  void toLobby() {
    tap(sam, paint::TOGETHER);
    assert(sam.g.screen() == Game::Screen::Looking);
    wait(200);
    assert(zoe.g.duo().inviteName && !strcmp(zoe.g.duo().inviteName, "Sam"));
    tap(zoe, paint::JOIN);
    wait(300);
    assert(sam.g.screen() == Game::Screen::Lobby && zoe.g.screen() == Game::Screen::Lobby);
  }
  void toRace() {
    toLobby();
    tap(sam, paint::NEXT);   // Go: both to the grip page
    wait(200);
    assert(sam.g.screen() == Game::Screen::Calibrate && zoe.g.screen() == Game::Screen::Calibrate);
    wait(600);
    tap(sam, paint::NEXT);
    assert(sam.g.screen() == Game::Screen::Calibrate && sam.g.duo().waiting);   // waits for Zoe's grip
    tap(zoe, paint::NEXT);
    wait(200);
    assert(sam.g.screen() == Game::Screen::Race && zoe.g.screen() == Game::Screen::Race);
  }
};

// The wire on its own: an invite, a join, the pair talking only to each other, a Not now, a goodbye.
void link() {
  Air air;
  MemRadio a{&air, 1}, b{&air, 2}, c{&air, 3};
  a.start(); b.start(); c.start();
  together::Link la, lb, lc;
  la.begin(&a); lb.begin(&b); lc.begin(&c);
  together::Wire wa{}, wb{}, wc{};
  wa.stage = (uint8_t)Stage::Looking; wa.session = 7; snprintf(wa.name, sizeof wa.name, "Sam");
  la.tick(0, wa);
  lb.tick(10, wb); lc.tick(10, wc);
  assert(lb.invited(10) && lc.invited(10) && !strcmp(lb.invite().name, "Sam"));
  assert(!lb.invited(10 + together::INVITE_MS));   // a host that stopped looking is no invite
  lc.decline(wc);
  assert(!lc.invited(10));
  la.tick(15, wa);   // the Not now reaches the host
  assert(la.refused() && !strcmp(la.refusal().name, wc.name) && !la.paired());
  wc.stage = (uint8_t)Stage::Idle;
  lb.join(10);
  wb.stage = (uint8_t)Stage::Lobby;
  lb.tick(20, wb);   // addressed to the host
  assert(b.inbox.empty() && c.inbox.empty() && a.inbox.size() == 1);
  la.tick(30, wa);
  assert(la.paired() && la.heard() && la.gen() == 1);
  wa.stage = (uint8_t)Stage::Lobby;
  la.tick(40, wa);
  assert(c.inbox.empty());   // paired: only the guest hears the host
  lb.tick(50, wb);
  assert(lb.heard() && lb.peer().stage == (uint8_t)Stage::Lobby && !lb.lost(50));
  la.tick(60, wa);   // nothing new, inside SEND_MS: quiet
  assert(b.inbox.empty());
  assert(lb.lost(50 + together::LOST_MS + 1));
  wa.stage = (uint8_t)Stage::Looking; wa.session = 8;   // a new invite: shown again to the kid who said Not now
  together::Link la2;
  la2.begin(&a);
  la2.tick(70, wa);
  lc.tick(80, wc);
  assert(lc.invited(80));
  la.leave(wa);
  assert(!la.paired() && b.inbox.size() == 4);   // the new invite, then three goodbyes
  uint8_t junk[8] = {1};
  a.send(junk, sizeof junk, 0);   // the wrong size: dropped
  lb.tick(90, wb);
  assert(lb.peer().stage == (uint8_t)Stage::Idle);
}

// Invite, Not now, invite again, join, and only the host picks the track.
void inviteAndLobby(Pair& p) {
  assert(p.sam.g.duo().radio && !p.sam.g.duo().on);
  p.tap(p.sam, paint::TOGETHER);
  p.wait(200);
  p.tap(p.zoe, paint::NOT_NOW);
  assert(p.zoe.g.screen() == Game::Screen::Tracks && !p.zoe.g.duo().inviteName);
  p.wait(300);   // Sam hears the Not now: back to the cards, told so
  assert(p.sam.g.screen() == Game::Screen::Tracks);
  p.tap(p.sam, paint::TOGETHER);   // looking again: a new invite
  p.wait(200);
  assert(p.zoe.g.duo().inviteName);
  p.tap(p.zoe, paint::JOIN);
  p.wait(300);
  assert(p.zoe.g.screen() == Game::Screen::Lobby && p.zoe.g.duo().paired && !p.zoe.g.duo().host);
  p.tap(p.sam, paint::LOBBY_NEXT);
  p.tap(p.zoe, paint::LOBBY_NEXT);   // only the host picks
  p.wait(200);
  assert(p.zoe.g.screen() == Game::Screen::Lobby);
}

// Both grips, then the same race: the host's track and rivals on both, side by side, five places.
void sameRace(Pair& p) {
  p.tap(p.sam, paint::NEXT);
  p.wait(800);
  p.tap(p.sam, paint::NEXT);
  p.tap(p.zoe, paint::NEXT);
  p.wait(200);
  const Race &a = p.sam.g.race(), &b = p.zoe.g.race();
  assert(a.track == &TRACK[1] && b.track == &TRACK[1] && a.level == b.level);
  assert(a.car[PLAYER].x > 0 && b.car[PLAYER].x < 0);   // side by side
  p.wait(4000);   // through the countdown, racing
  assert(a.phase == Phase::Racing && b.phase == Phase::Racing);
  for (int i = 1; i < CARS; i++) assert(fabsf(a.car[i].dist - b.car[i].dist) < 8);   // the host's rivals on both
  const paint::Duo dz = p.zoe.g.duo();
  assert(dz.peerOnRoad && fabsf(dz.peerDist - a.car[PLAYER].dist) < 8 && dz.place >= 1 && dz.place <= 5);
  assert(p.sam.g.duo().place != dz.place);
}

void pauseTogether(Pair& p) {
  const Race& b = p.zoe.g.race();
  // Sam holds the pause: Zoe's race waits, then both get a Ready beat.
  for (int i = 0; i < 18; i++) { p.sam.frame(true, paint::LAP.x + 4, paint::LAP.y + 4); p.zoe.frame(); }
  p.sam.frame(); p.zoe.frame();
  assert(p.sam.g.screen() == Game::Screen::Pause);
  p.wait(200);
  const float held = b.car[PLAYER].dist;
  p.wait(600);
  assert(b.car[PLAYER].dist == held && p.zoe.g.duo().peerPaused);
  p.tap(p.sam, paint::RESUME);
  p.wait(1600);
  assert(b.car[PLAYER].dist > held && p.zoe.g.screen() == Game::Screen::Race);
}

void finishTogether(Pair& p) {
  // Zoe finishes first; Sam's rivals keep driving while Sam is still out; then Sam.
  p.zoe.g.debugCmd("lastlap");
  p.wait(6000);
  assert(p.zoe.g.screen() == Game::Screen::Results && !p.zoe.g.duo().peerFinished);
  p.sam.g.debugCmd("lastlap");
  p.wait(6000);
  assert(p.sam.g.screen() == Game::Screen::Results);
  p.wait(300);
  const paint::Duo s = p.sam.g.duo(), z = p.zoe.g.duo();
  assert(s.peerFinished && z.peerFinished && s.place == z.peerPlace && z.place == s.peerPlace && s.place != z.place);
  assert(p.sam.wallet == together::STARS && p.zoe.wallet == together::STARS && s.races == 1);
  gfx::textLogEnabled = true; gfx::textLogReset();
  p.zoe.frame();
  gfx::textLogEnabled = false;
  for (int i = 0; i < gfx::textLogCount; i++) assert(gfx::inCircle(gfx::textLog[i].x, gfx::textLog[i].y, 1));
  // Again: Zoe asks, Sam sees it; then Sam goes home and Zoe is told.
  p.tap(p.zoe, paint::REPLAY);
  p.wait(200);
  assert(p.zoe.g.screen() == Game::Screen::Lobby && p.sam.g.duo().peerAgain);
  p.tap(p.sam, paint::RESULT_HOME);
  p.wait(200);
  assert(p.sam.g.wantsHome() && p.zoe.g.screen() == Game::Screen::Tracks);
}

// The whole thing on two devices.
void raceTogether() {
  Pair p;
  inviteAndLobby(p);
  sameRace(p);
  pauseTogether(p);
  finishTogether(p);
}

// The wifi drops mid race: each kid races on alone, and nobody wins.
void dropped() {
  Pair p;
  p.toRace();
  p.wait(4000);
  p.air.cut = true;
  p.wait(together::LOST_MS + 400);
  assert(p.sam.g.screen() == Game::Screen::Race && p.zoe.g.screen() == Game::Screen::Race);
  assert(!p.zoe.g.duo().peerOnRoad && p.zoe.g.duo().gone);
  p.zoe.g.debugCmd("lastlap");
  p.wait(6000);
  assert(p.zoe.g.screen() == Game::Screen::Results && p.zoe.g.duo().noWinner);
  p.tap(p.zoe, paint::REPLAY);   // Replay: alone now
  assert(p.zoe.g.screen() == Game::Screen::Race && !p.zoe.g.duo().on);
}

// Leaving before the race: the other kid is back at the cards, told why.
void leftEarly() {
  Pair p;
  p.toLobby();
  p.tap(p.zoe, paint::BACK);
  p.wait(200);
  assert(p.sam.g.screen() == Game::Screen::Tracks && p.zoe.g.screen() == Game::Screen::Tracks);
  p.toLobby();
  p.air.cut = true;   // gone quiet in the lobby: the race is off
  p.wait(together::LOST_MS + 400);
  assert(p.sam.g.screen() == Game::Screen::Tracks && p.zoe.g.screen() == Game::Screen::Tracks);
  p.air.cut = false;
  // Together without wifi set up: the page says so, Back returns.
  Air air;
  struct NoWifi final : lan::Radio {
    void start() override {} void stop() override {}
    lan::State state() override { return lan::S_NO_WIFI; } uint32_t self() override { return 0; }
    void send(const void*, size_t, uint32_t) override {} size_t recv(void*, size_t, uint32_t*) override { return 0; }
  } none;
  Kid k("Kai", 0, &none);
  for (int i = 0; i < 15; i++) k.frame();
  k.frame(true, paint::TOGETHER.x + 20, paint::TOGETHER.y + 10); k.frame(true, paint::TOGETHER.x + 20, paint::TOGETHER.y + 10); k.frame();
  assert(k.g.screen() == Game::Screen::Looking && k.g.duo().net == lan::S_NO_WIFI);
  (void)air;
}

// Both kids tap Together at once: one joins the other (the higher address), and they are in one lobby.
void bothLooking() {
  Pair p;
  p.tap(p.sam, paint::TOGETHER);
  p.tap(p.zoe, paint::TOGETHER);
  p.wait(800);
  assert(p.sam.g.screen() == Game::Screen::Lobby && p.zoe.g.screen() == Game::Screen::Lobby);
  assert(p.sam.g.duo().host != p.zoe.g.duo().host && p.sam.g.duo().paired && p.zoe.g.duo().paired);
}

// The Together targets keep the house rules: whole targets on the glass, apart from their neighbors.
void geometry() {
  using namespace paint;
  const ui::Box boxes[] = {TOGETHER, JOIN, NOT_NOW, LOBBY_PREV, LOBBY_NEXT};
  for (const ui::Box& b : boxes) {
    assert(b.w >= 24 && b.h >= 22);
    for (int k = 0; k < 4; k++) assert(gfx::inCircle(b.x + (k & 1) * b.w, b.y + (k >> 1) * b.h, 2));
  }
}
}  // namespace

int main() {
  memset(AVATAR_PX, 3, sizeof AVATAR_PX);
  AVATAR_PX[0] = C_T;
  link();
  geometry();
  raceTogether();
  dropped();
  leftEarly();
  bothLooking();
  puts("test_racing_together: ok");
  return 0;
}
