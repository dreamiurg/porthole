#pragma once
#include <stdint.h>

namespace pumpkin {
constexpr uint32_t STEP_MS = 10, MAX_FRAME_MS = 100, ROUND_MS = 40000, CATCH_MS = 900;
constexpr float SPOTS[] = {36, 80, 124};
struct Round {
  float x = 80, velocity = 0;
  int hidden = -1;
  uint32_t elapsed = 0, exposure = 0;
  bool caught = false, won = false;
};
int nearby(const Round& round);
bool warning(const Round& round);
bool searching(const Round& round);
float beamX(const Round& round);
void step(Round& round, float roll, bool action);
}  // namespace pumpkin
