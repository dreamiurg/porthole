#include "rules.h"
#include <math.h>

namespace pumpkin {
namespace {
constexpr uint32_t CYCLE_MS = 12000, WARNING_MS = 3000, SEARCH_MS = 5000, REST_MS = 10000;
constexpr float DEAD_MG = 60, MAX_MG = 500, ACCEL = 150, FRICTION = 0.94f, SPEED = 38;
float clamp(float n, float low, float high) { return n < low ? low : n > high ? high : n; }
void rollPumpkin(Round& r, float roll) {
  if (r.hidden >= 0) return;
  float force = 0;
  if (fabsf(roll) > DEAD_MG) force = clamp(roll, -MAX_MG, MAX_MG) / MAX_MG;
  r.velocity = clamp((r.velocity + force * ACCEL * STEP_MS / 1000) * FRICTION, -SPEED, SPEED);
  if (fabsf(r.velocity) < 0.02f) r.velocity = 0;
  r.x += r.velocity * STEP_MS / 1000;
  if (r.x < 24 || r.x > 136) { r.x = clamp(r.x, 24, 136); r.velocity = 0; }
}
void toggleHide(Round& r) {
  if (r.hidden >= 0) { r.hidden = -1; return; }
  r.hidden = nearby(r);
  if (r.hidden < 0) return;
  r.x = SPOTS[r.hidden];
  r.velocity = 0;
  r.exposure = 0;
}
}  // namespace
int nearby(const Round& r) {
  for (int i = 0; i < 3; i++) if (fabsf(r.x - SPOTS[i]) <= 13) return i;
  return -1;
}
bool warning(const Round& r) {
  const uint32_t phase = r.elapsed % CYCLE_MS;
  return phase >= WARNING_MS && phase < SEARCH_MS;
}
bool searching(const Round& r) {
  const uint32_t phase = r.elapsed % CYCLE_MS;
  return phase >= SEARCH_MS && phase < REST_MS;
}
float beamX(const Round& r) {
  const float phase = clamp(((float)(r.elapsed % CYCLE_MS) - SEARCH_MS) / (REST_MS - SEARCH_MS), 0, 1);
  return r.elapsed / CYCLE_MS % 2 ? 136 - phase * 112 : 24 + phase * 112;
}
void step(Round& r, float roll, bool action) {
  if (r.caught || r.won) return;
  if (action) toggleHide(r);
  rollPumpkin(r, roll);
  r.elapsed += STEP_MS;
  const bool seen = r.hidden < 0 && searching(r) && fabsf(r.x - beamX(r)) <= 20;
  r.exposure = seen ? r.exposure + STEP_MS : 0;
  r.caught = r.exposure >= CATCH_MS;
  r.won = !r.caught && r.elapsed >= ROUND_MS;
}
}  // namespace pumpkin
