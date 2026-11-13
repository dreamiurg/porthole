#include "rules.h"
#include <math.h>

namespace pumpkin {
namespace {
constexpr uint32_t WARN_MS = 2000, CHECK_WARN_MS = 3500, REST_MS = 2500, REST_SPAN_MS = 2000;
constexpr float DEAD_MG = 60, MAX_MG = 500, ACCEL = 150, FRICTION = 0.94f, REACH = 7, TURN_BACK = 0.6f;
float clamp(float n, float low, float high) { return n < low ? low : n > high ? high : n; }
uint32_t rnd(Round& r) {  // xorshift32: a round needs variety, not quality
  r.seed ^= r.seed << 13; r.seed ^= r.seed >> 17; r.seed ^= r.seed << 5;
  return r.seed;
}
uint32_t searchMs(Move m) {
  return m == Move::Fast ? 3200 : m == Move::TurnBack ? 4000 : m == Move::Check ? 2000 : 5000;
}
// The first move is always a plain sweep from Grandma's side, so the opening teaches.
// After that, a rustle she heard decides the move: she checks that spot.
void pickMove(Round& r) {
  r.spot = -1;
  r.fromHeard = false;
  if (r.moves++ == 0) { r.move = Move::Sweep; r.fromRight = true; return; }
  if (r.heard >= 0) { r.fromHeard = true; r.move = Move::Check; r.spot = r.heard; r.heard = -1; return; }
  const uint32_t n = rnd(r) % 10;
  r.move = n < 3 ? Move::Sweep : n < 5 ? Move::Fast : n < 7 ? Move::TurnBack : Move::Check;
  r.fromRight = rnd(r) % 3 != 0;
  // A check usually goes for the pumpkin's spot, so a hiding place is a choice, not a fortress.
  if (r.move == Move::Check) r.spot = r.hidden >= 0 && rnd(r) % 5 < 3 ? r.hidden : (int)(rnd(r) % 3);
}
void advance(Round& r) {
  r.phaseMs += STEP_MS;
  if (r.phaseMs < r.phaseLen) return;
  r.phaseMs = 0;
  if (r.phase == Phase::Rest) {
    pickMove(r);
    r.phase = Phase::Warn;
    r.phaseLen = r.move == Move::Check ? CHECK_WARN_MS : WARN_MS;
  } else if (r.phase == Phase::Warn) {
    r.phase = Phase::Search;
    r.phaseLen = searchMs(r.move);
  } else {
    r.phase = Phase::Rest;
    r.phaseLen = REST_MS + rnd(r) % REST_SPAN_MS;
  }
}
void rollPumpkin(Round& r, float roll) {
  if (r.hidden >= 0) return;
  float force = 0;
  if (fabsf(roll) > DEAD_MG) force = clamp(roll, -MAX_MG, MAX_MG) / MAX_MG;
  r.velocity = (r.velocity + force * ACCEL * STEP_MS / 1000) * FRICTION;  // tops out near 23.5 px/s
  if (fabsf(r.velocity) < 0.02f) r.velocity = 0;
  r.x += r.velocity * STEP_MS / 1000;
  if (r.x < 24 || r.x > 136) { r.x = clamp(r.x, 24, 136); r.velocity = 0; }
}
void hide(Round& r, bool hold) {
  if (!hold) { r.hidden = -1; return; }
  if (r.hidden >= 0) return;
  r.hidden = cover(r);
  if (r.hidden < 0) return;
  r.x = SPOTS[r.hidden];
  r.velocity = 0;
  r.settled = false;  // it may arrive at full tilt, rolled in while held
}
void listen(Round& r, float roll) {
  const bool hard = fabsf(roll) > RUSTLE_MG;
  if (!hard) r.settled = true;
  r.rustle = r.hidden >= 0 && r.settled && hard ? r.rustle + STEP_MS : 0;
  if (r.rustle < RUSTLE_MS) return;
  r.heard = r.hidden;
  r.rustle = 0;
}
// Something crashes across the room: whatever Grandma was doing, she goes to look.
// A move only announced is handed back, so the teaching opener and a heard check still come next.
void haunt(Round& r) {
  if (r.ghostUsed) return;
  r.ghostUsed = true;
  r.distract = DISTRACT_MS;
  if (r.phase == Phase::Warn) {
    r.moves--;
    if (r.fromHeard && r.heard < 0) r.heard = r.spot;
  }
  if (r.phase == Phase::Rest) { r.phaseLen += GHOST_EXTRA_MS; return; }
  r.phase = Phase::Rest;
  r.phaseMs = 0;
  r.phaseLen = GHOST_REST_MS;
}
void collect(Round& r) {
  if (r.sinceCandy < 60000) r.sinceCandy += STEP_MS;
  if (r.hidden >= 0) return;
  for (int i = 0; i < CANDIES; i++)
    if (!(r.got & (1 << i)) && fabsf(r.x - r.candy[i]) <= REACH) { r.got |= 1 << i; r.sinceCandy = 0; }
}
// Exposure builds only while seen, so mashing the sign can't reset it; hidden is never seen, since nothing hides in the
// spot a Check names. PHEW! waits for the end of a search the beam came close in: shown as the beam merely passed, it
// told a 6-year-old to let go mid-search.
void watch(Round& r) {
  const bool inBeam = searching(r) && fabsf(r.x - beamX(r)) <= 20;
  const bool seen = inBeam && r.hidden < 0;
  const bool phew = r.close && !searching(r);
  r.close = searching(r) && (r.close || inBeam);
  r.covered = inBeam && !seen;
  r.exposure = seen ? r.exposure + STEP_MS : 0;
  r.phew = phew ? PHEW_MS : r.phew > STEP_MS ? r.phew - STEP_MS : 0;
  r.caught = r.exposure >= CATCH_MS;
}
}  // namespace
Round newRound(uint32_t seed) {
  Round r;
  r.seed = seed ? seed : 1;
  r.candy[0] = (float)(52 + rnd(r) % 15);   // between the chair and the table
  r.candy[1] = (float)(94 + rnd(r) % 15);   // between the table and the plant
  r.candy[2] = (float)(130 + rnd(r) % 7);   // out on Grandma's side
  return r;
}
int nearby(const Round& r) {
  for (int i = 0; i < 3; i++) if (fabsf(r.x - SPOTS[i]) <= 13) return i;
  return -1;
}
int cover(const Round& r) {
  const int s = nearby(r);
  return s == checking(r) ? -1 : s;
}
// Rolls a copy, friction and tilt included: speed alone over-promised a levelled pumpkin that stops short.
// shortcut: Grandma's phase is frozen for the 0.5 s, so a Check starting or ending inside it can mispredict
// one "no"; step advance() on the copy if a playtest ever notices.
int arriving(const Round& r, float roll, uint32_t ms) {
  Round c = r;
  for (uint32_t t = 0; cover(c) < 0 && t < ms; t += STEP_MS) rollPumpkin(c, roll);
  return cover(c);
}
bool warning(const Round& r) { return r.phase == Phase::Warn; }
bool searching(const Round& r) { return r.phase == Phase::Search; }
int checking(const Round& r) { return r.move == Move::Check && r.phase != Phase::Rest ? r.spot : -1; }
float beamX(const Round& r) {
  if (checking(r) >= 0) return SPOTS[r.spot];
  float t = r.phase == Phase::Search ? (float)r.phaseMs / (float)r.phaseLen : 0;
  if (r.move == Move::TurnBack) t = (t < 0.5f ? t : 1 - t) * 2 * TURN_BACK;
  return r.fromRight ? 136 - t * 112 : 24 + t * 112;
}
bool doorOpen(const Round& r) { return r.got == ALL_CANDY; }
void step(Round& r, float roll, bool hold, bool ghost) {
  if (r.caught || r.won) return;
  hide(r, hold);
  if (ghost) haunt(r);
  r.distract = r.distract > STEP_MS ? r.distract - STEP_MS : 0;
  r.startle = r.startle > STEP_MS ? r.startle - STEP_MS : 0;
  listen(r, roll);
  rollPumpkin(r, roll);
  collect(r);
  r.elapsed += STEP_MS;
  advance(r);
  // She names the spot it hides in: no cover now, so it jumps out. A held finger hides it at the next cover it reaches.
  if (r.hidden >= 0 && r.hidden == checking(r)) { r.hidden = -1; r.startle = STARTLE_MS; }
  watch(r);
  r.won = !r.caught && doorOpen(r) && r.hidden < 0 && r.x <= DOOR_X;
}
}  // namespace pumpkin
