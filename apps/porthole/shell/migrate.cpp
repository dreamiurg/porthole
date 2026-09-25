// First boot of Porthole on a Pets Club device: every Pets Club house becomes a profile.
// The only shell file that knows a game's save layout. It runs on every boot until porthole/m exists, and every
// write is idempotent, so power lost anywhere in it just means the next boot does it again.
#include <string.h>
#include "pet.h"
#include "profiles.h"

namespace shell {
static const char* const PETS_NS = "crago";   // Pets Club's namespace, keys s0..s2 (and "save" before houses)

// House crago/s<slot> becomes profile <slot>: the id is the slot, so the save never moves and a gap stays a gap.
static void adoptHouse(Store& st, Profiles& p, int slot, const Save& h) {
  Record& r = p.rec[slot];
  memset(&r, 0, sizeof r);
  memcpy(r.name, h.kidName, sizeof r.name); r.name[sizeof r.name - 1] = 0;
  // Pets Club stored 0000 as 1, so a legacy 1 becomes code 0001: that one kid types 0001 instead of 0000.
  r.avatar = (uint8_t)slot; r.age = h.kidAge; r.muted = h.muted; r.pin = h.pin ? (uint16_t)(h.pin + 1) : 0;
  r.restUntil = h.restUntil; r.playSec = h.playSec; r.lastPlayed = h.lastSeen;
  p.used[slot] = true;
  saveRecord(st, p, slot);
}

void migrate(Store& st, Profiles& p) {
  uint8_t blob[BLOB_MAX]; Save h; int n = 0;
  for (int slot = 0; slot < MAX_HOUSES; slot++) {
    size_t got = st.load(PETS_NS, key('s', slot), blob, sizeof blob);
    if (got && pet::loadBlob(blob, got, h)) { adoptHouse(st, p, slot, h); n++; }
  }
  size_t got = n ? 0 : st.load(PETS_NS, "save", blob, sizeof blob);   // the pre-house key, only on a device without houses
  if (got && pet::loadBlob(blob, got, h)) {
    st.save(PETS_NS, key('s', 0), blob, got);   // the save first: a crash after it re-migrates from s0
    adoptHouse(st, p, 0, h);
  }
  uint8_t mark = 1;
  st.save(NS, MIGRATED, &mark, sizeof mark);
  st.erase(PETS_NS, "save");   // only after the marker: until then it may be the only copy
}
}  // namespace shell
