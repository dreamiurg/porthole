// Times Cards: the times tables as flash cards, one deck per kid. A card shows "7 x 8"; the kid says the answer and
// taps the card; four answers show, and the kid picks one: green if right, and if not, the right one shows a moment.
// The deck grades the pick from right or wrong and how long it took (deck.h), and decides when each card comes back,
// what the next card is and when the next table opens. A round is ten cards; Again deals the next. No sound, no
// timer on screen in a round (only in the fast round a kid picks for 6x, 8x and 9x), nothing lost: a missed card only
// comes back sooner. All on the RGB565 surface.
#pragma once
#include <stdint.h>
#include "app.h"
#include "canvas.h"
#include "deck.h"
#include "ui.h"

namespace cards {
class Game : public App {
 public:
  const char* name() const override { return "Times Cards"; }
  const gfx::Sprite& icon() const override;
  const char* store() const override { return STORE; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }   // the buzzer is for rare moments; a card is not one
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool wantsHome() override { bool h = home_; home_ = false; return h; }
  void leave() override {}
  const char* screenName() const override;
  void debugPrint() override;
  // "level<N>": a fresh deck on table N (1..10); "card<N>": one card's round; "right" / "wrong": pick that answer;
  // "slow": the known facts of 6x, 8x and 9x answered slowly last half (a fast round on offer)
  void debugCmd(const char* cmd) override;
  const Save& save() const { return save_; }

  enum Page : uint8_t { P_HOME, P_ASK, P_ANSWER, P_DONE, P_STATS, P_COUNT };   // P_STATS: the grown-ups' page

 private:
  Save save_{};
  uint8_t packed_[sizeof(Save)] = {};   // what takeSave hands the shell: the deck as stored (version 4)
  Save check_{};                        // that blob read back, before it may replace the stored deck
  Round round_{};
  Page page_ = P_HOME;
  Input in_{};
  ui::FreshGate gate_;
  ui::Hold title_;   // a long hold on the home page's title opens the grown-ups' page
  canvas::Frames<1> frames_;
  uint32_t ms_ = 0, now_ = 0, askedMs_ = 0;   // askedMs_: when the current card showed
  uint32_t* wallet_ = nullptr;   // the kid's stars (AppEnter::stars), shared with every game
  skills::Book* book_ = nullptr;   // the kid's skill ratings (AppEnter::book), or own_ when the shell gives none
  skills::Book own_{};
  NewSpots* news_ = nullptr;   // the kid's new dots (AppEnter::news): Fast! keeps one until tapped
  skills::Round pay_{};   // this round's points toward stars (os/skills.h)
  int paid_ = 0;          // the stars the last finished round paid, bonuses included
  bool home_ = false, dirty_ = false;
  bool keep_ = false;   // a deck is stored that this build cannot read: it is left as it is until the kid answers a card
  // The answer page: its card (the round moves on as soon as the kid picks), its "k of 10", the four answers, the right
  // one's place, the kid's pick (-1 none yet) and when the page gives way to the next card.
  int face_ = 0, faceNumber_ = 0, options_[OPTIONS] = {}, right_ = 0, picked_ = -1;
  bool faceRedo_ = false;
  uint32_t pickedMs_ = 0;
  int32_t half() const { return halfOf(now_); }
  void go(Page p);
  void deal(bool fast);   // a round, or a fast round (deck.h, buildFast)
  const ui::Box& goBox() const;   // Go!'s place: beside Fast! when the deck offers a fast round
  void turn();
  void pick(int i);   // the kid's answer: options_[i]
  void tell(int card, Grade g, bool retry, uint32_t ms) const;   // telemetry
  void updateHome();     // the deck: Go!, its card, or Fast!
  void updateAnswer();   // the answer page: the pick, then the next card
  void draw() const;
  void drawAnswer() const;
  void drawDone() const;
  void drawOnes() const;
  void fastTime(char* t, size_t n) const;   // the done page's time against the best
  void slowDown();   // debugCmd("slow")   // a missed 1 x n or n x 1, as groups of dots
  uint32_t elapsed() const;   // a fast round's running time
  void heading(int number, bool redo) const;   // "k of n", "One more try", and a fast round's time
  bool heldTitle();
};
}  // namespace cards
