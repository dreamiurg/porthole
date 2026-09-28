// Self-check for Tilt FC (games/tilt-fc/): the tilt's dead zone from any grip, pass targeting in the cone, control
// switching, goals, the kickoff, tackles and slides, the keeper's room and throw, the whistle, a tap held level; the
// challenge (a greedy player scores clearly less than one who passes and aims and does not win, a player who does
// nothing is not scored on in a hurry, and each level no easier than the last); the save, the result saved the
// moment the match is decided; the court's pages without a fresh-page pause, the leave hold, pages mashing cannot skip;
// every drawn string in the font; every incrementally drawn frame equal to a full repaint; and the playtests' recorded
// matches in step with the rules (`build/host/test_tiltfc --write-playtests` rewrites them).
// Run: make test
#include <assert.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "games/tilt-fc/game.h"
#include "games/tilt-fc/generated/fonts.h"
#include "host/grip.h"

using namespace fc;
using namespace grip;
static int checks = 0;
#define CHECK(c) (assert(c), checks++)

static float len(Vec v) { return sqrtf(v.x * v.x + v.y * v.y); }
static Vec unit(Vec v) { const float l = len(v); return l > 0 ? Vec{v.x / l, v.y / l} : Vec{0, 0}; }
static Vec sub(Vec a, Vec b) { return {a.x - b.x, a.y - b.y}; }
static float dist(Vec a, Vec b) { return len(sub(a, b)); }
static Vec tiltAt(Vec dir) { return {dir.x * 400, dir.y * 400}; }   // well past the dead zone
static const Vec THEIR_GOAL = {0, -HALF_H};

// ---- the rules
// A match with everyone parked where a test wants them, standing, `owner` on the ball (or it loose at `ball`).
static Match parked(const Vec at[PLAYERS], int owner, Vec ball = {0, 0}) {
  Match m = start(0, 5);
  for (int i = 0; i < PLAYERS; i++) m.pl[i] = {at[i], {0, 0}, {0, i < 3 ? -1.0f : 1.0f}, 0, 0, 0};
  m.ball = {owner >= 0 ? Vec{at[owner].x + m.pl[owner].face.x * FOOT, at[owner].y + m.pl[owner].face.y * FOOT} : ball,
            {0, 0}, (int8_t)owner, -1, (int8_t)owner, 0};
  m.control = owner == 1 ? 1 : 0;
  return m;
}
// Teal 10 at the bottom of the centre circle with the ball, his keeper home, coral out of the way up the court.
static const Vec HOME[PLAYERS] = {{0, 60}, {-110, 120}, {0, KEEPER_Y}, {-120, -150}, {120, -150}, {0, -KEEPER_Y}};

// Tilt sets the direction only: nothing inside the dead zone, the tilt's direction past it however small or big, from
// any grip; the kid's player runs at one speed.
static void deadZone() {
  for (Grav n : {FLAT, LEANED, UPRIGHT}) {
    CHECK(len(heading(tilt::from(n, n))) == 0);
    CHECK(len(heading(tilt::from(gravityFor(DEAD_MG - 5, 0, n), n))) == 0);
    CHECK(len(heading(tilt::from(gravityFor(-50, 60, n), n))) == 0);   // 78 mg diagonally
    const Vec small = heading(tilt::from(gravityFor(0, -(DEAD_MG + 25), n), n)), big = heading(tilt::from(gravityFor(0, -600, n), n));
    CHECK(fabsf(small.y + 1) < 0.02f && fabsf(big.y + 1) < 0.02f);
    const Vec diag = heading(tilt::from(gravityFor(300, 300, n), n));
    CHECK(fabsf(diag.x - 0.7071f) < 0.02f && fabsf(diag.y - 0.7071f) < 0.02f);
  }
  const float tilts[3] = {DEAD_MG - 5.0f, DEAD_MG + 5.0f, 900};
  for (int k = 0; k < 3; k++) {
    Match m = parked(HOME, 0);
    step(m, {0, -tilts[k]}, false);
    CHECK(fabsf(len(m.pl[0].v) - (k ? RUN_SPEED : 0)) < 0.01f);
  }
  // the aim is the tilt past the dead zone, else where the kid's player faces
  Match m = parked(HOME, 0);
  m.pl[0].face = {1, 0};
  CHECK(aimDir(m, {10, 10}).x == 1 && fabsf(aimDir(m, {0, 300}).y - 1) < 0.001f);
}

static Kick aimFrom(const Vec at[PLAYERS], float degrees) {   // #10's kick along `degrees` clockwise from straight up
  const Match m = parked(at, 0);
  const float a = degrees * 3.14159265f / 180;
  return aim(m, 0, {sinf(a), -cosf(a)});
}
// A tap passes to the teammate nearest the aim within +-30 degrees, shoots when the goal is in the cone and in range
// (at where the aim crosses the goal line, inside the posts), and otherwise rolls the ball on ahead.
static void passCone() {
  Vec at[PLAYERS];
  memcpy(at, HOME, sizeof at);
  at[1] = {40, -60};   // 22 degrees right of straight up from the ball at (0, 39): the cone is -8 to 52 degrees
  Kick k = aimFrom(at, 0);
  CHECK(k.kind == Kick::PASS && k.to == 1 && dist(k.at, at[1]) < 0.01f);
  CHECK(aimFrom(at, 51.5f).kind == Kick::PASS && aimFrom(at, 52.5f).kind == Kick::ROLL);
  CHECK(aimFrom(at, -7.5f).kind == Kick::PASS && aimFrom(at, -8.5f).kind == Kick::ROLL);
  k = aimFrom(at, -90);
  CHECK(k.kind == Kick::ROLL && fabsf(k.at.x + 100) < 0.01f);   // nobody that way: on ahead
  at[1] = {30, 150};   // behind him, beside his own keeper: the aim picks whichever is nearer it
  CHECK(aimFrom(at, 180).to == TEAL_KEEPER && aimFrom(at, 162).to == 1);
  // near their goal: a shot, where the aim crosses the line, never outside the posts
  at[0] = {20, -60};   // the ball at (20, -81): 110 px from the goal's middle
  k = aimFrom(at, 0);
  CHECK(k.kind == Kick::SHOOT && k.at.y == -HALF_H && fabsf(k.at.x - 20) < 0.5f);
  k = aimFrom(at, 15);   // 25 degrees right of the goal's middle: at the far post, not past it
  CHECK(k.kind == Kick::SHOOT && k.at.x == GOAL_HALF - BALL_R - 5);
  at[0] = {0, 40};   // the goal in the cone but out of range: no shot
  CHECK(aimFrom(at, 0).kind != Kick::SHOOT);
  at[0] = {110, -130};   // in range but out of the cone
  CHECK(aimFrom(at, 0).kind != Kick::SHOOT);
}

