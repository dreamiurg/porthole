// Rally: a short, silent arcade race, stateless between runs. Every launcher entry starts at the track cards.
#pragma once
#include "app.h"
#include "controls.h"
#include "garage.h"
#include "purse.h"
#include "race.h"
#include "render.h"
#include "save.h"
#include "together.h"
#include "ui.h"

namespace racing {
constexpr uint32_t RESUME_MS = 1000;   // the Ready beat before a paused race moves again
class Game : public App {
 public:
  const char* name() const override { return "Rally"; }
  const gfx::Sprite& icon() const override { return paint::icon(); }
  const char* store() const override { return "racing"; }
  void enter(const AppEnter& e) override;
  void update(uint32_t nowSec, uint32_t ms, const Input& in) override;
  void render() override;
  Surface surface() const override { return SURFACE_RGB565; }
  Tint tint() const override { return daylight_ ? TINT_DAY : clockTint(now_); }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void** data, size_t* len, bool allowed) override;
  bool saveUrgent() const override { return urgent_; }
  bool wantsHome() override { const bool w = home_; home_ = false; return w; }
  void leave() override;
  const char* screenName() const override;
  void debugPrint() override;
  void debugCmd(const char* cmd) override;
  const Race& race() const { return race_; }
  float steering() const { return steer_; }
  using Screen = paint::Page;   // one page per screen
  Screen screen() const { return screen_; }
  paint::Duo duo() { paint::Duo d; duoView(d); return d; }   // the race together as the pages show it

 private:
  Screen screen_ = Screen::Tracks;
  Race race_{};
  Input in_{};
  controls::Calibration calibration_;
  tilt::Grav neutral_ = {0, 1000, 0}, grip_ = {0, 1000, 0};   // the race's straight ahead; the grip page's candidate
  ui::FreshGate gate_;
  ui::Hold hold_;
  paint::Leave leave_ = paint::Leave::Restart;
  uint32_t now_ = 0, rawMs_ = 0, ms_ = 0, stepMs_ = 0, resumeMs_ = 0, badGripMs_ = 0, finishMs_ = 0, seed_ = 1, runs_ = 0;
  int track_ = 0;
  float steer_ = 0;
  bool home_ = false, daylight_ = false, seenHowTo_ = false, recenter_ = false, gripLost_ = false, gripSet_ = false;
  bool boost_ = false, rocket_ = false;   // fresh presses waiting for the next simulation step
  // The leaderboard: every profile's best times (their saves, read on entry), this kid's own among them.
  struct Kid { char name[12]; Best best[TRACKS]; uint16_t stock[TRACKS], power; bool me; };   // as in Save
  Kid kids_[MAX_PROFILES] = {};
  int nKids_ = 0, me_ = -1;
  Save save_{};
  bool dirty_ = false, recorded_ = false, urgent_ = false;   // urgent_: a setting change, checkpointed at once
  uint8_t newBest_ = 0;            // BEST_RACE / BEST_LAP bits from this finish
  uint32_t prevBestMs_ = 0;        // the kid's best race here before this finish (0: first time)
  // The garage (garage.h): the page's item, this visit's purchases for Undo, the not-enough-stars hint's timer.
  garage::Item item_{};
  garage::Visit visit_{};
  uint32_t hintMs_ = 0, resultsMs_ = 0;
  int8_t rankUp_ = -1;             // the rank this finish reached, for the Results banner (-1: none)
  bool boardStock_ = false;        // Times: the Stock board (parts off) rather than Tuned (every time)
  purse::Day pay_;                 // today's paid finishes (purse.h)

  tilt::Grav gravity() const { return {in_.gx, in_.gy, in_.gz}; }
  bool gripReady() const { return gripSet_ && controls::upright(gravity()); }   // a steady grip, still held like a wheel
  void go(Screen s);
  void startRace();
  void updateTracks();
  void updateHowTo();
  void updateCalibrate();
  void updateRace(uint32_t dt);
  void updatePause();
  void updateConfirm();
  void updateResults();
  void updateBoard();
  void updateGarage();
  void updateShop();
  void updateBuy();
  void shopStep(int by);
  void dirtyNow() { dirty_ = urgent_ = true; }   // a purchase or a switch: checkpointed at once, with the stars
  bool raceStock() const { return garage::racePower(save_) == 0; }   // the class the next race counts in
  void countFinish();
  bool gripOk(uint32_t dt);
  void simulate(uint32_t dt);
  void debugFixture(const char* cmd);
  void loadKids(const AppEnter& e);
  void recordFinish();
  void seedLevels();
  int raceLevel() const;   // the level this kid's next race on this track runs at
  int board(paint::Row out[MAX_PROFILES], bool stock) const;
  int levelOn(int track) const;   // the level this kid's next race on `track` runs at
  void fresh();                   // a new run's timers and flags, then the race page

  // Together: a race with a kid on another Porthole on the same wifi (together.h). game_together.cpp.
  together::Link link_;
  together::Wire wire_{}, peer_{};   // what this device says; the other kid's last word (kept after they go)
  bool duo_ = false, host_ = false, gripDone_ = false, raced_ = false, gone_ = false, peerPaused_ = false;
  bool ready_ = false;   // this kid tapped Race! on the track cards
  uint8_t raceId_ = 0, applied_[CARS] = {}, pausesLeft_ = 0, starsGot_ = 0;
  uint32_t lastGen_ = 0, noticeMs_ = 0;
  uint32_t* wallet_ = nullptr;
  skills::Book* book_ = nullptr;   // the engine that caps play stars a day (purse.h)
  char notice_[32] = {};
  const gfx::Sprite* avatars_ = nullptr;
  NewSpots* news_ = nullptr;   // the red dot on Race! (Kid news)
  int nAvatars_ = 0;
  uint16_t facePx_[3][20 * 20] = {};   // this kid, the other kid, a kid on the cards: the avatars in RGB565
  pic::Pic faces_[3] = {};
  void togetherTick(uint32_t dt);
  void fillWire();
  together::Stage stageNow() const;
  void onPeer();
  void peerGone();
  void joinInvite();
  void endTogether();
  void say(const char* fmt, const char* name);
  bool tracksTogether();
  void updateLobby();
  void awaitStart();
  void startJointRace();
  void finishLine(uint32_t dt);
  void jointFinish();
  bool peerHold();                // the other kid paused: this race waits, then a Ready beat
  float peerDist() const;
  float windTop() const;
  void places(uint8_t* mine, uint8_t* theirs) const;
  const pic::Pic* face(int slot, int avatar);
  void duoView(paint::Duo& d);
  void peerView(paint::Duo& d);
  bool sameRace() const;   // the other kid's datagrams are about this race (not the last one, or none yet)
};
}  // namespace racing
