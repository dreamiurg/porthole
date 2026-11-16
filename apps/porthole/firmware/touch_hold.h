// One failed touch read is not a finger lift. An I2C read that fails in the middle of a press used to come back as
// "not touching": release then press, so a phantom tap and a drag cut in two. The last good sample stands in for one
// failed read; a second failure in a row is a release, so a dead bus can never hold a finger down.
#pragma once
#include <stdint.h>
namespace board {
struct Touch { bool down; int x, y; };  // physical pixels
class TouchHold {
 public:
  Touch step(bool readOk, Touch now) {
    if (readOk) { last_ = now; missed_ = false; return now; }
    misses_++;
    if (missed_) last_ = {false, 0, 0};
    missed_ = true;
    return last_;
  }
  uint32_t misses() const { return misses_; }   // failed reads since boot, for the serial "M" line
 private:
  Touch last_ = {false, 0, 0};
  uint32_t misses_ = 0;
  bool missed_ = false;
};
}  // namespace board
