#include "profiles.h"
#include <stdio.h>
#include <string.h>
#include "crc32.h"

namespace shell {
static bool validLimits(const Limits& l) {
  return l.sessionMinutes >= 1 && l.sessionMinutes <= 1440 && l.dailyMinutes >= 1 && l.dailyMinutes <= 1440;
}
Limits loadLimits(Store& st) {
  Limits l{};
  return st.load("sys", "limits", &l, sizeof l) == sizeof l && validLimits(l) ? l : Limits{};
}
bool saveLimits(Store& st, const Limits& l) {
  return validLimits(l) && st.save("sys", "limits", &l, sizeof l);
}
static bool validWindow(const PlayWindow& w) {
  return w.version == 1 && w.enabled <= 1 && !w.reserved && w.startMinute < 1440 && w.endMinute < 1440 && w.startMinute != w.endMinute;
}
PlayWindow loadPlayWindow(Store& st) {
  PlayWindow w{};
  return st.load("sys", "window", &w, sizeof w) == sizeof w && validWindow(w) ? w : PlayWindow{};
}
bool savePlayWindow(Store& st, const PlayWindow& w) {
  if (!validWindow(w)) return false;
  st.save("sys", "window", &w, sizeof w);
  PlayWindow saved{};
  return st.load("sys", "window", &saved, sizeof saved) == sizeof saved && !memcmp(&saved, &w, sizeof w);
}
bool windowAllows(const PlayWindow& w, uint32_t now) {
  if (!w.enabled) return true;
  const uint32_t minute = now % DAY_SEC / 60;
  return w.startMinute < w.endMinute ? minute >= w.startMinute && minute < w.endMinute : minute >= w.startMinute || minute < w.endMinute;
}
uint32_t windowWait(const PlayWindow& w, uint32_t now) {
  if (windowAllows(w, now)) return 0;
  return (w.startMinute * 60u + DAY_SEC - now % DAY_SEC) % DAY_SEC;
}
int Profiles::count() const { int n = 0; for (bool u : used) n += u; return n; }
int Profiles::firstFree() const {
  for (int id = 0; id < MAX_PROFILES; id++) if (!used[id] && !blocked[id]) return id;
  return -1;
}
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
    p.blocked[id] = n && !p.used[id];
  }
  uint8_t mark;
  if (!st.load(NS, MIGRATED, &mark, sizeof mark)) migrate(st, p);
  migrateBiscuit(st, p);   // after migrate: it matches the migrated names; a no-op once porthole/mb exists
}
bool saveRecord(Store& st, Profiles& p, int id) { seal(p.rec[id]); return st.save(NS, key('p', id), &p.rec[id], sizeof(Record)); }

