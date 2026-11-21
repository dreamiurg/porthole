// Kid news: after an update, "New for Sam!" with a line per game that got something (one or two screens), and a red
// dot on each new game's tile and on new buttons inside games until the kid opens or taps them. The entries are
// news.toml (shell/news_entries.h, made by tools/news.py); what earns one and how it reads: "Kid news" in AGENTS.md.
// What each kid has seen is in their profile record (v4), so it goes with them.
#pragma once
#include <stdint.h>
#include "app.h"
#include "profiles.h"

namespace shell {
// The card's order: a new game, then a new thing to do, then new things to find. The values are news.toml's sizes.
enum NewsSize : uint8_t { NEWS_NEW, NEWS_BIG, NEWS_GAME };
// store: the game's App::store(), "" for the launcher's own news. spot: a button inside the game, "" for none.
struct NewsEntry { uint16_t day; NewsSize size; bool gone; const char* store; const char* spot; const char* text; };

constexpr uint32_t NEWS_DAYS = 60;   // an entry is news this long after its day, by the device's clock
// A clock before this never got set (no network yet, no RTC): every entry is still news, nothing expires.
constexpr uint32_t NEWS_CLOCK_SET = 1767225600;   // 2026-01-01

// Entry numbers are 1-based places in news.toml; 0 is none. What a kid has seen is in their Record (newsSeen, newsFrom,
// newsTiles, newsSpots: shell/profiles.h), so it goes with the profile.
bool newsCurrent(const NewsEntry& e, uint32_t nowSec);   // not gone and still within NEWS_DAYS
// The lines of the "New for" screen, at most max, as entry indexes: one per game (its biggest entry, the newest of
// equals), in NewsSize order from NEWS_GAME, newest first. Entries past `after` (1-based) whose ok[] is true count.
struct NewsSince { uint16_t after; uint32_t nowSec; const bool* ok; };
int newsLines(const NewsEntry* e, int n, const NewsSince& since, int* out, int max);

// The active kid's news, for the shell and (as NewSpots) for the open game. It reads and saves the kid's Record.
class KidNews final : public NewSpots {
 public:
  void load(Store& st, Profiles& p, int id) { st_ = &st; p_ = &p; id_ = id; }
  void fresh();    // a new kid: everything there is now counts as seen
  void forget();   // sim hook: nothing seen, like a kid from before Kid news
  void open(const char* store, uint32_t nowSec);   // the kid opened this game: its tile dot goes
  void showed();                                   // the "New for" screen was OK'd
  uint16_t seen() const { return rec().newsSeen; }
  bool tileDot(const char* store, uint32_t nowSec) const;
  bool isNew(const char* spot) const override;
  void seen(const char* spot) override;

 private:
  Store* st_ = nullptr;
  Profiles* p_ = nullptr;
  int id_ = -1;
  const char* store_ = "";   // the open game's
  uint32_t now_ = 0;
  const Record& rec() const { return p_->rec[id_]; }
  bool done(int number, bool spot) const;
  void mark(int number, bool spot);   // done; slides the 16-entry window up when it must
  void save();
};
}  // namespace shell
