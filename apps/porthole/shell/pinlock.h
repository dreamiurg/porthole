// Guessing a code: a few wrong tries are free (small fingers slip), then each one more makes the next try wait,
// doubling from 5 s up to a minute. One lock per kid's code and one for the admin code, so guessing a sibling's
// code never slows anyone else's. Strikes belong to the code they were counted against: they clear on the right
// code, they no longer count once that code changes (set, removed, a new profile in the slot), and they are
// forgotten after an hour with no wrong try (nothing punishes being away). Kept in "sys"/"pinlock" so unplugging the
// board does not skip a wait. host/test_shell.cpp exercises it.
#pragma once
#include <stdint.h>
#include <string.h>
#include "profiles.h"

namespace shell {
constexpr int ADMIN_LOCK = MAX_PROFILES, PIN_LOCKS = MAX_PROFILES + 1;   // locks 0..3: profile ids; 4: admin
constexpr uint8_t PIN_FREE_TRIES = 3;
constexpr uint32_t PIN_FIRST_WAIT = 5, PIN_MAX_WAIT = 60, PIN_FORGET_SEC = 3600;

// Seconds the next try waits after `strikes` wrong codes in a row: 0, 0, 0, 5, 10, 20, 40, 60, 60...
inline uint32_t pinWaitFor(uint8_t strikes) {
  if (strikes <= PIN_FREE_TRIES) return 0;
  uint32_t wait = PIN_FIRST_WAIT;
  for (int n = strikes - PIN_FREE_TRIES - 1; n > 0 && wait < PIN_MAX_WAIT; n--) wait *= 2;
  return wait < PIN_MAX_WAIT ? wait : PIN_MAX_WAIT;
}

struct PinLock {
  uint32_t last;      // local wall-clock second of the latest wrong try
  uint8_t strikes, reserved;
  uint16_t code;      // the stored code (pinCode) the strikes were counted against
};
struct PinLocks {   // "sys"/"pinlock": a versioned, independent blob; append only
  uint16_t version = 1, reserved = 0;
  PinLock lock[PIN_LOCKS] = {};
};
static_assert(sizeof(PinLocks) == 44, "sys/pinlock is persisted: never resize a field");

inline PinLocks loadPinLocks(Store& st) {
  PinLocks l;
  return st.load("sys", "pinlock", &l, sizeof l) == sizeof l && l.version == 1 && !l.reserved ? l : PinLocks{};
}
// Reads the write back: a write that did not land must not let a reboot skip a wait unnoticed.
inline bool savePinLocks(Store& st, const PinLocks& l) {
  st.save("sys", "pinlock", &l, sizeof l);
  PinLocks saved;
  return st.load("sys", "pinlock", &saved, sizeof saved) == sizeof saved && !memcmp(&saved, &l, sizeof l);
}

// Strikes for another code, or an hour old, are none. A clock set back starts the wait from now: never longer than
// PIN_MAX_WAIT.
inline void pinSettle(PinLock& l, uint32_t now, uint16_t code) {
  if (l.code != code) l = PinLock{now, 0, 0, code};
  else if (now < l.last) l.last = now;
  else if (now - l.last >= PIN_FORGET_SEC) l.strikes = 0;
}
// Seconds before this lock takes a try at `code` again, 0 = now.
inline uint32_t pinWaitLeft(PinLock l, uint32_t now, uint16_t code) {
  pinSettle(l, now, code);
  const uint32_t wait = pinWaitFor(l.strikes), since = now - l.last;
  return since >= wait ? 0 : wait - since;
}
inline void pinWrong(PinLock& l, uint32_t now, uint16_t code) {
  pinSettle(l, now, code);
  if (l.strikes < 255) l.strikes++;
  l.last = now;
}
}  // namespace shell