static void stepN(Match& m, int n, Vec tilt = {0, 0}) { for (int i = 0; i < n && m.scored < 0; i++) step(m, tilt, false); }
// Control follows the ball: to the receiver of a pass at once, to the teal player who wins it, and when the ball is
// someone else's to the nearer teal outfield player, by a margin.
static void controlSwitch() {
  Vec at[PLAYERS];
  memcpy(at, HOME, sizeof at);
  at[1] = {40, -60};
  Match m = parked(at, 0);
  step(m, tiltAt(unit(sub(at[1], m.ball.p))), true);
  CHECK(m.ball.owner < 0 && m.ball.passTo == 1);
  step(m, {0, 0}, false);
  CHECK(m.control == 1);
  int n = 0;
  for (; n < 150 && m.ball.owner != 1; n++) { step(m, {0, 0}, false); CHECK(m.control == 1); }   // he meets it by himself
  CHECK(m.ball.owner == 1);
  // coral on the ball: the nearer teal player, but only when he is SWITCH_MARGIN nearer
  memcpy(at, HOME, sizeof at);
  at[3] = {0, -40}; at[0] = {0, 60}; at[1] = {-40, -10};
  m = parked(at, 3);
  step(m, {0, 0}, false);
  CHECK(m.control == 1);
  at[1] = {-80, -10};   // 83 px from the ball against 101: not enough
  m = parked(at, 3);
  step(m, {0, 0}, false);
  CHECK(m.control == 0);
}

// A tap without a tilt while carrying: the aim is where he faces, and when nobody is that way it goes to the teammate
// instead of rolling on to whoever stands ahead (at a teal kickoff that was coral's #8).
static void levelTap() {
  Match m = start(0, 1);
  CHECK(aimDir(m, {0, 0}).y == -1 && aim(m, 0, {0, -1}).kind == Kick::ROLL);
  CHECK(kidKick(m, {0, 0}).kind == Kick::PASS && kidKick(m, {0, 0}).to == 1);
  step(m, {0, 0}, true);
  CHECK(m.ball.owner < 0 && m.ball.passTo == 1);
  CHECK(kidKick(parked(HOME, 0), tiltAt({-1, 0})).kind == Kick::ROLL);   // a tilt nobody stands along still rolls it
  Vec at[PLAYERS];
  memcpy(at, HOME, sizeof at);
  at[0] = {20, -60};   // facing their goal in range: still a shot
  CHECK(kidKick(parked(at, 0), {0, 0}).kind == Kick::SHOOT);
}

// A goal once the ball's middle crosses a goal line between the posts, then the ball rests in the net and nothing
// moves until the kickoff; wide of a post, the wall. Own goals count for the other side.
static void goals() {
  Vec at[PLAYERS];
  memcpy(at, HOME, sizeof at);
  at[5] = {-110, -KEEPER_Y};   // coral's keeper out of the way
  Match m = parked(at, -1, {10, -175});
  m.ball.v = {0, -300};
  stepN(m, 20);
  CHECK(m.scored == TEAL && m.score[TEAL] == 1 && m.ball.p.y < -HALF_H && m.ball.owner < 0);
  const uint32_t ms = m.ms;
  step(m, tiltAt({1, 0}), true);
  CHECK(m.ms == ms && m.ball.p.y < -HALF_H);
  m = parked(at, -1, {GOAL_HALF, -175});   // on the post's line: off the wall, back into the court
  m.ball.v = {0, -300};
  stepN(m, 40);
  CHECK(m.scored < 0 && m.ball.p.y > -HALF_H && m.ball.v.y > 0);
  at[2] = {110, KEEPER_Y};
  m = parked(at, -1, {-20, 176});   // into their own goal
  m.ball.v = {0, 300};
  stepN(m, 20);
  CHECK(m.scored == CORAL && m.score[CORAL] == 1);
}

// After a goal the side that conceded kicks off: everyone back in place (coral's the teal places turned half round),
// the taker on the ball on the spot, nobody moving; the score stays.
static void kickoffReset() {
  Match m = start(2, 9);
  const Match fresh = m;
  CHECK(m.ball.owner == 0 && m.control == 0 && m.ball.p.x == 0 && m.ball.p.y == 0 && m.level == 2);
  stepN(m, 300, {300, -200});
  m.score[TEAL] = 1; m.scored = TEAL;
  kickoff(m, CORAL);
  CHECK(m.scored < 0 && m.score[TEAL] == 1 && m.ball.owner == 3 && m.ball.p.x == 0 && m.ball.p.y == 0);
  for (int i = 0; i < PLAYERS; i++) {
    const Vec s = fresh.pl[(i + 3) % PLAYERS].p;
    CHECK(m.pl[i].p.x == -s.x && m.pl[i].p.y == -s.y && len(m.pl[i].v) == 0 && !m.pl[i].coolMs);
  }
  kickoff(m, TEAL);
  for (int i = 0; i < PLAYERS; i++) CHECK(m.pl[i].p.x == fresh.pl[i].p.x && m.pl[i].p.y == fresh.pl[i].p.y);
}

