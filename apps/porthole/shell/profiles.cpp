#include "profiles.h"
#include <stdio.h>
#include <string.h>
#include "crc32.h"

namespace shell {
int Profiles::count() const { int n = 0; for (bool u : used) n += u; return n; }
int Profiles::nth(int i) const {
  for (int id = 0; id < MAX_PROFILES; id++) if (used[id] && i-- == 0) return id;
  return -1;
}

const char* key(char prefix, int id) { static char k[4]; snprintf(k, sizeof k, "%c%d", prefix, id); return k; }

static uint32_t recCrc(const Record& r) { return os::crc32(&r, sizeof r - sizeof r.crc); }
void seal(Record& r) { r.magic = REC_MAGIC; r.version = REC_VERSION; r.size = sizeof(Record); r.crc = recCrc(r); }
bool loadRecord(const void* data, size_t n, Record& out) {   // this version's size, or v1's (the new fields stay 0)
  if (n != sizeof(Record) && n != REC_V1_SIZE) return false;
  memset(&out, 0, sizeof out);
  memcpy(&out, data, n - sizeof out.crc);
  uint32_t crc; memcpy(&crc, (const uint8_t*)data + n - sizeof crc, sizeof crc);
  bool ok = out.magic == REC_MAGIC && out.version == (n == REC_V1_SIZE ? 1 : REC_VERSION) && out.size == n &&
            crc == os::crc32(data, n - sizeof crc);
  out.name[sizeof out.name - 1] = 0;   // a CRC proves the bytes, not that the name ends
  if (ok) seal(out);                   // in memory it is a current record
  return ok;
}

void loadAll(Store& st, Profiles& p) {
  memset(&p, 0, sizeof p);
  for (int id = 0; id < MAX_PROFILES; id++) {
    uint8_t blob[BLOB_MAX];
    size_t n = st.load(NS, key('p', id), blob, sizeof blob);
    p.used[id] = n && loadRecord(blob, n, p.rec[id]);
  }
  uint8_t mark;
  if (!st.load(NS, MIGRATED, &mark, sizeof mark)) migrate(st, p);
}
void saveRecord(Store& st, Profiles& p, int id) { seal(p.rec[id]); st.save(NS, key('p', id), &p.rec[id], sizeof(Record)); }

int create(Store& st, Profiles& p, const Record& draft, const char* const* stores, int nStores) {
  for (int id = 0; id < MAX_PROFILES; id++) {
    if (p.used[id]) continue;
    for (int i = 0; i < nStores; i++) st.erase(stores[i], key('s', id));
    Record& r = p.rec[id];
    memset(&r, 0, sizeof r);
    memcpy(r.name, draft.name, sizeof r.name); r.name[sizeof r.name - 1] = 0;
    r.avatar = draft.avatar; r.age = draft.age; r.pin = draft.pin;
    p.used[id] = true;
    saveRecord(st, p, id);
    return id;
  }
  return -1;
}
void removeProfile(Store& st, Profiles& p, int id, const char* const* stores, int nStores) {
  for (int i = 0; i < nStores; i++) st.erase(stores[i], key('s', id));
  st.erase(NS, key('p', id));
  p.used[id] = false;
}
Profile toProfile(const Profiles& p, int id) {
  const Record& r = p.rec[id];
  Profile out = {(uint8_t)id, {0}, r.avatar, r.age, r.muted};
  memcpy(out.name, r.name, sizeof out.name);
  return out;
}

void recharge(Record& r, uint32_t now) {   // lastPlayed ahead of now is a clock set back, not a break
  if (!resting(r, now) && now >= r.lastPlayed && now - r.lastPlayed >= REST_SEC) r.playSec = 0;
}
bool play(Record& r, uint32_t now, uint32_t dt, int nProfiles, bool idle) {
  r.lastPlayed = now;
  if (r.playDay != now / DAY_SEC) { r.playDay = now / DAY_SEC; r.dayPlaySec = 0; }   // a new day, a new budget
  // A clock jump (the device was off, or the clock was set) is not play time, and neither is a screen nobody touches.
  uint32_t add = idle || resting(r, now) ? 0 : dt > 5 ? 5 : dt;
  r.dayPlaySec += add;
  if (r.dayPlaySec >= DAILY_SEC) { r.playSec = 0; return true; }
  if (nProfiles < 2) return false;
  r.playSec += add;
  if (r.playSec < SESSION_SEC) return false;
  r.restUntil = now + REST_SEC; r.playSec = 0;
  return true;
}
}  // namespace shell
