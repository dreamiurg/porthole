// First boot of Porthole on a Pets Club device: every Pets Club house becomes a profile. And the save Biscuit had
// before Porthole moves into a profile.
// The only shell file that knows a game's save layout. Each migration runs on every boot until its marker exists
// (porthole/m, porthole/mb), and every write is idempotent, so power lost anywhere in it just means the next boot
// does it again.
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "games/biscuit/pet.h"
#include "games/pets-club/pet.h"
#include "profiles.h"
#include "ui.h"

namespace shell {
static const char* const PETS_NS = "crago";   // Pets Club's namespace, keys s0..s2 (and "save" before houses)

// House crago/s<slot> becomes profile <slot>: the id is the slot, so the save never moves and a gap stays a gap.
static bool adoptHouse(Store& st, Profiles& p, int slot, const Save& h) {
  Record& r = p.rec[slot];
  memset(&r, 0, sizeof r);
  memcpy(r.name, h.kidName, sizeof r.name); r.name[sizeof r.name - 1] = 0;
  // Pets Club stored 0000 as 1, so a legacy 1 becomes code 0001: that one kid types 0001 instead of 0000.
  r.avatar = (uint8_t)slot; r.age = h.kidAge; r.muted = h.muted; r.pin = h.pin ? (uint16_t)(h.pin + 1) : 0;
  r.restUntil = h.restUntil; r.playSec = h.playSec; r.lastPlayed = h.lastSeen;
  p.used[slot] = true; p.blocked[slot] = false;
  return saveRecord(st, p, slot);
}

void migrate(Store& st, Profiles& p) {
  uint8_t blob[BLOB_MAX]; Save h; int n = 0; bool ok = true;
  for (int slot = 0; slot < MAX_HOUSES; slot++) {
    size_t got = st.load(PETS_NS, key('s', slot), blob, sizeof blob);
    if (!got || !pet::loadBlob(blob, got, h)) continue;
    n++;
    // A record already there is this house from a run cut short, or (the marker never landed) a profile made since:
    // its stars, face and code are not in the pup's save, so it is never rebuilt from it.
    if (!p.used[slot]) ok &= adoptHouse(st, p, slot, h);
  }
  size_t got = n ? 0 : st.load(PETS_NS, "save", blob, sizeof blob);   // the pre-house key, only on a device without houses
  if (got && pet::loadBlob(blob, got, h))   // the save first: a crash after it re-migrates from s0
    ok = st.save(PETS_NS, key('s', 0), blob, got) && (p.used[0] || adoptHouse(st, p, 0, h));
  // A write that did not land leaves no marker: the next boot does it again.
  uint8_t mark = 1;
  if (!ok || !st.save(NS, MIGRATED, &mark, sizeof mark)) return;
  st.erase(PETS_NS, "save");   // only after the marker: until then it may be the only copy
}

// Biscuit before Porthole kept its save in one of two places: its first firmware (namespace "zoegotchi", keys
// pet3/pet2/pet1 by generation) and its first release from this repo (namespace "biscuit", key pet1; today's Biscuit
// store is the same namespace, but its keys are s<id>).
static const char* const OLD_BISCUIT_NS = "zoegotchi";
static const char* const OLD_BISCUIT_KEYS[] = {"pet3", "pet2", "pet1"};
static const char* const NO_NAME = "Friend";   // the profile for a save that never knew the kid's name
static const char* const GAME_STORES[] = {PETS_NS, biscuit::STORE};   // every game store this file knows
using Kid = char[biscuit::NAME_LIMIT + 1];
enum OldBiscuit { OLD_NONE, OLD_FIRST_FIRMWARE, OLD_REPO_RELEASE };

enum OldKey { KEY_MISSING, KEY_UNUSABLE, KEY_CONVERTED };
static OldKey oldBiscuit(Store& st, const char* ns, const char* k, biscuit::Save& s, Kid& kid) {
  uint8_t blob[BLOB_MAX];
  size_t n = st.load(ns, k, blob, sizeof blob);
  return !n ? KEY_MISSING : biscuit::fromLegacy(blob, n, s, kid) ? KEY_CONVERTED : KEY_UNUSABLE;
}
// The newer of the two places' saves that convert. In the first firmware the newest key present decides even when it
// does not convert: its loader never fell back past it, so an older key may hold progress the kid already reset.
static OldBiscuit findOldBiscuit(Store& st, biscuit::Save& best, Kid& kid) {
  OldBiscuit from = OLD_NONE;
  for (const char* k : OLD_BISCUIT_KEYS) {
    OldKey got = oldBiscuit(st, OLD_BISCUIT_NS, k, best, kid);
    if (got == KEY_CONVERTED) from = OLD_FIRST_FIRMWARE;
    if (got != KEY_MISSING) break;
  }
  biscuit::Save s{}; Kid name = "";
  if (oldBiscuit(st, biscuit::STORE, "pet1", s, name) == KEY_CONVERTED && (!from || s.updatedAt > best.updatedAt)) {
    best = s; memcpy(kid, name, sizeof kid); from = OLD_REPO_RELEASE;
  }
  return from;
}
// Whose save it becomes: the profile named like the kid in it (case aside), else the first profile, else a new one.
static int biscuitOwner(Store& st, Profiles& p, const Kid& kid) {
  char want[ui::NAME_LEN + 1];   // as long as a name the shell lets a kid type: longer ones never match, or fit
  memcpy(want, kid, ui::NAME_LEN); want[ui::NAME_LEN] = 0;
  for (int id = 0; id < MAX_PROFILES; id++) if (p.used[id] && *want && !strcasecmp(p.rec[id].name, want)) return id;
  if (p.count()) return p.nth(0);
  Record draft = {};
  snprintf(draft.name, sizeof draft.name, "%s", *want ? want : NO_NAME);
  const int id = create(st, p, draft, GAME_STORES, 2);   // age 0: the shell asks once, like a migrated v1 house
  // create opens a profile whose record did not land (the shell writes it again as the kid plays). Nothing would here:
  // the pup would move to a profile the next boot does not have, so without the record there is no owner yet.
  Record back; uint8_t blob[sizeof back];
  const size_t n = id < 0 ? 0 : st.load(NS, key('p', id), blob, sizeof blob);
  if (n && loadRecord(blob, n, back)) return id;
  if (id >= 0) p.used[id] = false;
  return -1;
}

void migrateBiscuit(Store& st, Profiles& p) {
  uint8_t mark = 1;
  if (st.load(NS, MIGRATED_BISCUIT, &mark, sizeof mark)) return;
  biscuit::Save s{}; Kid kid = "";
  OldBiscuit from = findOldBiscuit(st, s, kid);
  if (from) {   // before the marker; with no profile to take it, or a copy that did not land, the old save stays for the next boot
    const int owner = biscuitOwner(st, p, kid);
    if (owner < 0) return;
    // A pup already at the owner's key is never replaced: it is this copy from a boot cut short (the same bytes), or
    // one played since a marker that did not land, and then the old place is left as it is, unread.
    biscuit::Save there{}; uint8_t blob[sizeof there];
    const size_t n = st.load(biscuit::STORE, key('s', owner), blob, sizeof blob);
    if (n && biscuit::loadBlob(blob, n, there)) { if (memcmp(&there, &s, sizeof s)) from = OLD_NONE; }
    else if (!st.save(biscuit::STORE, key('s', owner), &s, sizeof s)) return;
  }
  st.save(NS, MIGRATED_BISCUIT, &mark, sizeof mark);
  // Only after the marker, and only the place the pup came from: anything else stays put, unread, never lost.
  if (from == OLD_FIRST_FIRMWARE) for (const char* k : OLD_BISCUIT_KEYS) st.erase(OLD_BISCUIT_NS, k);
  if (from == OLD_REPO_RELEASE) st.erase(biscuit::STORE, "pet1");
}
}  // namespace shell