// Touching the ball someone else carries knocks it loose (nobody falls; the carrier cannot take it straight back); a
// slide reaches further; a slide that wins nothing costs SLOW_MS; a keeper's ball is his and gives him room.
static void tackles() {
  Vec at[PLAYERS];
  memcpy(at, HOME, sizeof at);
  at[0] = {0, 0}; at[3] = {0, -40};   // coral right on #10's ball
  Match m = parked(at, 0);
  step(m, {0, 0}, false);
  CHECK(m.ball.owner < 0 && m.ball.kicker == 3 && m.pl[0].coolMs > 0 && len(m.ball.v) > 0);
  memcpy(at, HOME, sizeof at);
  at[3] = {0, -90}; at[0] = {0, -15};   // coral carrying down the court, #10 a little way off
  m = parked(at, 3);
  step(m, tiltAt({0, -1}), true);
  int n = 0;
  for (; n < 40 && m.ball.owner == 3; n++) step(m, {0, 0}, false);
  CHECK(m.ball.owner != 3 && m.ball.kicker == 0 && !m.pl[0].slowMs);
  memcpy(at, HOME, sizeof at);
  at[1] = {130, 150};   // far from the ball, so control stays with #10
  m = parked(at, 3);   // a slide the wrong way wins nothing: half a second of getting up, slowly
  step(m, tiltAt({1, 0}), true);
  CHECK(m.pl[0].slideMs > 0 && fabsf(len(m.pl[0].v) - SLIDE_SPEED) < 0.01f);
  stepN(m, SLIDE_MS / STEP_MS + 1, tiltAt({1, 0}));
  CHECK(m.pl[0].slowMs > 0 && fabsf(len(m.pl[0].v) - SLOW_SPEED) < 0.01f);
  memcpy(at, HOME, sizeof at);
  at[0] = {0, -130};   // right in front of coral's keeper, who holds the ball: pushed back, and cannot take it
  m = parked(at, CORAL_KEEPER);
  step(m, {0, 0}, false);
  CHECK(m.ball.owner == CORAL_KEEPER && dist(m.pl[0].p, m.pl[CORAL_KEEPER].p) >= KEEPER_ROOM - 0.01f);
}

// A keeper throws up the court, never back past himself: the kid running down behind his own keeper is not thrown the
// ball into his own net.
static void keeperThrowsForward() {
  Vec at[PLAYERS];
  memcpy(at, HOME, sizeof at);
  at[0] = {40, HALF_H - PLAYER_R};                // #10 behind his keeper, by the goal line
  at[1] = {-100, -20}; at[3] = {-80, -20};        // #9 up the court, closely marked
  at[4] = {100, -150};
  Match m = parked(at, TEAL_KEEPER);
  for (int n = 0; n < 300 && m.scored < 0; n++) step(m, tiltAt({0, 1}), false);   // he keeps running at the line
  CHECK(m.scored < 0);
}

// At the whistle a teal attack plays on, up to EXTRA_MS; anything else ends the match there.
static void whistle() {
  Match m = parked(HOME, 0);
  m.ms = MATCH_MS - 20;
  stepN(m, 10, tiltAt({0, 1}));
  CHECK(!m.over && timeUp(m) && finished(m));
  for (int n = 0; n < 2000 && !m.over; n++) step(m, tiltAt({1, 0}), false);   // keeps it, running along
  CHECK(m.over && m.ms <= MATCH_MS + EXTRA_MS + STEP_MS);
  m = parked(HOME, 3);
  m.ms = MATCH_MS - 20;
  stepN(m, 3);
  CHECK(m.over);
}

// A teal kickoff: coral stay outside the centre circle until the ball moves, for KICKOFF_WAIT_MS at most.
static void kickoffWait() {
  Match m = start(4, 3);
  for (int n = 0; n < (int)(KICKOFF_WAIT_MS / STEP_MS) - 1; n++) {
    step(m, {0, 0}, false);
    for (int j = 3; j <= 4; j++) CHECK(len(m.pl[j].p) >= CIRCLE_R + PLAYER_R - 1);
  }
  CHECK(m.ball.owner == 0);
  stepN(m, 300);
  CHECK(m.ball.owner != 0);
}

