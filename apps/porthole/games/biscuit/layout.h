// Biscuit's screen geometry, one place for the screens and the content gate (host/test_biscuit_content.cpp holds
// every content string to these boxes and to the round glass): the screens draw with these constants, not numbers of
// their own, so change a box here and the gate follows.
// Labels are physical px on the 480x480 panel: the label box (font, width, the most height the text may take, line
// spacing, alignment) and its top-left. They start from the LVGL firmware's screens (biscuit-v0.1.0 main.cpp):
// label() is a LONG_WRAP label with line space 4; a button pads 5 on each side and holds a label 14 px narrower than
// itself with line space 0, centered. h runs to the next thing on the screen.
// Hit boxes are logical px (1 = 3 physical, os/input.h), at least 24x22 (8 mm), inside the round glass, 2 px apart:
// the UI audit (make playtest) holds every screen to that at zero findings. That floor moved several of the LVGL
// build's controls (Back 86x56 -> 90x66, rows 56 -> 66, Previous/Next 128x56 -> 120x66), and the labels with them.
#pragma once
#include "font.h"
#include "generated/fonts.h"
#include "ui.h"

namespace biscuit {
using Box = ui::Box;   // logical
// middle: the text is centered vertically in the h-tall box (a button's label), else it starts at y. For a column
// of buttons, y is the row farthest from the panel's center, where the chord is tightest.
struct Label { font::Box box; int16_t x, y; bool middle; };
using font::Align;

// ---- every screen but Home and World: Back, the story stars, a title
constexpr Box BACK = {36, 14, 30, 22};
inline constexpr Label STARS = {{&FONT20, 120, 22, 4, Align::CENTER}, 246, 64, false};
// The screen title under Back and the stars: topic names, adventure titles, pocket words, trick names. One line.
inline constexpr Label HEADER = {{&FONT24, 340, 35, 4, Align::CENTER}, 70, 112, false};
// Previous / page / Next along the bottom chord
constexpr Box PREV = {32, 121, 40, 22}, NEXT = {88, 121, 40, 22};
inline constexpr Label PAGE_NUMBER = {{&FONT16, 48, 17, 4, Align::CENTER}, 216, 387, false};
// A wide button under the content, above the bottom chord (My turn, Peek again, Rename pup).
constexpr Box WIDE = {37, 121, 86, 22};

// Story, ending and discovery pages. The screen splits a content page with font::pageBreaks into at most
// PAGE_SCREENS screens and shows "n / m" under it, as the LVGL build did ("1 / 2" on a discovery).
inline constexpr Label PAGE = {{&FONT24, 352, 176, 4, Align::LEFT}, 64, 146, false};
constexpr int PAGE_SCREENS = 2;
// TODO(biscuit 7b): the Library, Choice, Discovery, Source, Today and Sticker screens re-tune these to the 8 mm
// controls the way HEADER and TRICK_BUTTON already are; until then they are the LVGL build's.
// Library: two story buttons (60, 213 + 66j, 360x60), "Day N: <title>" while locked, "<title> *" once read.
inline constexpr Label STORY_BUTTON = {{&FONT20, 346, 50, 0, Align::CENTER}, 67, 284, true};
// Not drawn by the LVGL firmware (only its web version showed it, under the title): one small line, button wide.
inline constexpr Label SUBTITLE = {{&FONT16, 346, 17, 0, Align::CENTER}, 67, 284, false};
// Choice: the prompt above two choice buttons (78, 256 + 80j, 324x68).
inline constexpr Label PROMPT = {{&FONT24, 352, 108, 4, Align::CENTER}, 64, 148, false};
inline constexpr Label CHOICE_BUTTON = {{&FONT20, 310, 58, 0, Align::CENTER}, 85, 341, true};
// A discovery's first page, above its picture at y 228. The firmware used FONT24 when the title fit in 76 px, else
// this FONT20 box: the gate checks the fallback.
inline constexpr Label FACT_TITLE = {{&FONT20, 348, 82, 4, Align::CENTER}, 66, 146, false};
// Discovery list: two buttons (66, 210 + 70j, 348x64), the picture on the left, the title in a 230 px label at the
// right. FONT20 when the title fit in 52 px, else this FONT16 box: the gate checks the fallback.
inline constexpr Label FACT_BUTTON = {{&FONT16, 230, 54, 0, Align::CENTER}, 179, 285, true};
// Topics list: three buttons (82, 153 + 61j, 316x56), the picture on the left, the name in a 200 px label.
inline constexpr Label TOPIC_BUTTON = {{&FONT20, 200, 46, 0, Align::CENTER}, 193, 280, true};
// The wonder page, under "I wonder..." and above Source / Keep at y 374.
inline constexpr Label WONDER = {{&FONT24, 352, 180, 4, Align::CENTER}, 64, 194, false};
// The source page: its name, then its address.
inline constexpr Label SOURCE_NAME = {{&FONT24, 352, 64, 4, Align::CENTER}, 64, 156, false};
inline constexpr Label SOURCE_URL = {{&FONT16, 332, 154, 4, Align::LEFT}, 74, 220, false};
// Today: the adventure's description above its three action buttons at y 228.
inline constexpr Label ADVENTURE = {{&FONT16, 350, 81, 4, Align::CENTER}, 65, 147, false};
// The pocket word's meaning, above Back to today at y 374.
inline constexpr Label MEANING = {{&FONT24, 352, 218, 4, Align::LEFT}, 64, 156, false};
// Sticker album: three rows 56 px apart from y 164, one line each.
inline constexpr Label STICKER = {{&FONT24, 320, 52, 4, Align::CENTER}, 80, 164, false};

// ---- Home
// The way out to the launcher: the shell's orange home button (os/ui.h), top center, drawn at 3x.
constexpr int HOME_CX = 80, HOME_CY = 12, HOME_HIT_R = 16;   // logical, like ui::back
// The pup's name in capitals, left of the mood line ("Day 3 · puppy"), one line each: the name gets what the mood
// leaves (less a gap), in FONT20, else FONT16, else FONT16 as typed.
inline constexpr Label HOME_NAME = {{&FONT20, 150, 22, 4, Align::LEFT}, 96, 72, false};
inline constexpr Label HOME_MOOD = {{&FONT16, 160, 17, 4, Align::RIGHT}, 232, 74, false};
constexpr int HOME_NAME_GAP = 10;
constexpr int NEED_X[3] = {72, 188, 304}, NEED_Y = 100, NEED_W = 104, NEED_H = 34;
inline constexpr Label NEED_VALUE = {{&FONT16, 40, 17, 4, Align::CENTER}, 58, 6, false};   // inside the card
constexpr int BUBBLE_X = 72, BUBBLE_Y = 304, BUBBLE_W = 336, BUBBLE_H = 42;
inline constexpr Label BUBBLE = {{&FONT16, 322, 38, 4, Align::CENTER}, 79, 306, true};   // two lines at most
// Feed, Play, Pet, More: 72x66 physical each, low on the glass
constexpr Box ACTIONS[4] = {{28, 118, 24, 22}, {55, 118, 24, 22}, {82, 118, 24, 22}, {109, 118, 24, 22}};
// The room: the bookshelf, the window (nap), the fern, and the pup itself
constexpr Box SHELF = {16, 49, 36, 49}, WINDOW = {110, 46, 32, 32}, FERN = {117, 80, 25, 22}, PUP = {54, 56, 53, 39};
// Fetch: the ball's five spots, the score and the way out
constexpr Box BALL[5] = {{34, 58, 27, 24}, {100, 66, 27, 24}, {66, 90, 27, 24}, {32, 93, 27, 24}, {98, 94, 27, 24}};
inline constexpr Label FETCH_SCORE = {{&FONT20, 350, 22, 4, Align::CENTER}, 65, 140, false};
constexpr Box ALL_DONE = {50, 124, 60, 22};

// ---- World
constexpr Box WORLD_BACK = {31, 18, 24, 22};
inline constexpr Label WORLD_NAMES = {{&FONT16, 222, 17, 4, Align::CENTER}, 172, 64, false};   // just the pup's if wider
inline constexpr Label WORLD_LINE = {{&FONT16, 352, 17, 4, Align::CENTER}, 64, 124, false};    // day, stage, friendship, stars
constexpr Box WORLD_TRICKS = {22, 48, 57, 32}, WORLD_NAP = {81, 48, 57, 32}, WORLD_BOOK = {51, 83, 57, 32};
inline constexpr Label TILE_TITLE = {{&FONT20, 118, 44, 0, Align::LEFT}, 42, 10, false};   // inside the tile
inline constexpr Label TILE_DETAIL = {{&FONT16, 150, 34, 0, Align::LEFT}, 10, 58, false};

// ---- Tricks: three rows a page, two pages; "<name>   N/3", or "<name>   Day N" while locked
constexpr int TRICK_ROWS = 3;
constexpr Box TRICK_ROW[TRICK_ROWS] = {{28, 49, 104, 22}, {28, 73, 104, 22}, {28, 97, 104, 22}};
inline constexpr Label TRICK_BUTTON = {{&FONT20, 298, 56, 0, Align::CENTER}, 91, 296, true};   // the lowest row

// ---- Training: watch the cues, then tap them on the pad
inline constexpr Label TRAIN_HINT = {{&FONT24, 352, 60, 4, Align::CENTER}, 64, 160, false};
inline constexpr Label TRAIN_CUES = {{&FONT28, 348, 100, 4, Align::CENTER}, 66, 250, false};
constexpr Box CUE_KEY[5] = {   // indexed by Cue: Left, Up, Right, Down, Paw
  {31, 73, 30, 22}, {65, 49, 30, 22}, {99, 73, 30, 22}, {65, 97, 30, 22}, {65, 73, 30, 22}};

// ---- Scrapbook (the profile page)
inline constexpr Label BOOK_BADGE = {{&FONT24, 330, 30, 4, Align::CENTER}, 75, 150, false};
inline constexpr Label BOOK_LINES = {{&FONT20, 340, 110, 4, Align::CENTER}, 70, 196, false};
}  // namespace biscuit
