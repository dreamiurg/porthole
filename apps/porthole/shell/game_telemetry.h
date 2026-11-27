// What every game reports without a line of its own (docs/telemetry-events.md): the shell runs one of these around
// the open game. Active time leaves out idle stretches (shell::IDLE_MS without a touch or a move). A game's screen is
// its screenName(), so the time on each screen, where kids leave and the path through a game come for free.
#pragma once
#include <stdint.h>
#include <string.h>
#include "telemetry.h"

namespace shell {
class GameTelemetry {
 public:
  static constexpr uint32_t BEAT_MS = 60000;      // game_heartbeat: a battery pulled mid-game still leaves its minutes
  static constexpr uint32_t SCREEN_MIN_MS = 1000; // a screen shown for less (a flash between two) is not reported

  void open(const char* game, uint32_t ms, const char* where) {
    game_ = game; openMs_ = lastMs_ = beatMs_ = screenMs_ = ms; activeMs_ = 0; copy(where);
    telemetry::emit("game_opened", {telemetry::P("game", game)});
  }
  void tick(uint32_t ms, bool idle, const char* where) {   // every frame the game is open, after its update
    if (!idle) activeMs_ += ms - lastMs_;
    lastMs_ = ms;
    if (strncmp(where, screen_, sizeof screen_ - 1)) { left(ms); copy(where); screenMs_ = ms; }
    if (ms - beatMs_ < BEAT_MS) return;
    beatMs_ = ms;
    telemetry::emit("game_heartbeat", {telemetry::P("game", game_), telemetry::P("active_s", activeS()), telemetry::P("where", screen_)});
  }
  void close(uint32_t ms, long stars, const char* why) {   // why: "home", "rest", "time_up"
    left(ms);
    telemetry::emit("game_closed", {telemetry::P("game", game_), telemetry::P("seconds", (long)((ms - openMs_) / 1000)),
                                    telemetry::P("stars", stars), telemetry::P("exit", why), telemetry::P("active_s", activeS()),
                                    telemetry::P("where", screen_)});
  }

 private:
  long activeS() const { return (long)(activeMs_ / 1000); }
  void copy(const char* where) { strncpy(screen_, where ? where : "", sizeof screen_ - 1); screen_[sizeof screen_ - 1] = 0; }
  void left(uint32_t ms) {   // the screen just left: which, and how long it was shown
    if (ms - screenMs_ < SCREEN_MIN_MS) return;
    telemetry::emit("screen", {telemetry::P("game", game_), telemetry::P("screen", screen_), telemetry::P("secs", (long)((ms - screenMs_) / 1000))});
  }
  const char* game_ = "";
  uint32_t openMs_ = 0, lastMs_ = 0, beatMs_ = 0, screenMs_ = 0, activeMs_ = 0;
  char screen_[telemetry::TEXT] = {0};
};
}  // namespace shell