// ---- the challenge: three ways to play, as bots deciding once per 40 ms frame
using Brain = void (*)(const Match&, Vec*, bool*);
// Without the ball every bot defends the same way: run at the ball, slide when a coral carrier is close.
static void chase(const Match& m, Vec* tilt, bool* tap) {
  const Player& k = m.pl[m.control];
  *tilt = tiltAt(unit(sub(m.ball.p, k.p)));
  *tap = m.ball.owner >= 3 && dist(k.p, m.ball.p) < 40;
}
static void idle(const Match&, Vec* tilt, bool* tap) { *tilt = {0, 0}; *tap = false; }
// Greedy: straight at the goal's middle with the ball, and tap as soon as that shoots.
static void greedy(const Match& m, Vec* tilt, bool* tap) {
  if (m.ball.owner != m.control) { chase(m, tilt, tap); return; }
  const Vec dir = unit(sub(THEIR_GOAL, m.ball.p));
  *tilt = tiltAt(dir);
  *tap = aim(m, m.control, dir).kind == Kick::SHOOT;
}
// Passing: with a defender close ahead, pass to the teammate when his lane is clear, else go round the defender; from
// closer in, shoot for the corner the keeper is not in.
static bool laneClear(const Match& m, Vec to) {
  for (int j = 3; j <= 4; j++) {
    const Vec a = m.ball.p, ab = sub(to, a), ap = sub(m.pl[j].p, a);
    float t = (ap.x * ab.x + ap.y * ab.y) / (ab.x * ab.x + ab.y * ab.y);
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    if (dist(m.pl[j].p, {a.x + ab.x * t, a.y + ab.y * t}) < 30) return false;
  }
  return true;
}
static void passer(const Match& m, Vec* tilt, bool* tap) {
  if (m.ball.owner != m.control) { chase(m, tilt, tap); return; }
  const Vec p = m.ball.p, fwd = unit(sub(THEIR_GOAL, p));
  const Vec corner = {m.pl[CORAL_KEEPER].p.x > 0 ? -36.0f : 36.0f, -(float)HALF_H};
  *tap = false;
  if (dist(p, THEIR_GOAL) < SHOOT_RANGE - 25) {
    *tilt = tiltAt(unit(sub(corner, p)));
    *tap = aim(m, m.control, unit(sub(corner, p))).kind == Kick::SHOOT;
    return;
  }
  const int d = dist(m.pl[3].p, p) < dist(m.pl[4].p, p) ? 3 : 4;
  const Vec toD = sub(m.pl[d].p, p);
  *tilt = tiltAt(fwd);
  if (len(toD) >= 80 || toD.x * fwd.x + toD.y * fwd.y <= 0) return;
  const Vec mate = m.pl[1 - m.control].p, toMate = unit(sub(mate, p));
  const Kick k = aim(m, m.control, toMate);
  if (laneClear(m, mate) && k.kind == Kick::PASS && k.to == 1 - m.control) { *tilt = tiltAt(toMate); *tap = true; return; }
  const Vec side = {-fwd.y, fwd.x}, away = side.x * toD.x + side.y * toD.y > 0 ? Vec{-side.x, -side.y} : side;
  *tilt = tiltAt(unit({fwd.x + away.x * 1.5f, fwd.y + away.y * 1.5f}));
}

// Where anything may be at any step: players on the court, the ball on it or in a goal's mouth, nothing NaN.
static bool sane(const Match& m) {
  for (const Player& p : m.pl)
    if (!(fabsf(p.p.x) <= HALF_W - PLAYER_R + 0.01f && fabsf(p.p.y) <= HALF_H - PLAYER_R + 0.01f)) return false;
  const Vec b = m.ball.p;
  const bool inMouth = fabsf(b.x) <= GOAL_HALF - BALL_R && fabsf(b.y) <= HALF_H + NET_DEPTH;
  return inMouth || (fabsf(b.x) <= HALF_W - BALL_R + 0.01f && fabsf(b.y) <= HALF_H - BALL_R + 0.01f);
}
struct Result { int goalsFor, goalsAgainst; uint32_t firstAgainstMs, ms; };
static Result playMatch(Brain brain, uint8_t level, uint32_t seed) {
  Match m = start(level, seed);
  Result r = {0, 0, 0, 0};
  for (int frames = 0; frames < 20000; frames++) {
    Vec tilt; bool tap;
    brain(m, &tilt, &tap);
    for (int s = 0; s < 4 && m.scored < 0 && !m.over; s++) { step(m, tilt, tap && s == 0); assert(sane(m)); }
    if (m.scored == CORAL && !r.firstAgainstMs) r.firstAgainstMs = m.ms;
    if (m.scored >= 0 && !finished(m)) kickoff(m, 1 - m.scored);
    else if (m.scored >= 0 || m.over) break;
  }
  r.goalsFor = m.score[TEAL]; r.goalsAgainst = m.score[CORAL]; r.ms = m.ms;
  if (!r.firstAgainstMs) r.firstAgainstMs = m.ms;   // never scored on: as long as the match
  return r;
}
struct Tally { float goalsFor, goalsAgainst, firstAgainstS, soonestS; int wins; };
static Tally series(Brain brain, uint8_t level) {
  constexpr int N = 30;
  Tally t = {0, 0, 0, 1e9f, 0};
  for (int s = 1; s <= N; s++) {
    const Result r = playMatch(brain, level, (uint32_t)s);
    t.goalsFor += r.goalsFor / (float)N; t.goalsAgainst += r.goalsAgainst / (float)N; t.wins += r.goalsFor > r.goalsAgainst;
    t.firstAgainstS += r.firstAgainstMs / 1000.0f / N;
    if (r.firstAgainstMs / 1000.0f < t.soonestS) t.soonestS = r.firstAgainstMs / 1000.0f;
  }
  return t;
}
// Thirty seeded matches per way of playing, at every level. The bounds are the design, not measurements: see the
// spec's "Challenge" section for the numbers they are set against. Thirty seeds are noisy, so from one level to the
// next passing may win two more and an idle player be scored on 0.1 a match less; the top level against the first is
// the strict "gets harder": passing wins fewer, and an idle player is scored on more.
static void challenge() {
  Tally prevP = {}, prevI = {}, firstP = {}, firstI = {};
  for (uint8_t level = 0; level < LEVELS; level++) {
    const Tally g = series(greedy, level), p = series(passer, level), i = series(idle, level);
    printf("level %d: greedy %.2f-%.2f won %2d | passing %.2f-%.2f won %2d | idle scored on %.2f a match, first after %.0f s "
           "(soonest %.1f s)\n", level, g.goalsFor, g.goalsAgainst, g.wins, p.goalsFor, p.goalsAgainst, p.wins,
           i.goalsAgainst, i.firstAgainstS, i.soonestS);
    CHECK(g.goalsFor * 2 <= p.goalsFor && p.goalsFor - g.goalsFor >= 1.5f);   // straight at the goal: half as much...
    CHECK(g.wins <= 6);   // ...and no reliable win: one in five at most
    CHECK(p.wins >= 15);                          // playing well wins at every level, not always at the top
    CHECK(i.soonestS * 1000 >= KICKOFF_WAIT_MS + 1500);   // nobody is scored on before he could have moved
    CHECK(i.firstAgainstS >= 20);
    if (level) CHECK(p.wins <= prevP.wins + 2 && i.goalsAgainst >= prevI.goalsAgainst - 0.1f);   // no easier than the last
    if (!level) { firstP = p; firstI = i; }
    prevP = p; prevI = i;
  }
  CHECK(prevP.wins < firstP.wins && prevI.goalsAgainst > firstI.goalsAgainst);   // the top level is harder than the first
}

