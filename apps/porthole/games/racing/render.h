// Rally's look: a pseudo-3D chase view drawn straight into the 480x480 RGB565 target every frame, and the menu pages.
// Hit boxes are logical 160x160 pixels; every corner of every box sits inside the round glass (host/test_racing.cpp).
#pragma once
#include "gfx.h"
#include "race.h"
#include "ui.h"

namespace racing::paint {
enum class Page : uint8_t { Tracks, HowTo, Calibrate, Race, Pause, Confirm, Results, Board };
constexpr ui::Box CARD[TRACKS] = {{27, 33, 52, 38}, {81, 33, 52, 38}, {27, 75, 52, 38}, {81, 75, 52, 38}};
constexpr ui::Box HOME{32, 117, 28, 24};                          // Tracks: back to the launcher
constexpr ui::Box TIMES{62, 117, 32, 24};                         // Tracks: every kid's best times, no race needed
constexpr ui::Box MODE_CHIP{96, 117, 32, 24};                     // Tracks: tap to cycle the difficulty setting
constexpr ui::Box PREV_TRACK{22, 32, 26, 24}, NEXT_TRACK{112, 32, 26, 24}, BOARD_BACK{56, 120, 48, 24};   // Times
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

struct Row { const char* name; uint32_t raceMs, lapMs; bool me; };   // a leaderboard line
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
};
void draw(const View& v);
const gfx::Sprite& icon();
}  // namespace racing::paint
