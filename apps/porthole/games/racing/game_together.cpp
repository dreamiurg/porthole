// Rally's race together (together.h): the Game's side of it. The host (the kid who tapped Together) picks the track,
// starts the race and runs the rivals; the guest drives its own car among the host's rivals. Joint races never touch
// Best times or the adaptive level: two kids racing each other is not a time trial.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "palette.h"
#include "stars.h"

namespace racing {
using together::Stage;
namespace {
float clamp01(float v) { return v < 0 ? 0 : v > 1 ? 1 : v; }
struct Entry { bool finished; uint32_t finishMs; float dist; int tie; };
bool ahead(const Entry& a, const Entry& b) {   // order(): finished by time, others by progress; ties to the host
  if (a.finished != b.finished) return a.finished;
  if (a.finished && a.finishMs != b.finishMs) return a.finishMs < b.finishMs;
  if (!a.finished && a.dist != b.dist) return a.dist > b.dist;
  return a.tie < b.tie;
}
uint8_t rank(const Entry* e, int n, int who) {
  int p = 1;
  for (int i = 0; i < n; i++) p += i != who && ahead(e[i], e[who]);
  return (uint8_t)p;
}
}  // namespace

Stage Game::stageNow() const {
  if (!duo_) return Stage::Idle;
  switch (screen_) {
    case Screen::Looking: return Stage::Looking;
    case Screen::Lobby: return Stage::Lobby;
    case Screen::Calibrate: return recenter_ ? Stage::Race : gripDone_ ? Stage::Ready : Stage::Grip;
    case Screen::Race: case Screen::Pause: case Screen::Confirm: return Stage::Race;
    case Screen::Results: return Stage::Done;
    default: return Stage::Idle;
  }
}

void Game::fillWire() {
  together::Wire& w = wire_;
  w.stage = (uint8_t)stageNow();
  w.paused = screen_ == Screen::Pause || screen_ == Screen::Confirm || (screen_ == Screen::Calibrate && recenter_);
  w.track = (uint8_t)track_; w.session = session_; w.raceId = raceId_;
  // The levels go only to the paired device (the invite is a broadcast: name and avatar only).
  for (int t = 0; t < TRACKS; t++) w.level[t] = link_.paired() ? (int8_t)levelOn(t) : 0;
  w.raceLevel = race_.level;
  const Car& me = race_.car[PLAYER];
  w.finished = me.finished; w.dist = me.dist; w.x = me.x; w.speed = me.speed; w.finishMs = me.finishMs;
  uint8_t theirs = 0;
  places(&w.place, &theirs);
  for (int i = 1; i < CARS; i++) {
    w.hits[i - 1] = race_.struck[i];
    const Car& c = race_.car[i];
    if (host_) w.bot[i - 1] = {c.dist, c.x, c.vx, c.speed, c.finishMs, (uint16_t)c.boostMs, (uint16_t)c.airMs, c.finished, {}};
  }
}

void Game::togetherTick(uint32_t dt) {
  if (!link_.on()) return;
  fillWire();
  link_.tick(ms_, wire_);
  if (!duo_ || !link_.paired()) return;
  if (link_.gen() != lastGen_) { lastGen_ = link_.gen(); peer_ = link_.peer(); onPeer(); }
  if (link_.lost(ms_) || (link_.heard() && (Stage)peer_.stage == Stage::Idle)) { peerGone(); return; }
  // The host's rivals keep driving after its kid is home, while the other kid is still out there.
  if (!host_ || !raced_ || race_.phase != Phase::Finished || (sameRace() && peer_.finished) || peer_.paused) return;
  stepMs_ += dt;
  while (stepMs_ >= STEP_MS) { stepMs_ -= STEP_MS; step(race_, Controls{}); }
}

// Any device on the network can send anything: numbers that would not draw are held to the track (the grid starts behind 0).
static float sane(float v, float lo, float hi) { return isfinite(v) ? (v < lo ? lo : v > hi ? hi : v) : 0; }
bool Game::sameRace() const { return raced_ && peer_.raceId == raceId_; }

// A datagram from the other kid, this race: the guest takes the host's rivals; the host takes the guest's rocket hits.
void Game::onPeer() {
  const float far = (LAPS + 1) * (race_.track ? race_.track->length : 4000.0f);
  peer_.dist = sane(peer_.dist, -far, far); peer_.x = sane(peer_.x, -2, 2); peer_.speed = sane(peer_.speed, 0, 2);
  for (together::Bot& b : peer_.bot) {
    b.dist = sane(b.dist, -far, far); b.x = sane(b.x, -2, 2); b.vx = sane(b.vx, -4, 4); b.speed = sane(b.speed, 0, 2);
  }
  face(1, peer_.avatar);
  if (!sameRace()) return;
  for (int i = 1; i < CARS; i++) {
    if (host_) {
      for (; applied_[i] < peer_.hits[i - 1]; applied_[i]++) strike(race_, i);
      continue;
    }
    const together::Bot& b = peer_.bot[i - 1];
    Car& c = race_.car[i];
    c.dist = b.dist; c.x = b.x; c.vx = b.vx; c.speed = b.speed; c.finishMs = b.finishMs;
    c.boostMs = b.boostMs; c.airMs = b.airMs; c.finished = b.finished;
  }
}

void Game::say(const char* fmt, const char* name) {
  snprintf(notice_, sizeof notice_, fmt, name);
  noticeMs_ = ms_ + together::NOTICE_MS;
}

// The other kid left or went quiet. In a race this one races on alone (no winner unless they were already home);
// anywhere else the race is off and it is back to the cards.
void Game::peerGone() {
  const bool left = (Stage)peer_.stage == Stage::Idle;
  link_.unpair();
  gone_ = true;
  if (raced_) {
    if (!(sameRace() && peer_.finished)) say(left ? "%s left. Keep racing!" : "Lost %s, keep racing!", peer_.name);
    return;
  }
  if (left) say("%s left", peer_.name);
  else say("Race is off", "");
  endTogether();
  go(Screen::Tracks);
}

void Game::startLooking() {
  duo_ = host_ = true; raced_ = gone_ = false;
  session_ = (uint8_t)(session_ + 1 + (ms_ & 7));   // a new invite: a kid who said Not now sees it again
  link_.unpair();
  go(Screen::Looking);
}

void Game::joinInvite() {
  link_.join(ms_);
  raceId_ = 0;   // the host's races count from 1: a race id left from an earlier session never matches
  duo_ = true; host_ = raced_ = gone_ = false;
  peer_ = link_.invite();
  lastGen_ = link_.gen();
  face(1, peer_.avatar);
  track_ = peer_.track % TRACKS;
  go(Screen::Lobby);
}

void Game::endTogether() {
  if (link_.paired()) link_.leave(wire_);
  duo_ = host_ = raced_ = false;
  wire_.stage = (uint8_t)Stage::Idle;
}

void Game::updateLooking() {
  if (ui::clicked(in_, paint::BOARD_BACK)) { endTogether(); go(Screen::Tracks); }
  else if (link_.paired()) go(Screen::Lobby);
  else if (link_.refused()) { say("%s said not now", link_.refusal().name); endTogether(); go(Screen::Tracks); }
  // Both kids tapped Together: the device with the higher address joins the other, which hosts.
  else if (link_.lookingToo(ms_) && link_.self() > link_.inviter()) joinInvite();
}

void Game::updateLobby() {
  if (ui::clicked(in_, paint::BACK)) { endTogether(); go(Screen::Tracks); return; }
  if (!host_) {   // the host picks; the guest follows it to the grip page
    if (!link_.heard()) return;
    track_ = peer_.track % TRACKS;
    const Stage s = (Stage)peer_.stage;
    if (s == Stage::Grip || s == Stage::Ready) go(Screen::Calibrate);
    return;
  }
  if (ui::clicked(in_, paint::LOBBY_PREV)) track_ = (track_ + TRACKS - 1) % TRACKS;
  else if (ui::clicked(in_, paint::LOBBY_NEXT)) track_ = (track_ + 1) % TRACKS;
  else if (ui::clicked(in_, paint::NEXT)) go(Screen::Calibrate);
}

// Both grips set: the host starts the race; the guest starts the host's, as soon as it hears of it.
void Game::awaitStart() {
  if (!link_.paired()) return;
  const Stage s = (Stage)peer_.stage;
  if (host_ ? s == Stage::Ready : s == Stage::Race && peer_.raceId != raceId_) startJointRace();
}

// The same race on both: the host's track, seed and level (the weaker kid's, so the rivals suit both). The kids start
// side by side at the back, the one with the lower level half a grid slot ahead.
void Game::startJointRace() {
  const int mine = levelOn(track_), theirs = peer_.level[track_ % TRACKS];
  if (host_) {
    raceId_ = raceId_ == 255 ? 1 : raceId_ + 1;
    wire_.seed = seed_ + ++runs_;
    start(race_, TRACK[track_], wire_.seed, mine < theirs ? mine : theirs);
  } else {
    raceId_ = peer_.raceId;
    track_ = peer_.track % TRACKS;
    wire_.seed = peer_.seed;
    start(race_, TRACK[track_], peer_.seed, peer_.raceLevel);
  }
  race_.keepField = true;
  Car& me = race_.car[PLAYER];
  me.x = host_ ? 0.35f : -0.35f;
  if (levelOn(track_) < theirs) me.dist += GRID_SPACING / 2.0f;
  memset(applied_, 0, sizeof applied_);
  pausesLeft_ = together::PAUSES; raced_ = true; gone_ = peerPaused_ = false; starsGot_ = 0;
  fresh();
}

void Game::jointFinish() {
  if (gone_ && !(sameRace() && peer_.finished)) return;   // stars are for a race finished together
  const uint32_t day = now_ / 86400;   // shortcut: a UTC day, and a reboot forgets the count; fine for a daily cap
  if (day != starDay_) { starDay_ = day; jointToday_ = 0; }
  if (jointToday_ < 255) jointToday_++;
  if (jointToday_ > together::STAR_RACES) return;
  stars::award(wallet_, together::STARS);
  starsGot_ = together::STARS;
}

bool Game::peerHold() {
  if (duo_ && sameRace() && link_.paired() && peer_.paused) { peerPaused_ = true; boost_ = rocket_ = false; return true; }
  if (peerPaused_) { peerPaused_ = false; resumeMs_ = RESUME_MS; }   // they're back: a Ready beat for both
  return false;
}

float Game::peerDist() const {
  if (!sameRace()) return 0;
  if (peer_.finished || peer_.paused || !link_.paired()) return peer_.dist;
  const uint32_t age = link_.peerAge(ms_);
  return peer_.dist + peer_.speed * V_MAX * (float)(age < together::GUESS_MS ? age : together::GUESS_MS) / 1000;
}

float Game::windTop() const {
  if (!duo_ || !sameRace() || !link_.paired() || peer_.finished) return 1;
  const float behind = (peerDist() - race_.car[PLAYER].dist) / V_MAX;   // seconds
  using namespace together;
  return 1 + TAILWIND * clamp01((behind - WIND_FROM_S) / (WIND_FULL_S - WIND_FROM_S));
}

// Places 1..5 among both kids and the three rivals, the same sum on both devices; theirs 0 when they went before
// finishing.
void Game::places(uint8_t* mine, uint8_t* theirs) const {
  Entry e[CARS + 1];
  int n = 0;
  const Car& me = race_.car[PLAYER];
  e[n++] = {me.finished, me.finishMs, me.dist, host_ ? 0 : 1};
  const bool counted = sameRace() && (!gone_ || peer_.finished);
  if (counted) e[n++] = {peer_.finished != 0, peer_.finishMs, peerDist(), host_ ? 1 : 0};
  for (int i = 1; i < CARS; i++) e[n++] = {race_.car[i].finished, race_.car[i].finishMs, race_.car[i].dist, 1 + i};
  *mine = rank(e, n, 0);
  *theirs = counted ? rank(e, n, 1) : 0;
}

const pic::Pic* Game::face(int slot, int avatar) {
  if (!avatars_ || nAvatars_ <= 0) return nullptr;
  const gfx::Sprite& s = avatars_[avatar % nAvatars_];
  if (s.w * s.h > 20 * 20) return nullptr;
  for (int i = 0; i < s.w * s.h; i++)
    facePx_[slot][i] = s.px[i] == C_T ? pic::CLEAR : rgb888_to_565(PALETTE_RGB[s.px[i] % C_COUNT]);
  faces_[slot] = {s.w, s.h, facePx_[slot]};
  return &faces_[slot];
}

void Game::peerView(paint::Duo& d) {
  d.peerFace = faces_[1].px ? &faces_[1] : nullptr;
  d.peerName = peer_.name;
  const bool same = sameRace();
  d.peerOnRoad = d.paired && same && !peer_.finished;
  d.peerDist = peerDist(); d.peerX = peer_.x;
  d.peerPaused = d.paired && same && peer_.paused;
  d.peerFinished = same && peer_.finished; d.peerFinishMs = peer_.finishMs;
  const Stage ps = (Stage)peer_.stage;
  d.peerAgain = d.paired && screen_ == Screen::Results && (ps == Stage::Lobby || ps == Stage::Grip || ps == Stage::Ready);
}
void Game::duoView(paint::Duo& d) {
  d.on = duo_; d.radio = link_.on(); d.net = (uint8_t)link_.state(); d.host = host_;
  d.paired = link_.paired() && link_.heard(); d.waiting = gripDone_;
  d.face = face(0, wire_.avatar);
  peerView(d);
  d.gone = gone_; d.noWinner = raced_ && gone_ && !d.peerFinished; d.holdOff = duo_ && raced_ && !pausesLeft_;
  d.wind = clamp01((race_.car[PLAYER].top - 1) / together::TAILWIND);
  places(&d.place, &d.peerPlace);
  d.races = jointToday_; d.stars = starsGot_;
  if (screen_ == Screen::Tracks && link_.invited(ms_)) {
    d.inviteName = link_.invite().name;
    d.inviteFace = face(2, link_.invite().avatar);
  }
}
}  // namespace racing