// ---- the save
static void saveBlob() {
  Save s{};
  s.wins = 12; s.level = 3;
  seal(s);
  Save back{};
  CHECK(loadBlob(&s, sizeof s, back) && back.wins == 12 && back.level == 3 && back.version == SAVE_VERSION);
  Save bad = s;
  bad.crc ^= 1;
  back.wins = 7;
  CHECK(!loadBlob(&bad, sizeof bad, back) && back.wins == 7);
  CHECK(!loadBlob(&s, sizeof s - 4, back) && !loadBlob(&s, sizeof s + 4, back) && !loadBlob(&s, 0, back));
  bad = s; bad.magic = 0x4B524D4D; bad.crc = os::crc32(&bad, sizeof bad - 4);   // another game's
  CHECK(!loadBlob(&bad, sizeof bad, back));
  bad = s; bad.version = SAVE_VERSION + 1; bad.crc = os::crc32(&bad, sizeof bad - 4);   // from a newer firmware
  CHECK(!loadBlob(&bad, sizeof bad, back) && back.wins == 7);
}

// ---- the game, driven as the sim drives it: 40 ms frames of real 1 g readings and a finger
static const Profile KID = {0, "Sam", 0, 6, 0};
struct Rig {
  Game game;
  InputTracker tracker;
  uint32_t ms = 1000;
  Grav grav = UPRIGHT;
  void enter(const SaveSlot& s) { game.enter({&KID, &KID, &s, 1, ms / 1000, ms}); }
  void frame(bool down, int x = 0, int y = 0) {
    Input in = tracker.step(down, x, y, ms);
    in.gx = (int16_t)grav.x; in.gy = (int16_t)grav.y; in.gz = (int16_t)grav.z;
    game.update(ms / 1000, ms, in);
    ms += 40;
  }
  void wait(int ms_) { for (int t = 0; t < ms_; t += 40) frame(false); }
  void tap(int x, int y) { frame(true, x, y); frame(true, x, y); frame(false, x, y); }   // the sim's `tap`: 80 ms
  bool on(const char* screen) const { return !strcmp(game.screenName(), screen); }
};
// Where the signs are, logical px: the leave sign on the left, the go sign on the Full time page.
constexpr int LEAVE_X = 17, LEAVE_Y = 80, GO_X = 80, FT_GO_Y = 125;
static void toMatch(Rig& r) {   // calibrated as held, through the Kickoff page's 3-2-1
  r.wait(200);
  r.game.debugCmd("kickoff");
  r.wait(KICKOFF_MS + 40);
}
static void toFullTime(Rig& r, const char* score) {   // the score set, then the whistle (a teal attack plays on)
  toMatch(r);
  r.game.debugCmd(score);
  r.game.debugCmd("clock0");
  for (int f = 0; f < 400 && !r.on("fc_fulltime"); f++) r.frame(false);
}
static bool saved(Rig& r, Save* s) {
  const void* data; size_t n;
  return r.game.takeSave(&data, &n, true) && loadBlob(data, n, *s);
}
// Full time counts a win and raises the level; a loss by two lowers it (never below 0); the save carries both.
static void fullTimeSaves() {
  Rig r;
  r.enter({nullptr, 0});
  toFullTime(r, "score2-0");
  CHECK(r.on("fc_fulltime"));
  Save s;
  CHECK(saved(r, &s) && s.wins == 1 && s.level == 1);
  CHECK(!saved(r, &s));   // nothing new since
  static Save blob;
  blob = s;
  Rig again;
  again.enter({&blob, sizeof blob});
  again.wait(200);
  again.game.debugCmd("kickoff");
  CHECK(again.game.match().level == 1);
  again.wait(KICKOFF_MS + 40);
  again.game.debugCmd("score0-2");
  again.game.debugCmd("clock0");
  again.wait(EXTRA_MS + 400);
  CHECK(again.on("fc_fulltime") && saved(again, &s) && s.level == 0 && s.wins == 1);
}

