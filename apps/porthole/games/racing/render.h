// Rally's look: a pseudo-3D chase view drawn straight into the 480x480 RGB565 target every frame, and the menu pages.
// Hit boxes are logical 160x160 pixels; every corner of every box sits inside the round glass (host/test_racing.cpp).
#pragma once
#include "garage.h"
#include "gfx.h"
#include "pic.h"
#include "race.h"
#include "ui.h"

namespace racing::paint {
enum class Page : uint8_t { Tracks, HowTo, Calibrate, Race, Pause, Confirm, Results, Board, Lobby, Garage, Shop, Buy };
constexpr ui::Box CARD[TRACKS] = {{27, 33, 52, 36}, {81, 33, 52, 36}, {27, 71, 52, 36}, {81, 71, 52, 36}};
constexpr ui::Box HOME{25, 110, 24, 24};                          // Tracks: back to the launcher
constexpr ui::Box TIMES{51, 110, 26, 24};                         // Tracks: every kid's best times, no race needed
constexpr ui::Box CAR_BTN{79, 110, 24, 24};                       // Tracks: the garage
constexpr ui::Box MODE_CHIP{105, 110, 30, 24};                    // Tracks: tap to cycle the difficulty setting
constexpr ui::Box TOGETHER{48, 9, 64, 22};   // Tracks: Race! with a kid on another Porthole's cards (shown only then)
constexpr ui::Box PREV_TRACK{26, 24, 24, 24}, NEXT_TRACK{110, 24, 24, 24}, BOARD_BACK{56, 128, 48, 24};   // Times
constexpr ui::Box TUNED_CHIP{48, 50, 30, 22}, STOCK_CHIP{82, 50, 30, 22};   // Times: which board
// The garage. Confirm's Buy sits clear of every button of the item page, so a double tap never passes both.
constexpr ui::Box STOCK_SW{126, 62, 28, 30}, LOOKS_BTN{28, 96, 50, 24}, POWER_BTN{82, 96, 50, 24};
constexpr ui::Box GARAGE_BACK{56, 126, 48, 24};
constexpr ui::Box SHOP_BACK{56, 6, 48, 22}, GROUP_CHIP{46, 31, 68, 22}, PREV_ITEM{6, 60, 24, 30}, NEXT_ITEM{130, 60, 24, 30};
constexpr ui::Box MAIN{46, 118, 68, 24}, SPLIT_UNDO{34, 118, 44, 24}, SPLIT_MAIN{82, 118, 44, 24};
constexpr ui::Box BUY_YES{34, 84, 44, 24}, BUY_NO{82, 84, 44, 24};
constexpr uint32_t HINT_MS = 2000, BANNER_MS = 2500;   // the not-enough-stars hint; the new-rank banner on Results
constexpr uint8_t NO_POWER = 255;                      // a Stock board row: no Power badge
constexpr const char* MODE_NAME[] = {"Auto", "Easy", "Normal", "Hard"};   // by Mode (save.h)
constexpr ui::Box BACK{30, 112, 48, 24}, NEXT{82, 112, 48, 24};   // How to play, Calibrate
constexpr ui::Box LAP{26, 28, 32, 24};                            // hold to pause
constexpr ui::Box PLACE{100, 30, 34, 18};                         // shown only, never tapped
constexpr ui::Box ROCKET_BTN{5, 66, 26, 28}, BOOST_BTN{129, 66, 26, 28};   // 9 and 3 o'clock, at the rim, off the road view
constexpr ui::Box RESUME{40, 36, 80, 24}, RECENTER{26, 66, 52, 24}, RESTART{82, 66, 52, 24};
constexpr ui::Box TRACKS_BTN{26, 96, 52, 24}, HOME_BTN{82, 96, 52, 24};
constexpr ui::Box YES{30, 84, 48, 24}, NO{82, 84, 48, 24};
constexpr ui::Box REPLAY{26, 100, 52, 24}, RESULT_TRACKS{82, 100, 52, 24}, RESULT_HOME{62, 128, 36, 24};
enum class Leave : uint8_t { Restart, Tracks, Home };

struct Row { const char* name; uint32_t raceMs, lapMs; bool me; uint8_t power; };   // a leaderboard line, power: P<n>
// The garage as its pages show it (garage.h).
struct Shop {
  const Save* save = nullptr;
  uint32_t stars = 0;                     // the kid's balance
  garage::Item item;                      // the item page's
  const garage::Visit* visit = nullptr;   // this visit's purchases (Undo)
  bool hint = false;                      // "Stars come from..." after a tap on a price the kid can't pay yet
  bool dot = false;                       // something new to buy: the gold dot on the Car button
};
// A race together (together.h), as the pages show it. Faces: the profiles' avatars, 20x20.
struct Duo {
  bool on = false;              // in a together session (lobby, grip, race, results)
  bool radio = false;           // this build has a network (no Together button without one)
  uint8_t net = 0;              // lan::State
  bool host = false, paired = false, waiting = false;   // waiting: this kid's grip is set, the other's is not
  const pic::Pic* face = nullptr;     // this kid's
  const pic::Pic* peerFace = nullptr;
  const char* peerName = "";
  bool peerOnRoad = false, peerBoost = false, peerPaused = false, peerFinished = false, peerAgain = false;
  bool gone = false, noWinner = false, holdOff = false;
  float peerDist = 0, peerX = 0, wind = 0;   // wind: the tailwind's help, 0..1
  uint8_t place = 0, peerPlace = 0;          // 1..5 among both kids and the rivals
  uint8_t races = 0, stars = 0;              // races together today, stars this finish earned
  uint32_t peerFinishMs = 0;
  const char* inviteName = nullptr;          // a kid on another Porthole's track cards
  const pic::Pic* inviteFace = nullptr;
  bool ready = false, peerReady = false;     // this kid / that kid tapped Race!
  bool isNew = false;                        // Race! is news to this kid (Kid news): the red dot
};
struct View {
  Page page = Page::Tracks;
  const Race* race = nullptr;
  int track = 0;
  uint32_t ms = 0;            // the page clock, for animation only
  float steer = 0;            // smoothed wheel, -1..1
  uint16_t calibration = 0;   // permille
  uint16_t hold = 0;          // lap-chip hold, permille
  uint32_t resumeMs = 0;      // the "ready" cue before a paused race moves again
  bool gripReady = false, gripUpright = false, gripHeld = false, gripLost = false;
  Leave leave = Leave::Restart;
  const Row* board = nullptr;   // this track's best times across every profile, fastest first
  int boardRows = 0;
  uint8_t newBest = 0;          // BEST_RACE / BEST_LAP: what this finish improved on the kid's own record here
  uint32_t prevBestMs = 0;      // the kid's best race here before this finish, for "+1.9" when slower
  uint8_t mode = 0;             // the kid's difficulty setting (save.h Mode), on the track cards
  const char* notice = nullptr; // a passing line: the other kid went home, the race is off
  Duo duo;
  bool boardStock = false;      // the board shown is the Stock one (Times, Results, the time to beat)
  uint8_t paint = 0, stripe = 0;   // the kid's car (garage.h); stock in a race together
  uint8_t earned = 0;           // stars this finish paid
  int8_t rankUp = -1;           // a rank just reached, for its banner on Results
  Shop shop;
};
void draw(const View& v);
const gfx::Sprite& icon();
// What is drawn, for the host tests to hold the race to: a car's picture covers its dist up to carReach() m ahead and
// carHalf() road half-widths either side; an obstacle's, obstacleHalf() either side of its x.
float carReach();
float carHalf();
float obstacleHalf(int track, const Feature& o);
float pxPerHalfWidth();   // screen px in a road half-width at the car's depth
}  // namespace racing::paint
