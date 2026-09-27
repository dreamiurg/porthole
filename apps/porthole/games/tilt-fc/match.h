// Tilt FC's rules: two a side plus a keeper each on a street court, first to three or three minutes. No drawing, a
// fixed STEP_MS step and a seeded random source, so host/test_tiltfc.cpp plays whole matches as they happen on the glass.
//
// Teal (the kid's team) defends the bottom goal and attacks the top one; coral the other way round. The kid always
// steers one teal outfield player (`control`): the one with the ball, the one a teal pass is heading to, or else the
// one nearer the ball. Tilt sets his direction (never his speed); a tap passes or shoots with the ball (aim: the
// tilt, or where he faces) and slides for it without. Everyone else plays themselves: the other teal player gets open
// or marks, the coral pair press the ball and stand in the passing lane, the keepers cover the shot. Touching the
// ball an opponent is carrying knocks it loose; nobody falls, there are no fouls.
//
// Positions are panel px from the middle of the glass, +y down (render.cpp draws them where they are).
#pragma once
#include <stdint.h>
#include "tilt.h"
#include "tune.h"

namespace fc {
using Vec = tilt::Vec;
enum Team : uint8_t { TEAL, CORAL };
constexpr int PLAYERS = 6;     // 0-1 teal outfield, 2 teal keeper, 3-4 coral outfield, 5 coral keeper
constexpr int TEAL_KEEPER = 2, CORAL_KEEPER = 5;
inline int teamOf(int i) { return i < 3 ? TEAL : CORAL; }
inline bool isKeeper(int i) { return i == TEAL_KEEPER || i == CORAL_KEEPER; }

// The court: a rectangle with round corners that fits the round glass, a goal in the middle of each end.
constexpr int HALF_W = 150, HALF_H = 190, CORNER_R = 48;
constexpr int GOAL_HALF = 50, NET_DEPTH = 22;   // the goal's mouth, post to post, and how far its net goes back
constexpr int PLAYER_R = 16, BALL_R = 7;
constexpr int KEEPER_Y = HALF_H - PLAYER_R - 2;  // a keeper's line, just off his goal line
constexpr float FOOT = PLAYER_R + BALL_R - 2;    // a carried ball sits this far in front of its carrier

struct Player {
  Vec p, v, face;            // where, how fast, and which way he looks (a unit vector)
  uint16_t coolMs;           // cannot touch the ball (just kicked it, or just lost it)
  uint16_t slideMs, slowMs;  // sliding for the ball; getting up after a slide that won nothing
};
struct Ball {
  Vec p, v;
  int8_t owner;       // carried by this player, or -1: loose
  int8_t passTo;      // loose and on its way to this player, or -1
  int8_t kicker;      // who touched it last
  uint32_t heldMs;    // how long its owner has had it, or how long it has been loose
};
struct Match {
  Player pl[PLAYERS];
  Ball ball;
  uint8_t score[2];   // by Team
  int8_t control;     // the teal outfield player the kid steers
  int8_t scored;      // the team that just scored (the match waits for kickoff()), or -1
  uint8_t level;      // difficulty, 0..LEVELS-1 (tune.h)
  bool over;          // the clock ran out
  bool kickoffOn;     // a kickoff still on: the ball not yet off the spot (nor KICKOFF_WAIT_MS gone by)
  uint32_t ms;        // time played (the clock)
  uint32_t rng;       // the coral players' coin flips and aim errors, and each shot's keeper reaction
  uint16_t reactMs;   // how much slower than his best the keeper is to react to the ball in the air (a draw per kick)
};
// What a kick along a direction would do: shoot at a spot in the goal, pass to a teammate, or roll the ball ahead.
struct Kick {
  enum Kind : uint8_t { ROLL, PASS, SHOOT } kind;
  int8_t to;    // PASS: the teammate
  Vec at;       // where the ball is sent (SHOOT: on the goal line, inside the posts)
};

Match start(uint8_t level, uint32_t seed);   // 0-0, teal to kick off
void kickoff(Match& m, int team);            // everyone back in place, `team` on the ball; clears `scored`
Vec heading(Vec tilt);                       // the tilt's direction (a unit vector) past the dead zone, else (0, 0)
Vec aimDir(const Match& m, Vec tilt);        // the kid's aim: the tilt past the dead zone, else where he faces
Kick aim(const Match& m, int who, Vec dir);  // a kick from `who` along the unit vector `dir`
Kick kidKick(const Match& m, Vec tilt);      // what a tap by the kid's player would do now (with the ball)
void step(Match& m, Vec tilt, bool tap);     // one STEP_MS; tilt from tilt::from (milli-g), tap: pressed this step
inline bool timeUp(const Match& m) { return m.ms >= MATCH_MS; }
inline bool finished(const Match& m) {       // after a goal (or at the whistle): no kickoff, the Full time page
  return m.over || m.score[TEAL] >= GOALS_TO_WIN || m.score[CORAL] >= GOALS_TO_WIN || timeUp(m);
}
}  // namespace fc
