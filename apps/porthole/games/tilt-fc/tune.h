// Tilt FC's feel and difficulty: every number a playtest on the device might retune, in the same spirit as Marble
// Kick's tune.h. Distances are panel px (480 across the glass), speeds px/s, times ms unless named otherwise.
// host/test_tiltfc.cpp holds the balance to account: a greedy player must score clearly less than one who passes, a
// player who does nothing must not be scored on in a hurry. Retune, then run `make test`.
#pragma once
#include <stdint.h>

namespace fc {
// Tilt, milli-g away from the kid's own neutral (tilt::from). Past the dead zone it sets the direction only: players
// run at one fixed speed, so there is no "full tilt".
constexpr int DEAD_MG = 87;   // ~5 degrees: a resting hand's wobble moves nothing

// The match.
constexpr int GOALS_TO_WIN = 3;          // first to three...
constexpr uint32_t MATCH_MS = 180000;    // ...or three minutes of play, whichever comes first
constexpr uint32_t EXTRA_MS = 10000;     // at the whistle, a teal attack still on plays on this long at most
constexpr int STEP_MS = 10;              // fixed rules step: the frame rate never changes the game
constexpr int MAX_FRAME_MS = 100;        // a stalled frame catches up at most this much
constexpr uint32_t KICKOFF_MS = 2400;    // the Kickoff page's 3-2-1: 0.8 s a digit, time for a 6-year-old to read it
constexpr uint32_t KICKOFF_WAIT_MS = 3000;   // at a teal kickoff coral wait outside the circle until the ball moves, this long at most
constexpr float CIRCLE_R = 50;           // the centre circle
constexpr uint32_t GOAL_MS = 2000;       // the Goal page moves on by itself after this
constexpr uint32_t FT_READY_MS = 1500;   // the Full time page shows the result this long before its go sign works

// Your team. The kid's player turns on the spot (direction is the tilt); the others steer with some weight.
constexpr float RUN_SPEED = 140;         // the controlled player, with or without the ball
constexpr float MATE_SPEED = 125;        // teal players the kid is not steering
constexpr float PASS_SPEED = 300, SHOT_SPEED = 400, ROLL_SPEED = 190;
constexpr float SHOOT_RANGE = 125;       // the goal's middle this close and in the cone: a tap shoots
constexpr float CONE_DEG = 30;           // a tap passes to a teammate within this many degrees of the aim
constexpr float SLIDE_SPEED = 250;       // a tap without the ball: a slide this fast...
constexpr uint32_t SLIDE_MS = 300;       // ...this long, and if it wins nothing,
constexpr uint32_t SLOW_MS = 500;        // half a second at SLOW_SPEED to get up again: a missed tackle's only cost
constexpr float SLOW_SPEED = 40;
constexpr float SWITCH_MARGIN = 30;      // control moves to the other player only when he is this much nearer the ball

// Them, by difficulty level (0 = first match; one up per win, one down per loss by two or more).
constexpr int LEVELS = 5;
constexpr float DEF_SPEED[LEVELS] = {93, 98, 103, 109, 114};   // coral outfield players, under RUN_SPEED: outrun them
constexpr float DEF_ACCEL[LEVELS] = {329, 428, 527, 626, 725};   // how fast they turn: a sidestep beats a slow one
constexpr float KEEPER_SPEED[LEVELS] = {97, 102, 107, 112, 116};   // coral's keeper, across the goal
constexpr uint32_t KEEPER_REACT_MS[LEVELS] = {57, 43, 29, 15, 1};    // before he moves for a shot, plus...
constexpr uint32_t REACT_SPREAD_MS = 220;   // ...up to this much more, a new draw every shot: a save is a chance
constexpr float AI_SHOT_SPEED[LEVELS] = {285, 295, 305, 314, 324};        // coral shots: slower than the kid's
constexpr float TEAL_KEEPER_SPEED = 100;  // the kid's keeper does not get better with the level: they do
constexpr uint32_t TEAL_KEEPER_REACT_MS = 40;
constexpr float KEEPER_REACH = 0;       // px a keeper's catch reaches past touching (less: he has to be right there)
constexpr float AI_SHOOT_RANGE = 105;    // coral shoot once this close to the goal's middle...
constexpr uint32_t AI_HOLD_MS = 6000;    // ...or after holding the ball this long: their attacks end
constexpr float AI_SHOT_MISS[LEVELS] = {21, 18, 14, 11, 10};   // px of random aim error on a coral shot

// The ball.
constexpr float BALL_DRAG = 0.9f;        // 1/s
constexpr float BALL_FRICTION = 25;      // px/s^2: a slow ball comes to rest
constexpr float BALL_BOUNCE = 0.55f;     // off the court's walls
constexpr float KNOCK_SPEED = 100;       // a tackled ball squirts away this fast
constexpr uint32_t KICK_COOL_MS = 250;   // the kicker cannot touch his own kick for this long
constexpr uint32_t LOSE_MS = 500;        // nor can a player who was just tackled
constexpr uint32_t PASS_MS = 1500;       // a pass's receiver runs to meet it for at most this long
constexpr uint32_t KEEPER_HOLD_MS = 900; // a keeper holds a caught ball this long, then throws it out
constexpr float KEEPER_ROOM = 80;        // and nobody from the other team comes closer to him meanwhile
}  // namespace fc
