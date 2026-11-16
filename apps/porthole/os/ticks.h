// millis() deadlines. Milliseconds in a uint32_t wrap every 49.7 days, and the board stays up for longer.
#pragma once
#include <stdint.h>

namespace os {
// True while `until` is still ahead of `now`, across the wrap too. Only right for a deadline less than 24.8 days
// old: one kept for longer (a member that is reset now and then) also needs settle().
inline bool before(uint32_t now, uint32_t until) { return (int32_t)(until - now) > 0; }
// Once a frame, for a deadline that outlives its use: one that has passed, or that reads further ahead than any is
// ever set (`lead` ms: an old one, seen from the other side of the wrap), becomes `now`. It then never comes round
// again as still ahead, and one stranded just under the wrap is not waited on for 49.7 days.
inline void settle(uint32_t now, uint32_t& until, uint32_t lead) { if (until - now - 1 >= lead) until = now; }
}  // namespace os