int create(Store& st, Profiles& p, const Record& draft, const char* const* stores, int nStores) {
  const int id = p.firstFree();
  if (id < 0) return -1;
  for (int i = 0; i < nStores; i++) st.erase(stores[i], key('s', id));
  st.erase(NS, key('f', id));
  Record& r = p.rec[id];
  memset(&r, 0, sizeof r);
  memcpy(r.name, draft.name, sizeof r.name); r.name[sizeof r.name - 1] = 0;
  r.avatar = draft.avatar; r.age = draft.age; r.pin = draft.pin;
  p.used[id] = true;
  saveRecord(st, p, id);
  return id;
}
void removeProfile(Store& st, Profiles& p, int id, const char* const* stores, int nStores) {
  for (int i = 0; i < nStores; i++) st.erase(stores[i], key('s', id));
  st.erase(NS, key('f', id));
  st.erase(NS, key('p', id));
  p.used[id] = false;
}
static uint32_t favCrc(const Favorites& f) { return os::crc32(&f, sizeof f - sizeof f.crc); }
bool loadFavorites(Store& st, int profileId, Favorites& out) {
  const bool ok = st.load(NS, key('f', profileId), &out, sizeof out) == sizeof out && out.magic == FAV_MAGIC &&
                  out.version == FAV_VERSION && out.size == sizeof out && out.count <= MAX_FAVS && out.crc == favCrc(out);
  if (!ok) memset(&out, 0, sizeof out);
  return ok;
}
bool saveFavorites(Store& st, int profileId, Favorites& f) {
  f.magic = FAV_MAGIC; f.version = FAV_VERSION; f.size = sizeof f; f.crc = favCrc(f);
  Favorites back;
  return st.save(NS, key('f', profileId), &f, sizeof f) && loadFavorites(st, profileId, back) && !memcmp(&back, &f, sizeof f);
}
static uint32_t gamesCrc(const GamesOff& g) { return os::crc32(&g, sizeof g - sizeof g.crc); }
uint32_t loadGamesOff(Store& st) {
  GamesOff g{};
  const bool ok = st.load("sys", "games", &g, sizeof g) == sizeof g && g.version == 1 && g.size == sizeof g && g.crc == gamesCrc(g);
  return ok ? g.off : 0;
}
bool saveGamesOff(Store& st, uint32_t off) {
  GamesOff g{1, sizeof(GamesOff), off, 0}; g.crc = gamesCrc(g);
  return st.save("sys", "games", &g, sizeof g) && loadGamesOff(st) == off;
}
int enabledGames(const uint8_t* ids, int n, uint32_t off, int* out) {
  const bool none = allOff(ids, n, off);   // never none: an all-off mask is no mask
  int m = 0;
  for (int k = 0; k < n; k++) if (none || !(off & gameBit(ids[k]))) out[m++] = k;
  return m;
}
static int indexOf(uint8_t id, const uint8_t* ids, int n) {
  for (int k = 0; k < n; k++) if (ids[k] == id) return k;
  return -1;
}
static bool listed(int k, const int* list, int m) {
  for (int j = 0; j < m; j++) if (list[j] == k) return true;
  return false;
}
int ownGames(const Favorites& f, const uint8_t* ids, int n, uint32_t off, int* out) {
  int on[MAX_GAME_ID]; const int m = enabledGames(ids, n, off, on);
  if (m <= MAX_FAVS + 1) { memcpy(out, on, m * sizeof *out); return m; }
  if (!f.picked) { memcpy(out, on, MAX_FAVS * sizeof *out); return MAX_FAVS; }
  int h = 0;
  for (int i = 0; i < f.count && i < MAX_FAVS; i++) {
    const int k = indexOf(f.ids[i], ids, n);
    if (k >= 0 && listed(k, on, m)) out[h++] = k;
  }
  return h;
}
int homeGames(const Favorites& f, const uint8_t* ids, int n, uint32_t off, int* out) {
  const int h = ownGames(f, ids, n, off, out);
  if (h) return h;
  int on[MAX_GAME_ID]; const int m = enabledGames(ids, n, off, on), d = m < MAX_FAVS ? m : MAX_FAVS;   // no games: none
  memcpy(out, on, d * sizeof *out); return d;   // every star hidden or taken off: the default (the list stays)
}
bool toggleStar(Favorites& f, uint8_t id, const uint8_t* ids, int n, uint32_t off) {
  int home[MAX_GAME_ID]; const int h = ownGames(f, ids, n, off, home);   // even with none showing
  Favorites g = f;   // f changes only on success: a refused star leaves even the default untouched
  if (!g.picked) {   // the default becomes the kid's own list
    g.count = (uint8_t)(h < MAX_FAVS ? h : MAX_FAVS); g.picked = 1;
    for (int i = 0; i < g.count; i++) g.ids[i] = ids[home[i]];
  }
  int i = 0;
  while (i < g.count && g.ids[i] != id) i++;
  if (i < g.count) { memmove(g.ids + i, g.ids + i + 1, g.count - i - 1); g.count--; f = g; return true; }
  if (h >= MAX_FAVS) return false;
  if (g.count == MAX_FAVS) {   // a hidden favorite (switched off, or gone) gives up its place: the oldest, the last
    i = MAX_FAVS - 1;
    while (i > 0 && listed(indexOf(g.ids[i], ids, n), home, h)) i--;
    memmove(g.ids + i, g.ids + i + 1, g.count - i - 1); g.count--;
  }
  memmove(g.ids + 1, g.ids, g.count); g.ids[0] = id; g.count++;   // what was just picked shows first
  f = g; return true;
}
Profile toProfile(const Profiles& p, int id) {
  const Record& r = p.rec[id];
  Profile out = {(uint8_t)id, {0}, r.avatar, r.age, r.muted};
  memcpy(out.name, r.name, sizeof out.name);
  return out;
}

RestHint restHint(const Record& r, const PlayWindow& window, uint32_t now, const Limits& limits) {
  if (playedToday(r, now, limits)) return REST_DAY;
  return windowAllows(window, now + restLeft(r, now, limits)) ? REST_BREAK : REST_WINDOW;
}
void liftCaps(Store& st, Profiles& p) {
  for (int id = 0; id < MAX_PROFILES; id++)
    if (p.used[id] && p.rec[id].dayPlaySec) { p.rec[id].dayPlaySec = 0; saveRecord(st, p, id); }
}
bool resetLimits(Store& st, Profiles& p) {
  bool ok = true;
  for (int id = 0; id < MAX_PROFILES; id++) {
    Record& r = p.rec[id];
    if (!p.used[id] || !(r.restUntil || r.playSec || r.dayPlaySec)) continue;
    const Record before = r;
    r.restUntil = r.playSec = r.dayPlaySec = 0;
    if (!saveRecord(st, p, id)) { r = before; ok = false; }
  }
  return ok;
}
void recharge(Record& r, uint32_t now) {   // lastPlayed ahead of now is a clock set back, not a break
  if (!resting(r, now) && now >= r.lastPlayed && now - r.lastPlayed >= REST_SEC) r.playSec = 0;
}
void playUnrestricted(Record& r, uint32_t now, uint32_t dt, bool idle) {
  r.lastPlayed = now;
  if (r.playDay != now / DAY_SEC) { r.playDay = now / DAY_SEC; r.dayPlaySec = 0; }
  const uint32_t add = idle ? 0 : dt > 5 ? 5 : dt;
  r.dayPlaySec += add; r.playSec += add;
}
bool enforceBudget(Record& r, uint32_t now, const Limits& limits) {
  if (playedToday(r, now, limits)) return true;
  if (resting(r, now)) return true;
  if (r.playSec < (uint32_t)limits.sessionMinutes * 60) return false;
  r.restUntil = now + REST_SEC; r.playSec = 0;
  return true;
}
bool play(Record& r, uint32_t now, uint32_t dt, bool idle, const Limits& limits) {
  // Clock jumps are not play time. Only the monotonic dt supplied by the shell is counted.
  const bool wasResting = resting(r, now);
  playUnrestricted(r, now, dt, idle || wasResting);
  if (playedToday(r, now, limits)) { r.playSec = 0; return true; }
  return !wasResting && enforceBudget(r, now, limits);
}
}  // namespace shell
