#include "news.h"
#include <string.h>
#include "news_entries.h"

namespace shell {
bool newsCurrent(const NewsEntry& e, uint32_t nowSec) {
  return !e.gone && (nowSec < NEWS_CLOCK_SET || e.day + NEWS_DAYS >= nowSec / 86400);
}
static bool before(const NewsEntry* e, int a, int b) {   // a goes on the card above b
  return e[a].size != e[b].size ? e[a].size > e[b].size : a > b;
}
int newsLines(const NewsEntry* e, int n, const NewsSince& since, int* out, int max) {
  constexpr int GROUPS = 16;   // more than the launcher's games, plus the launcher's own news
  int best[GROUPS], games = 0;   // each game's line: its biggest entry, the newest of equals
  for (int i = since.after; i < n; i++) {
    if (!since.ok[i] || !newsCurrent(e[i], since.nowSec)) continue;
    int g = 0;
    while (g < games && strcmp(e[best[g]].store, e[i].store)) g++;
    if (g == games) { if (games < GROUPS) best[games++] = i; }
    else if (e[i].size >= e[best[g]].size) best[g] = i;
  }
  for (int a = 1; a < games; a++)
    for (int b = a; b > 0 && before(e, best[b], best[b - 1]); b--) { const int t = best[b]; best[b] = best[b - 1]; best[b - 1] = t; }
  const int m = games < max ? games : max;
  for (int k = 0; k < m; k++) out[k] = best[k];
  return m;
}

static uint16_t newest() { return (uint16_t)NEWS_COUNT; }
void KidNews::save() { saveRecord(*st_, *p_, id_); }
void KidNews::fresh() {
  Record& r = p_->rec[id_];
  r.newsSeen = newest(); r.newsFrom = (uint16_t)(newest() + 1); r.newsTiles = r.newsSpots = 0; save();
}
void KidNews::forget() { Record& r = p_->rec[id_]; r.newsSeen = r.newsFrom = r.newsTiles = r.newsSpots = 0; save(); }
void KidNews::showed() { if (rec().newsSeen != newest()) { p_->rec[id_].newsSeen = newest(); save(); } }
bool KidNews::done(int number, bool spot) const {
  const Record& r = rec();
  if (number < r.newsFrom) return true;
  return number - r.newsFrom < 16 && ((spot ? r.newsSpots : r.newsTiles) >> (number - r.newsFrom) & 1);
}
// shortcut: 16 entries a window; a dot not cleared falls out (done) once 16 newer ones are marked. Fine at a few
// entries a week; widen in a new record version if news.toml ever gets busier.
void KidNews::mark(int number, bool spot) {
  Record& r = p_->rec[id_];
  if (number >= r.newsFrom + 16) {
    const int by = number - 15 - r.newsFrom;
    r.newsTiles = by >= 16 ? 0 : (uint16_t)(r.newsTiles >> by); r.newsSpots = by >= 16 ? 0 : (uint16_t)(r.newsSpots >> by);
    r.newsFrom = (uint16_t)(number - 15);
  }
  (spot ? r.newsSpots : r.newsTiles) |= (uint16_t)(1u << (number - r.newsFrom));
}
void KidNews::open(const char* store, uint32_t nowSec) {
  store_ = store; now_ = nowSec;
  bool changed = false;
  for (int i = 0; i < NEWS_COUNT; i++)
    if (!strcmp(NEWS[i].store, store) && newsCurrent(NEWS[i], nowSec) && !done(i + 1, false)) { mark(i + 1, false); changed = true; }
  if (changed) save();   // no flash write for a game with nothing new
}
bool KidNews::tileDot(const char* store, uint32_t nowSec) const {
  for (int i = 0; i < NEWS_COUNT; i++) if (!strcmp(NEWS[i].store, store) && newsCurrent(NEWS[i], nowSec) && !done(i + 1, false)) return true;
  return false;
}
bool KidNews::isNew(const char* spot) const {
  for (int i = 0; i < NEWS_COUNT; i++)
    if (!strcmp(NEWS[i].store, store_) && !strcmp(NEWS[i].spot, spot) && newsCurrent(NEWS[i], now_) && !done(i + 1, true)) return true;
  return false;
}
void KidNews::seen(const char* spot) {
  bool changed = false;
  for (int i = 0; i < NEWS_COUNT; i++)
    if (!strcmp(NEWS[i].store, store_) && !strcmp(NEWS[i].spot, spot) && !done(i + 1, true)) { mark(i + 1, true); changed = true; }
  if (changed) save();
}
}  // namespace shell
