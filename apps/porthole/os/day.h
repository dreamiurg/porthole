// One clock policy for every daily rule. Time is the shell's local wall-clock seconds, and that clock can jump: set by
// a grown-up, restored after a flat battery, corrected over wifi. No rule may leave a kid waiting on a date the clock
// once ran ahead to, so a rule that asks "is it a new day?" or "how long since?" asks here.
#pragma once
#include <stdint.h>

namespace os {
constexpr uint32_t DAY_SEC = 86400;
inline uint32_t dayOf(uint32_t localSec) { return localSec / DAY_SEC; }   // it ends at local midnight
// A new day is any day that differs from the stored one, an earlier one too: after the clock is put right the day
// starts over at once. The cost, accepted: a clock that steps back over midnight and forward again starts two.
inline bool newDay(uint32_t today, uint32_t stored) { return today != stored; }
// Seconds from `then` to `now`; none across a clock set back (the caller then takes `now` as its new mark).
inline uint32_t elapsed(uint32_t now, uint32_t then) { return now > then ? now - then : 0; }
}  // namespace os