// The result counts the moment the match is decided, once: a third goal is saved while GOAL! still shows, so leaving
// then (or the shell closing the game) keeps the win, and Full time after it adds nothing.
static bool botToGoal(Rig& r) {   // the passing bot plays until a goal, deciding every 5 frames
  for (int f = 0; f < 4000 && !r.on("fc_goal"); f += 5) {
    Vec t = {0, 0};
    bool tap = false;
    if (r.on("fc_match")) passer(r.game.match(), &t, &tap);
    r.grav = gravityFor((int)lroundf(t.x), (int)lroundf(t.y), UPRIGHT);
    if (tap) { r.tap(130, 40); r.frame(false); r.frame(false); continue; }
    for (int k = 0; k < 5; k++) r.frame(false);
  }
  r.grav = UPRIGHT;
  return r.on("fc_goal");
}
static void decidedOnce() {
  Rig r;
  r.enter({nullptr, 0});
  toMatch(r);
  r.game.debugCmd("score2-0");
  CHECK(botToGoal(r) && r.game.match().score[TEAL] == 3);
  Save s;
  CHECK(saved(r, &s) && s.wins == 1 && s.level == 1);   // on the Goal page already
  r.wait(GOAL_MS + 80);
  CHECK(r.on("fc_fulltime") && !saved(r, &s));
  Rig left;   // the same, leaving while GOAL! shows
  left.enter({nullptr, 0});
  toMatch(left);
  left.game.debugCmd("score2-0");
  CHECK(botToGoal(left));
  bool gone = false;
  for (int f = 0; f < 20 && !gone; f++) { left.frame(true, LEAVE_X, LEAVE_Y); gone = left.game.wantsHome(); }
  CHECK(gone && left.on("fc_goal") && saved(left, &s) && s.wins == 1);
  Rig whistled;   // a hold that completes on the very frame of the whistle: the frame is played, the win saved
  whistled.enter({nullptr, 0});
  toMatch(whistled);
  whistled.game.debugCmd("score1-0");
  for (int f = 0; f < 30; f++) {   // up to the last frame before the hold would leave
    Rig probe = whistled;
    probe.frame(true, LEAVE_X, LEAVE_Y);
    if (probe.game.wantsHome()) break;
    whistled.frame(true, LEAVE_X, LEAVE_Y);
  }
  whistled.game.debugCmd("clock-10");   // the extra time all gone: the whistle on the next step
  whistled.frame(true, LEAVE_X, LEAVE_Y);
  CHECK(whistled.game.wantsHome() && saved(whistled, &s) && s.wins == 1 && s.level == 1);
}

// Kickoff, Match and Goal are one court: no fresh-page pause between them, so a press at the first moment of the match
// plays and a hold on the leave sign carries on from one to the next. The hold counts from when the finger is on the
// sign: a press that drifts off and back starts over, and one that starts on the court never leaves.
static void firstTapPlays() {
  Rig r;
  r.enter({nullptr, 0});
  r.wait(200);
  r.game.debugCmd("kickoff");
  for (int f = 0; f < 200 && r.on("fc_kickoff"); f++) r.frame(false);
  r.frame(true, 130, 40);   // the first frame of the match
  r.frame(false, 130, 40);
  CHECK(r.game.match().ball.owner != 0 && r.game.match().ball.passTo == 1);
}
static void holdCarriesOn() {
  Rig h;
  h.enter({nullptr, 0});
  h.wait(200);
  h.game.debugCmd("kickoff");
  h.wait(KICKOFF_MS - 280);
  int f = 0;
  for (; f < 30 && !h.game.wantsHome(); f++) h.frame(true, LEAVE_X, LEAVE_Y);   // over the page change
  CHECK(h.on("fc_match") && f * 40 >= (int)ui::HOLD_MS && f * 40 <= (int)ui::HOLD_MS + 80);
}
static void holdCountsOnTheSign() {
  Rig d;
  int f = 0;
  d.enter({nullptr, 0});
  toMatch(d);
  for (int k = 0; k < 10; k++) d.frame(true, LEAVE_X, LEAVE_Y);   // 400 ms on the sign
  for (int k = 0; k < 5; k++) d.frame(true, 45, 80);              // drifts off
  for (f = 0; f < 30 && !d.game.wantsHome(); f++) d.frame(true, LEAVE_X, LEAVE_Y);   // and back: from the start
  CHECK(f * 40 >= (int)ui::HOLD_MS && f * 40 <= (int)ui::HOLD_MS + 80);
  d.frame(false);
  d.enter({nullptr, 0});
  toMatch(d);
  for (int k = 0; k < 2; k++) d.frame(true, 45, 80);   // down on the court, then onto the sign
  for (f = 0; f < 30 && !d.game.wantsHome(); f++) d.frame(true, LEAVE_X, LEAVE_Y);
  CHECK(f == 30 && d.on("fc_match"));
}

// Mashing does not skip what the kid should see: the Goal page ignores taps and moves on after GOAL_MS; Full time
// moves on only by the go sign, once the result has shown for FT_READY_MS. The go sign and the leave sign off the
// match take a tap or a long press let go on them.
static void noSkipping() {
  Rig r;
  r.enter({nullptr, 0});
  toMatch(r);
  CHECK(botToGoal(r));
  for (int k = 0; k < 10; k++) { r.tap(130, 40); r.frame(false); }   // 1.6 s of taps
  CHECK(r.on("fc_goal"));
  r.wait(GOAL_MS);
  CHECK(r.on("fc_kickoff"));
  Rig ft;
  ft.enter({nullptr, 0});
  toFullTime(ft, "score2-0");
  CHECK(ft.on("fc_fulltime"));
  for (int k = 0; k < 6; k++) { ft.tap(130, 40); ft.frame(false); }
  ft.tap(GO_X, FT_GO_Y);   // too soon, even on the sign
  CHECK(ft.on("fc_fulltime"));
  for (int k = 0; k < 20; k++) ft.frame(true, GO_X, FT_GO_Y);   // down on the unlit sign, let go once it is lit
  ft.frame(false, GO_X, FT_GO_Y);
  CHECK(ft.on("fc_fulltime"));
  ft.wait(FT_READY_MS);
  for (int k = 0; k < 20; k++) ft.frame(true, GO_X, FT_GO_Y);   // a long press, let go on the sign
  ft.frame(false, GO_X, FT_GO_Y);
  CHECK(ft.on("fc_calibrate"));
  ft.wait(600);
  for (int k = 0; k < 20; k++) ft.frame(true, LEAVE_X, LEAVE_Y);
  CHECK(!ft.game.wantsHome());
  ft.frame(false, LEAVE_X, LEAVE_Y);
  CHECK(ft.game.wantsHome());
}

