#include "match.h"
#include <math.h>

namespace fc {
namespace {
constexpr float DT = STEP_MS / 1000.0f, PI = 3.14159265f;
constexpr float LEAD_S = 0.25f;     // a pass goes where the receiver will be this much later
constexpr float LANE_T = 0.55f;     // the covering defender stands this far along the lane from the ball to its target
constexpr float PRESSED_PX = 58;    // a carrier with an opponent this close ahead passes or bends away
constexpr float PASS_CHANCE = 0.06f;   // per step, once pressed with an open lane: a coral pass comes within ~0.2 s
constexpr float MATE_ACCEL = 900;   // teal teammates turn quicker than coral: they are on the kid's side

Vec add(Vec a, Vec b) { return {a.x + b.x, a.y + b.y}; }
Vec sub(Vec a, Vec b) { return {a.x - b.x, a.y - b.y}; }
Vec mul(Vec a, float k) { return {a.x * k, a.y * k}; }
float dot(Vec a, Vec b) { return a.x * b.x + a.y * b.y; }
float len(Vec a) { return sqrtf(dot(a, a)); }
float dist(Vec a, Vec b) { return len(sub(a, b)); }
Vec unit(Vec a, Vec otherwise) { const float l = len(a); return l > 0.001f ? mul(a, 1 / l) : otherwise; }
float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
uint16_t down(uint16_t t) { return t > STEP_MS ? (uint16_t)(t - STEP_MS) : 0; }
float frand(Match& m) { m.rng = m.rng * 1664525u + 1013904223u; return (float)(m.rng >> 8) / 16777216.0f; }

Vec goalOf(int team) { return {0, team == TEAL ? -(float)HALF_H : (float)HALF_H}; }   // the goal a team attacks
Vec ownGoal(int team) { return goalOf(1 - team); }
Vec forward(int team) { return {0, team == TEAL ? -1.0f : 1.0f}; }
int mateOf(int i) { return i == 0 ? 1 : i == 1 ? 0 : i == 3 ? 4 : 3; }   // an outfield player's partner
int keeperOf(int team) { return team == TEAL ? TEAL_KEEPER : CORAL_KEEPER; }
int firstOutfield(int team) { return team == TEAL ? 0 : 3; }

// Keeps a round body of radius r inside the court (a rectangle with round corners); true, with the wall's outward
// normal, when it had to.
bool wall(Vec& p, float r, Vec* n) {
  const float ix = HALF_W - CORNER_R, iy = HALF_H - CORNER_R, lim = CORNER_R - r;
  const float qx = fabsf(p.x) - ix, qy = fabsf(p.y) - iy, sx = p.x < 0 ? -1.0f : 1.0f, sy = p.y < 0 ? -1.0f : 1.0f;
  if (qx > 0 && qy > 0) {   // in a corner's square: the round part
    const float d = sqrtf(qx * qx + qy * qy);
    if (d <= lim) return false;
    *n = {sx * qx / d, sy * qy / d};
    p = {sx * (ix + qx / d * lim), sy * (iy + qy / d * lim)};
    return true;
  }
  if (qx > lim) { p.x = sx * (ix + lim); *n = {sx, 0}; return true; }
  if (qy > lim) { p.y = sy * (iy + lim); *n = {0, sy}; return true; }
  return false;
}
// A velocity toward `to` at `speed`, easing off over the last few px so a player settles instead of jittering.
Vec toward(Vec from, Vec to, float speed) {
  const Vec d = sub(to, from);
  const float l = len(d);
  if (l < 0.5f) return {0, 0};
  return mul(d, (l < 12 ? speed * l / 12 : speed) / l);
}
// Turns a player's velocity toward `want` by at most `accel` * DT: weight, so a sidestep can beat him.
void run(Player& p, Vec to, float speed, float accel) {
  const Vec want = toward(p.p, to, p.slowMs ? SLOW_SPEED : speed), d = sub(want, p.v);
  const float l = len(d), most = accel * DT;
  p.v = l <= most ? want : add(p.v, mul(d, most / l));
}
float speedOf(const Match& m, int i) { return teamOf(i) == TEAL ? MATE_SPEED : DEF_SPEED[m.level]; }
float accelOf(const Match& m, int i) { return teamOf(i) == TEAL ? MATE_ACCEL : DEF_ACCEL[m.level]; }

// Kicks: the ball leaves `who` toward `at` at `speed`.
void kick(Match& m, int who, Vec at, float speed, int passTo) {
  Ball& b = m.ball;
  b.v = mul(unit(sub(at, b.p), m.pl[who].face), speed);
  b.owner = -1; b.passTo = (int8_t)passTo; b.kicker = (int8_t)who; b.heldMs = 0;
  m.pl[who].coolMs = KICK_COOL_MS;
  m.reactMs = (uint16_t)(frand(m) * REACT_SPREAD_MS);
}
void play(Match& m, int who, const Kick& k) {
  const float speed = k.kind == Kick::SHOOT ? SHOT_SPEED : k.kind == Kick::PASS ? PASS_SPEED : ROLL_SPEED;
  kick(m, who, k.at, speed, k.kind == Kick::PASS ? k.to : -1);
}
// Where a shot along `dir` from `from` crosses the goal line at `gy`, inside the posts.
Vec shotSpot(Vec from, Vec dir, float gy) {
  const float inside = GOAL_HALF - BALL_R - 5;
  const float x = fabsf(dir.y) > 0.05f && (gy - from.y) * dir.y > 0 ? from.x + dir.x * (gy - from.y) / dir.y : 0;
  return {clampf(x, -inside, inside), gy};
}
// No opponent outfield player within reach of the line from the ball to `to`.
bool laneOpen(const Match& m, Vec to, int team) {
  const Vec a = m.ball.p, ab = sub(to, a);
  const float l2 = dot(ab, ab);
  for (int j = firstOutfield(1 - team), k = 0; k < 2; j++, k++) {
    const float t = l2 > 0 ? clampf(dot(sub(m.pl[j].p, a), ab) / l2, 0, 1) : 0;
    if (dist(m.pl[j].p, add(a, mul(ab, t))) < PLAYER_R + BALL_R + 4) return false;
  }
  return true;
}
int nearestOutfield(const Match& m, int team, Vec to) {
  const int a = firstOutfield(team);
  return dist(m.pl[a].p, to) <= dist(m.pl[a + 1].p, to) ? a : a + 1;
}

int nearestOpponent(const Match& m, int i) {   // keeper included
  int best = -1;
  for (int j = 0; j < PLAYERS; j++)
    if (teamOf(j) != teamOf(i) && (best < 0 || dist(m.pl[j].p, m.pl[i].p) < dist(m.pl[best].p, m.pl[i].p))) best = j;
  return best;
}

// ---- the players nobody steers
// Get open for the carrier (a teammate or the keeper): wide on the other side from him and further up the court.
Vec openSpot(const Match& m, int i, Vec carrier) {
  const int t = teamOf(i);
  const float side = carrier.x > 8 ? -1.0f : carrier.x < -8 ? 1.0f : m.pl[i].p.x < 0 ? -1.0f : 1.0f;
  const float y = carrier.y + forward(t).y * 95;
  return {side * 78, clampf(y, -(HALF_H - 70.0f), HALF_H - 70.0f)};
}
// A teal teammate without the ball marks the opponent further from it, goal side: the kid's player goes for the ball.
Vec markSpot(const Match& m, int i) {
  const int t = teamOf(i), a = firstOutfield(1 - t);
  const int o = dist(m.pl[a].p, m.ball.p) > dist(m.pl[a + 1].p, m.ball.p) ? a : a + 1;
  return sub(m.pl[o].p, mul(forward(t), 34));
}
// A coral defender who is not pressing covers: in the lane from the carrier to his partner, or when the ball is loose,
// between it and their goal.
Vec coverSpot(const Match& m, int i) {
  const Ball& b = m.ball;
  if (b.owner >= 0) return add(b.p, mul(sub(m.pl[mateOf(b.owner)].p, b.p), LANE_T));
  return add(b.p, mul(sub(ownGoal(teamOf(i)), b.p), 0.35f));
}
Vec offBallSpot(const Match& m, int i) {
  const Ball& b = m.ball;
  const int t = teamOf(i);
  if (b.owner >= 0 && teamOf(b.owner) == t) return openSpot(m, i, m.pl[b.owner].p);
  if (b.owner == keeperOf(1 - t)) return {i % 2 ? -60.0f : 60.0f, ownGoal(t).y * 0.35f};   // drop back
  if (t == TEAL) return markSpot(m, i);
  if (nearestOutfield(m, t, b.p) != i) return coverSpot(m, i);
  return b.owner >= 0 ? add(b.p, mul(m.pl[b.owner].v, 0.2f)) : b.p;   // press: at the ball, a little ahead of it
}
// A carrier the kid is not steering (coral's): shoot in range or after holding it long enough, pass when pressed and
// his partner's lane is open, else run at the goal, bending away from a defender in the way.
void aiShoot(Match& m, int i) {
  const Vec g = goalOf(teamOf(i));
  const float keeperX = m.pl[keeperOf(1 - teamOf(i))].p.x, corner = GOAL_HALF - 14.0f;
  const float x = (keeperX > 0 ? -corner : corner) + (frand(m) - 0.5f) * 2 * AI_SHOT_MISS[m.level];
  kick(m, i, {x, g.y}, AI_SHOT_SPEED[m.level], -1);
}
// Coral shoot from in front of the goal (not from beside a post, along the line into the keeper): within range and no
// wider than the goal's middle is far.
bool inShootingZone(Vec p, Vec g) { return dist(p, g) < AI_SHOOT_RANGE && fabsf(p.x) < fabsf(g.y - p.y) + 10; }
void carry(Match& m, int i) {
  Player& c = m.pl[i];
  const int t = teamOf(i);
  const Vec g = goalOf(t), spot = {clampf(c.p.x * 0.5f, -40, 40), g.y - forward(t).y * (AI_SHOOT_RANGE - 40)};
  if (inShootingZone(c.p, g) || m.ball.heldMs > AI_HOLD_MS) { aiShoot(m, i); return; }
  const Vec fwd = unit(sub(spot, c.p), forward(t));
  const Vec toD = sub(m.pl[nearestOpponent(m, i)].p, c.p);
  const bool pressed = len(toD) < PRESSED_PX && dot(unit(toD, fwd), fwd) > 0.2f;
  const int mate = mateOf(i);
  if (pressed && m.ball.heldMs > 300 && laneOpen(m, m.pl[mate].p, t) && frand(m) < PASS_CHANCE) {
    kick(m, i, add(m.pl[mate].p, mul(m.pl[mate].v, LEAD_S)), PASS_SPEED, mate);
    return;
  }
  Vec want = fwd;
  if (pressed) {   // bend away from him
    const Vec side = {-fwd.y, fwd.x};
    want = unit(add(fwd, mul(dot(side, toD) > 0 ? mul(side, -1) : side, 1.4f)), fwd);
  }
  run(c, add(c.p, mul(want, 40)), speedOf(m, i) * 0.92f, accelOf(m, i));
}
// A caught ball goes out to the teammate with the most room whose lane is open, once one has room; else, after a
// while, it is rolled out wide on the side away from the nearest opponent.
constexpr float THROW_ROOM = 45;   // px from the receiver to his nearest opponent, at least
void throwOut(Match& m, int k) {
  const int t = teamOf(k), them = firstOutfield(1 - t);
  int to = -1;
  float room = THROW_ROOM;
  for (int j = firstOutfield(t), n = 0; n < 2; j++, n++) {
    const Vec p = m.pl[j].p;
    const float r = fminf(dist(p, m.pl[them].p), dist(p, m.pl[them + 1].p));
    if (r > room && laneOpen(m, p, t)) { room = r; to = j; }
  }
  if (to >= 0) { kick(m, k, add(m.pl[to].p, mul(m.pl[to].v, LEAD_S)), PASS_SPEED * 0.9f, to); return; }
  if (m.ball.heldMs < 2 * KEEPER_HOLD_MS) return;
  const float side = m.pl[nearestOpponent(m, k)].p.x > m.pl[k].p.x ? -1.0f : 1.0f;
  kick(m, k, add({side * 110, m.pl[k].p.y}, mul(forward(t), 110)), PASS_SPEED * 0.9f, -1);
}
// Keepers stay on their line: level with the ball (half as far across), and once a shot has been in the air for their
// reaction time, where it will cross the line. A caught ball is thrown to the most open teammate.
void keeper(Match& m, int i) {
  Player& k = m.pl[i];
  const Ball& b = m.ball;
  const int t = teamOf(i);
  k.face = forward(t);
  if (b.owner == i) {
    k.v = {0, 0};
    if (b.heldMs >= KEEPER_HOLD_MS) throwOut(m, i);
    return;
  }
  const float ky = ownGoal(t).y > 0 ? (float)KEEPER_Y : -(float)KEEPER_Y;
  const bool teal = t == TEAL, incoming = b.owner < 0 && b.v.y * ky > 0 && fabsf(b.v.y) > 120;
  float x = b.p.x * 0.5f;
  if (incoming && b.heldMs < (teal ? TEAL_KEEPER_REACT_MS : KEEPER_REACT_MS[m.level]) + m.reactMs) x = k.p.x;   // not yet
  else if (incoming) x = b.p.x + b.v.x * (ky - b.p.y) / b.v.y;
  x = clampf(x, -(GOAL_HALF - 10.0f), GOAL_HALF - 10.0f);
  k.v = toward(k.p, {x, ky}, teal ? TEAL_KEEPER_SPEED : KEEPER_SPEED[m.level]);
}
// A kickoff: until the taker has moved the ball (or KICKOFF_WAIT_MS has gone by), the other team keeps out of the centre
// circle, so a kid still finding his grip is not robbed on the spot.
bool kickoffWait(const Match& m, int i) {
  const Ball& b = m.ball;
  return b.owner >= 0 && teamOf(b.owner) != teamOf(i) && len(b.p) < 1 && b.heldMs < KICKOFF_WAIT_MS;
}
Vec outsideCircle(Vec p) { return len(p) >= CIRCLE_R + PLAYER_R ? p : mul(unit(p, Vec{0, -1}), CIRCLE_R + PLAYER_R); }
void think(Match& m, int i) {
  const Ball& b = m.ball;
  if (isKeeper(i)) keeper(m, i);
  else if (b.owner == i) carry(m, i);
  else if (b.owner < 0 && b.passTo == i) run(m.pl[i], b.p, speedOf(m, i), accelOf(m, i));   // meet the pass
  else if (kickoffWait(m, i)) run(m.pl[i], outsideCircle(offBallSpot(m, i)), speedOf(m, i), accelOf(m, i));
  else run(m.pl[i], offBallSpot(m, i), speedOf(m, i), accelOf(m, i));
}

// ---- the kid's player
// The kid steers the teal outfield player with the ball, the one a teal pass is heading to, or else the one nearer
// the ball (by a margin, so control does not flicker between two players level with it).
void pickControl(Match& m) {
  const Ball& b = m.ball;
  int c = m.control;
  if (b.owner == 0 || b.owner == 1) c = b.owner;
  else if (b.owner < 0 && (b.passTo == 0 || b.passTo == 1)) c = b.passTo;
  else if (!m.pl[c].slideMs && dist(m.pl[1 - c].p, b.p) + SWITCH_MARGIN < dist(m.pl[c].p, b.p)) c = 1 - c;
  m.control = (int8_t)c;
}
void steerKid(Match& m, Vec tilt) {
  Player& k = m.pl[m.control];
  const Ball& b = m.ball;
  if (k.slideMs) return;   // a slide keeps its speed and direction
  Vec h = heading(tilt);
  if (b.owner < 0 && b.passTo == m.control) h = unit(sub(b.p, k.p), h);   // a pass to him: he runs to meet it himself
  k.v = mul(h, k.slowMs ? SLOW_SPEED : RUN_SPEED);
}
// With the ball: pass, shoot or roll it along the aim. Without: slide along the tilt (or where he faces).
void kidTap(Match& m, Vec tilt) {
  Player& k = m.pl[m.control];
  if (m.ball.owner == m.control) { play(m, m.control, aim(m, m.control, aimDir(m, tilt))); return; }
  if (k.slideMs || k.slowMs) return;
  k.face = aimDir(m, tilt);
  k.slideMs = SLIDE_MS;
  k.v = mul(k.face, SLIDE_SPEED);
}

// ---- the world
void timers(Match& m) {
  for (Player& p : m.pl) {
    p.coolMs = down(p.coolMs);
    p.slowMs = down(p.slowMs);
    if (p.slideMs && !(p.slideMs = down(p.slideMs))) p.slowMs = SLOW_MS;   // a slide that won nothing
  }
  Ball& b = m.ball;
  b.heldMs += STEP_MS;
  if (b.owner < 0 && b.passTo >= 0 && b.heldMs > PASS_MS) b.passTo = -1;
}
void separate(Match& m) {   // bodies do not overlap: two players touching push each other apart evenly
  for (int i = 0; i < PLAYERS; i++)
    for (int j = i + 1; j < PLAYERS; j++) {
      const Vec d = sub(m.pl[j].p, m.pl[i].p);
      const float l = len(d);
      if (l >= 2 * PLAYER_R) continue;
      const Vec n = l > 0.01f ? mul(d, 1 / l) : Vec{1, 0};
      const float push = (2 * PLAYER_R - l) / 2;
      m.pl[i].p = sub(m.pl[i].p, mul(n, push));
      m.pl[j].p = add(m.pl[j].p, mul(n, push));
    }
}
// Room the rules give the side on the ball: a keeper holding it (nobody camps on him and takes his throw), and a teal
// kickoff until the ball moves (kickoffWait). Opponents are kept out, whatever their run.
void room(Match& m) {
  const int k = m.ball.owner;
  if (k < 0) return;
  const bool keeper = isKeeper(k);
  if (!keeper && !kickoffWait(m, firstOutfield(1 - teamOf(k)))) return;
  const Vec c = keeper ? m.pl[k].p : Vec{0, 0};
  const float r = keeper ? KEEPER_ROOM : CIRCLE_R + PLAYER_R;
  for (int j = firstOutfield(1 - teamOf(k)), n = 0; n < 2; j++, n++) {
    const Vec d = sub(m.pl[j].p, c);
    if (len(d) < r) m.pl[j].p = add(c, mul(unit(d, forward(teamOf(k))), r));
  }
}
void move(Match& m) {
  for (int i = 0; i < PLAYERS; i++) {
    Player& p = m.pl[i];
    p.p = add(p.p, mul(p.v, DT));
    if (!isKeeper(i) && !p.slideMs && len(p.v) > 5) p.face = unit(p.v, p.face);
  }
  separate(m);
  room(m);
  Vec n;
  for (Player& p : m.pl) wall(p.p, PLAYER_R, &n);
}
// The ball off the walls; in front of a goal (between the posts) the end wall is open.
void ballWall(Ball& b) {
  if (fabsf(b.p.x) <= GOAL_HALF - BALL_R) return;
  Vec n;
  if (!wall(b.p, BALL_R, &n) || b.owner >= 0) return;
  const float vn = dot(b.v, n);
  if (vn > 0) b.v = sub(b.v, mul(n, (1 + BALL_BOUNCE) * vn));
}
void goalCheck(Match& m) {
  Ball& b = m.ball;
  if (fabsf(b.p.y) <= HALF_H) return;   // the whole ball past the line is not needed: its middle is enough
  m.scored = b.p.y < 0 ? TEAL : CORAL;
  m.score[m.scored]++;
  b.owner = b.passTo = -1;
  b.v = {0, 0};
  b.p.y = b.p.y < 0 ? -(HALF_H + NET_DEPTH / 2.0f) : HALF_H + NET_DEPTH / 2.0f;   // at rest in the net
}
void roll(Match& m) {
  Ball& b = m.ball;
  if (b.owner >= 0) {
    const Player& o = m.pl[b.owner];
    b.p = add(o.p, mul(o.face, FOOT));
    b.v = o.v;
  } else {
    const float s = len(b.v), lose = s * BALL_DRAG * DT + BALL_FRICTION * DT;
    b.v = s <= lose ? Vec{0, 0} : mul(b.v, (s - lose) / s);
    b.p = add(b.p, mul(b.v, DT));
  }
  ballWall(b);
  goalCheck(m);
}
float reach(const Match& m, int i) {   // a slide gets a foot further; a keeper has to be right there
  return isKeeper(i) ? KEEPER_REACH : m.pl[i].slideMs ? 8.0f : 0.0f;
}
// A loose ball goes to the nearest player touching it (one who may touch it).
void pickUp(Match& m) {
  Ball& b = m.ball;
  int best = -1;
  float bestD = 1e9f;
  for (int i = 0; i < PLAYERS; i++) {
    const float d = dist(m.pl[i].p, b.p);
    if (!m.pl[i].coolMs && d < PLAYER_R + BALL_R + reach(m, i) && d < bestD) { best = i; bestD = d; }
  }
  if (best < 0) return;
  b.owner = b.kicker = (int8_t)best;
  b.passTo = -1;
  b.heldMs = 0;
  m.pl[best].slideMs = 0;   // a slide that wins the ball: no getting up
}
// An opponent touching the carried ball knocks it loose, on along his run; the carrier cannot take it straight back.
// A keeper's ball in his hands is his.
void tackle(Match& m) {
  Ball& b = m.ball;
  const int o = b.owner;
  if (isKeeper(o)) return;
  for (int i = 0; i < PLAYERS; i++) {
    Player& t = m.pl[i];
    if (teamOf(i) == teamOf(o) || t.coolMs || dist(t.p, b.p) >= PLAYER_R + BALL_R + reach(m, i)) continue;
    m.pl[o].coolMs = LOSE_MS;
    b.v = mul(unit(len(t.v) > 5 ? t.v : sub(b.p, t.p), t.face), KNOCK_SPEED);
    b.owner = b.passTo = -1;
    b.kicker = (int8_t)i;
    b.heldMs = 0;
    t.slideMs = 0;
    t.coolMs = 120;
    return;
  }
}
// At the whistle a teal attack plays on (the ball at a teal outfield player's feet, or his kick still rolling), for
// EXTRA_MS at most: the clock never takes a chance away from the kid.
void whistle(Match& m) {
  if (m.scored >= 0 || !timeUp(m)) return;
  const Ball& b = m.ball;
  const int who = b.owner >= 0 ? b.owner : b.kicker;
  const bool attack = who >= 0 && teamOf(who) == TEAL && !isKeeper(who) && (b.owner >= 0 || b.heldMs < PASS_MS);
  if (!attack || m.ms >= MATCH_MS + EXTRA_MS) m.over = true;
}
}  // namespace

// Kickoff places with teal kicking off: the taker on the ball in the middle, his partner wide behind, coral in their
// half. Coral kicking off: the same places turned half round, player i where player (i + 3) % 6 stood.
constexpr Vec KICKOFF[PLAYERS] = {{0, FOOT}, {-80, 70}, {0, KEEPER_Y}, {-55, -70}, {55, -70}, {0, -KEEPER_Y}};
void kickoff(Match& m, int team) {
  for (int i = 0; i < PLAYERS; i++) {
    const Vec s = team == TEAL ? KICKOFF[i] : mul(KICKOFF[(i + 3) % PLAYERS], -1);
    m.pl[i] = {s, {0, 0}, forward(teamOf(i)), 0, 0, 0};
  }
  const int taker = firstOutfield(team);
  m.ball = {{0, 0}, {0, 0}, (int8_t)taker, -1, (int8_t)taker, 0};
  m.control = 0;
  m.scored = -1;
}
Match start(uint8_t level, uint32_t seed) {
  Match m{};
  m.level = level < LEVELS ? level : LEVELS - 1;
  m.rng = seed * 2654435761u + 1;
  kickoff(m, TEAL);
  return m;
}
Vec heading(Vec tilt) {
  const float l = len(tilt);
  return l <= DEAD_MG ? Vec{0, 0} : mul(tilt, 1 / l);
}
Vec aimDir(const Match& m, Vec tilt) {
  const Vec h = heading(tilt);
  return h.x != 0 || h.y != 0 ? h : m.pl[m.control].face;
}
// The goal in the cone and in range: a shot at where the aim crosses the goal line (inside the posts). Else a pass to
// the teammate (outfield or keeper) nearest the aim inside the cone. Else the ball rolls on ahead.
Kick aim(const Match& m, int who, Vec dir) {
  const int t = teamOf(who);
  const Vec from = m.ball.owner == who ? m.ball.p : m.pl[who].p, g = goalOf(t), toGoal = sub(g, from);
  const float cone = cosf(CONE_DEG * PI / 180);
  if (len(toGoal) <= SHOOT_RANGE && dot(unit(toGoal, dir), dir) >= cone) return {Kick::SHOOT, -1, shotSpot(from, dir, g.y)};
  const int mates[2] = {isKeeper(who) ? firstOutfield(t) : mateOf(who), keeperOf(t)};
  int to = -1;
  float best = cone;
  for (int j : mates) {
    const float c = dot(unit(sub(m.pl[j].p, from), dir), dir);
    if (j != who && c >= best) { best = c; to = j; }
  }
  if (to >= 0) return {Kick::PASS, (int8_t)to, add(m.pl[to].p, mul(m.pl[to].v, LEAD_S))};
  return {Kick::ROLL, -1, add(from, mul(dir, 100))};
}
void step(Match& m, Vec tilt, bool tap) {
  if (m.scored >= 0 || m.over) return;
  m.ms += STEP_MS;
  timers(m);
  pickControl(m);
  steerKid(m, tilt);
  if (tap) kidTap(m, tilt);
  for (int i = 0; i < PLAYERS; i++) if (i != m.control) think(m, i);
  move(m);
  roll(m);
  if (m.scored >= 0) return;
  if (m.ball.owner < 0) pickUp(m);
  else tackle(m);
  whistle(m);
}
}  // namespace fc
