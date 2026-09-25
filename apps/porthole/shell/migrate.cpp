// First boot of Porthole on a Pets Club device: every Pets Club house becomes a profile.
// The only shell file that knows a game's save layout; it runs once, when no porthole/p* record exists.
#include <string.h>
#include "pet.h"
#include "profiles.h"

namespace shell {
static const char* const PETS_NS = "crago";   // Pets Club's namespace, keys s0..s2 (and "save" before houses)

bool migrate(Store& st, Profiles& p) {
  Save houses[MAX_HOUSES]; char from[MAX_HOUSES][8]; int n = 0; uint8_t blob[BLOB_MAX];
  for (int slot = 0; slot < MAX_HOUSES; slot++) {   // loaded exactly as Pets Club's firmware did: s0..s2, gaps closed
    size_t got = st.load(PETS_NS, key('s', slot), blob, sizeof blob);
    if (got && pet::loadBlob(blob, got, houses[n])) strcpy(from[n++], key('s', slot));
  }
  if (!n) {
    size_t got = st.load(PETS_NS, "save", blob, sizeof blob);
    if (got && pet::loadBlob(blob, got, houses[0])) strcpy(from[n++], "save");
  }
  for (int i = 0; i < n; i++) {
    const Save& h = houses[i]; Record& r = p.rec[i];
    memset(&r, 0, sizeof r);
    memcpy(r.name, h.kidName, sizeof r.name); r.name[sizeof r.name - 1] = 0;
    r.avatar = (uint8_t)i; r.age = h.kidAge; r.muted = h.muted; r.pin = h.pin;
    r.restUntil = h.restUntil; r.playSec = h.playSec; r.lastPlayed = h.lastSeen;
    p.used[i] = true;
    saveRecord(st, p, i);
    if (strcmp(from[i], key('s', i))) {   // house i must live in s<i>, the key of profile i
      st.save(PETS_NS, key('s', i), &h, sizeof h);
      st.erase(PETS_NS, from[i]);
    }
  }
  return n > 0;
}
}  // namespace shell