// Every string drawn is in the font's glyphs (a glyph outside them draws nothing), and fits on a shirt or a tile.
static void strings() {
  const char* all[PLAYERS + 2] = {paint::GOAL_WORD, "0123456789"};
  for (int i = 0; i < PLAYERS; i++) all[2 + i] = paint::NUMBERS[i];
  for (const char* s : all)
    for (const char* c = s; *c; c++) CHECK(strchr(GLYPHS, *c) && FONT18.glyphs[*c - 31].boxW > 0);
  for (const char* s : paint::NUMBERS) CHECK(font::textWidth(FONT18, s) <= 2 * 15 + 2);   // the shirt, 30 px across
  CHECK(font::textWidth(FONT64, paint::GOAL_WORD) <= 2 * (HALF_W - 3));
}

// ---- drawing: every frame drawn into the panel's two buffers by turns, repainting only what moved, equals the frame
// painted whole. The drive: random tilts and jolts, taps, holds and brushes anywhere and on the signs, frames of 0 ms
// and stalls of 500 ms, and from time to time a match the passing bot plays (goals, the Goal page), or one sent
// straight to its whistle (Full time); then a match the kid carries through the extra time to Full time, tilting on.
struct Driver {
  Rig rig;
  uint32_t rng = 4242;
  bool down = false;
  int x = 0, y = 0, frames = 0, bot = 0;
  int rand(int n) { rng = rng * 1664525u + 1013904223u; return (int)((rng >> 8) % (uint32_t)n); }
  void touch() {
    if (down) { if (--frames <= 0) down = false; return; }
    if (rand(25)) return;
    static const int SPOTS[][2] = {{17, 80}, {80, 116}, {80, 125}, {130, 40}, {50, 50}, {110, 110}};
    const int k = rand(8);
    x = k < 6 ? SPOTS[k][0] : 20 + rand(120); y = k < 6 ? SPOTS[k][1] : 20 + rand(120);
    down = true; frames = 1 + rand(3) + (rand(5) ? 0 : 20);   // mostly taps, sometimes a hold past 600 ms
  }
  void tilt() {
    if (bot > 0 && rig.on("fc_match")) {
      Vec t; bool tap;
      passer(rig.game.match(), &t, &tap);
      rig.grav = gravityFor((int)lroundf(t.x), (int)lroundf(t.y), UPRIGHT);
      if (tap && !down) { down = true; frames = 2; x = 130; y = 40; }
      return;
    }
    if (!rand(15)) rig.grav = rand(12) ? gravityFor(rand(1200) - 600, rand(1200) - 600, UPRIGHT) : Grav{1800, 0, 900};
  }
  void maybeStart() {   // now and then a match: the bot plays it, or it is sent to its whistle
    if (--bot > 0 || rand(250)) return;
    rig.grav = UPRIGHT;
    rig.frame(false);
    rig.game.debugCmd("kickoff");
    bot = 1500;
    if (rand(3)) return;
    rig.game.debugCmd(rand(2) ? "score2-1" : "score0-2");
    rig.game.debugCmd("clock2");
  }
  void frame() {
    if (bot > 0 && (rig.on("fc_calibrate") || rig.on("fc_fulltime"))) bot = 0;   // play is over: the finger is back
    maybeStart();
    tilt();
    if (bot <= 0) touch();
    const uint32_t dt = bot > 0 || rand(10) ? 40 : rand(5) ? 0 : 500;
    rig.ms += dt;
    rig.ms -= 40;   // Rig::frame steps 40 on its own
    rig.frame(down, x, y);
    if (down && bot > 0 && --frames <= 0) down = false;
  }
};
static bool drawnRight(Game& game, uint16_t* shown, uint16_t* whole) {
  gfx565::target(shown);
  game.render();
  Game copy = game;
  gfx565::target(whole);
  copy.render();
  if (!memcmp(shown, whole, gfx565::W * gfx565::H * 2)) return true;
  int x0 = 999, y0 = 999, x1 = -1, y1 = -1;
  for (int i = 0; i < gfx565::W * gfx565::H; i++)
    if (shown[i] != whole[i]) { const int x = i % gfx565::W, y = i / gfx565::W; x0 = x < x0 ? x : x0; y0 = y < y0 ? y : y0; x1 = x > x1 ? x : x1; y1 = y > y1 ? y : y1; }
  printf("  on %s: pixels differ in %d,%d-%d,%d\n", game.screenName(), x0, y0, x1, y1);
  memcpy(shown, whole, gfx565::W * gfx565::H * 2);   // report each mistake once
  return false;
}
// The kid carries the ball through the extra time to Full time, then tilts back and forth there: the frames drawn wrong.
static int tiltingAtFullTime(Rig& r, uint16_t (*bufs)[gfx565::W * gfx565::H], uint16_t* whole) {
  int bad = 0;
  r.grav = UPRIGHT;
  r.enter({nullptr, 0});
  toMatch(r);
  r.game.debugCmd("clock-9");   // a second of extra time left, teal on the ball
  for (int f = 0; f < 60 && !r.on("fc_fulltime"); f++) { r.frame(false); bad += !drawnRight(r.game, bufs[f & 1], whole); }
  CHECK(r.on("fc_fulltime") && r.game.match().ball.owner == 0);
  for (int f = 0; f < 40; f++) {
    r.grav = gravityFor(f % 4 < 2 ? -150 : 300, f % 4 < 2 ? -300 : 0, UPRIGHT);
    r.frame(false);
    if (!drawnRight(r.game, bufs[f & 1], whole) && bad++ < 5) printf("Full time frame %d: the incremental frame differs\n", f);
  }
  return bad;
}
static void incrementalEqualsFull() {
  static const char* const PAGES[Game::SC_COUNT] = {"fc_calibrate", "fc_kickoff", "fc_match", "fc_goal", "fc_fulltime"};
  static uint16_t bufs[2][gfx565::W * gfx565::H], whole[gfx565::W * gfx565::H];
  for (auto& b : bufs) for (uint16_t& p : b) p = 0xF81F;
  static Driver d;
  const SaveSlot none = {nullptr, 0};
  d.rig.enter(none);
  int seen[Game::SC_COUNT] = {}, bad = 0;
  for (int f = 0; f < 5000; f++) {
    d.frame();
    if (d.rig.game.wantsHome()) { d.rig.game.leave(); d.rig.enter(none); d.bot = 0; }
    for (int s = 0; s < Game::SC_COUNT; s++) seen[s] += d.rig.on(PAGES[s]);
    if (!drawnRight(d.rig.game, bufs[f & 1], whole) && bad++ < 5) printf("frame %d: the incremental frame differs\n", f);
  }
  bad += tiltingAtFullTime(d.rig, bufs, whole);
  printf("render check: %d frames differ; frames per page %d %d %d %d %d\n", bad, seen[0], seen[1], seen[2], seen[3], seen[4]);
  CHECK(bad == 0);
  for (int s : seen) CHECK(s > 20);   // every page was drawn both ways
}

