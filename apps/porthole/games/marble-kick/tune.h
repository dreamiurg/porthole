// Marble Kick's feel: how tilt becomes rolling. Every number a playtest on the device might retune lives here, in the
// same spirit as Pets Club's pet.h knobs. Distances are panel px (480 across), times seconds unless named _MS.
#pragma once

namespace marble {
// Tilt, in milli-g of in-plane gravity away from the kid's own neutral (the Calibrate page).
constexpr int DEAD_MG = 87;    // ~5 degrees: a resting hand's wobble moves nothing
constexpr int FULL_MG = 423;   // ~25 degrees: full acceleration, and no more past it
constexpr float ACCEL_FULL = 700;   // px/s^2 at full tilt: the ball crosses the pitch in about a second
constexpr float DAMPING = 1.6f;     // 1/s: the felt's drag, so the ball settles instead of skating
constexpr float ROLL_DECEL = 40;    // px/s^2: rolling friction, so a ball in the dead zone comes to a stop
constexpr float V_MAX = 600;        // px/s: 3 px per substep, far under the smallest peg (see physics.h)
constexpr float RIM_BOUNCE = 0.4f;  // restitution off the tray's wall
constexpr float PEG_BOUNCE = 0.5f;  // off a peg, a goalpost or the back knob
constexpr int STEP_MS = 5;          // fixed physics substep: frame rate never changes the rolling
constexpr int MAX_FRAME_MS = 100;   // a stalled frame catches up at most this much (no burst of substeps)
constexpr int GOAL_MS = 2000;       // the Goal page moves on by itself after this
// Holes: a ball slower than this drops in (faster, it skims over); it sinks for SINK_MS and is back at the level's start,
// still, at RESPAWN_MS. Nothing else changes: stars stay picked up, the clock runs on.
constexpr float HOLE_SKIM = 380;    // px/s
constexpr int SINK_MS = 500, RESPAWN_MS = 1000;
}  // namespace marble
