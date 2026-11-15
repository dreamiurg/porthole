// Times Cards: the times tables as flash cards, one deck per kid. A card shows "7 x 8"; the kid says the answer out
// loud and taps the card; the answer shows, and the kid says how it went: Easy, Hard or Wrong. Spaced repetition
// (deck.h) decides when each card comes back and when the next table opens. A round is ten cards; Again deals the next.
// No sound, no timer, nothing lost: a Wrong card only comes back sooner. All on the RGB565 surface.
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
  void debugCmd(const char* cmd) override;   // "level<N>": a fresh deck starting on table N (1..10)
  const Save& save() const { return save_; }

  enum Page : uint8_t { P_HOME, P_ASK, P_ANSWER, P_DONE, P_COUNT };

 private:
  Save save_{};
  Round round_{};
  Page page_ = P_HOME;
  Input in_{};
  ui::FreshGate gate_;
  canvas::Frames<1> frames_;
  uint32_t ms_ = 0, now_ = 0;
  uint32_t* wallet_ = nullptr;   // the kid's stars (AppEnter::stars), shared with every game
  bool home_ = false, dirty_ = false;
  int32_t today() const { return dayOf(now_); }
  void go(Page p);
  void deal();
  void grade(Grade g);   // the kid's answer to the current card
  void draw() const;
};
}  // namespace cards