// ---- the playtests' recorded matches: from `dbg kickoff` held at a grip, the passing bot deciding every DECIDE
// frames until teal scores, as sim lines (tilt, wait, tap) the sim replays frame for frame. Every block in
// tests/playtests/*.txt from "# match from <x> <y> <z>" to "# end match" must be exactly what this writes, so a change to
// the rules fails here until `build/host/test_tiltfc --write-playtests` rewrites them.
constexpr int DECIDE = 5;
static std::string record(Grav n) {
  Rig r;
  r.grav = n;
  r.enter({nullptr, 0});
  r.wait(80);
  r.game.debugCmd("kickoff");
  std::string out;
  char line[64];
  Grav said = n;
  int waited = 0;
  auto flush = [&] { if (waited) { snprintf(line, sizeof line, "wait %d\n", waited * 40); out += line; waited = 0; } };
  for (int f = 0; f < 4000 && !r.on("fc_goal"); f += DECIDE) {
    Vec t = {0, 0};
    bool tap = false;
    if (r.on("fc_match")) passer(r.game.match(), &t, &tap);
    const Grav g = gravityFor((int)lroundf(t.x), (int)lroundf(t.y), n);
    if (g.x != said.x || g.y != said.y || g.z != said.z) {
      flush();
      snprintf(line, sizeof line, "tilt %d %d %d\n", g.x, g.y, g.z);
      out += line;
      said = r.grav = g;
    }
    if (tap) { flush(); out += "tap 130 40\n"; r.tap(130, 40); for (int k = 3; k < DECIDE; k++) r.frame(false); waited += DECIDE - 3; continue; }
    for (int k = 0; k < DECIDE; k++) r.frame(false);
    waited += DECIDE;
  }
  flush();
  snprintf(line, sizeof line, "tilt %d %d %d\n", n.x, n.y, n.z);   // back to the grip
  const bool scored = r.on("fc_goal") && r.game.match().score[TEAL] == 1;
  if (!scored) printf("test_tiltfc: the recorded match from %d %d %d did not end in a teal goal\n", n.x, n.y, n.z);
  return scored ? out + line : "";
}
static std::string fixBlocks(const std::string& text, int* seen, int* stale) {
  std::string out;
  size_t at = 0;
  while (at < text.size()) {
    const size_t eol = text.find('\n', at), next = eol == std::string::npos ? text.size() : eol + 1;
    const std::string line = text.substr(at, next - at);
    out += line;
    at = next;
    Grav n;
    if (sscanf(line.c_str(), "# match from %d %d %d", &n.x, &n.y, &n.z) != 3) continue;
    const size_t end = text.find("# end match", at);
    CHECK(end != std::string::npos && mag(n) > 900);
    ++*seen;
    const std::string want = record(n);
    CHECK(!want.empty());
    *stale += text.compare(at, end - at, want) != 0;
    out += want;
    at = end;
  }
  return out;
}
static void playtestMatches(bool write) {
  int seen = 0, stale = 0;
  DIR* d = opendir("tests/playtests");
  if (!d) printf("test_tiltfc: no tests/playtests here (run from apps/porthole)\n");
  CHECK(d);
  while (dirent* e = readdir(d)) {
    const std::string path = std::string("tests/playtests/") + e->d_name;
    if (path.size() < 4 || path.compare(path.size() - 4, 4, ".txt")) continue;
    FILE* f = fopen(path.c_str(), "rb");
    CHECK(f);
    std::string text;
    for (int c; (c = fgetc(f)) != EOF;) text += (char)c;
    fclose(f);
    const int before = stale;
    const std::string fixed = fixBlocks(text, &seen, &stale);
    if (!write || stale == before) continue;
    f = fopen(path.c_str(), "wb");
    CHECK(f);
    fputs(fixed.c_str(), f);
    fclose(f);
    printf("rewrote %s\n", path.c_str());
  }
  closedir(d);
  if (stale && !write) printf("test_tiltfc: %d playtest match blocks are stale: run build/host/test_tiltfc --write-playtests\n", stale);
  CHECK(write || stale == 0);
  CHECK(seen >= 2);
}

int main(int argc, char** argv) {
  if (argc > 1 && !strcmp(argv[1], "--write-playtests")) { playtestMatches(true); return 0; }
  deadZone();
  passCone();
  controlSwitch();
  goals();
  kickoffReset();
  tackles();
  keeperThrowsForward();
  whistle();
  kickoffWait();
  levelTap();
  challenge();
  saveBlob();
  fullTimeSaves();
  decidedOnce();
  firstTapPlays();
  holdCarriesOn();
  holdCountsOnTheSign();
  noSkipping();
  strings();
  incrementalEqualsFull();
  playtestMatches(false);
  printf("test_tiltfc: all %d checks passed (sizeof Save = %zu)\n", checks, sizeof(Save));
  return 0;
}
