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
#include <stdio.h>
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
// A wide button under the content, above the bottom chord (My turn, Peek again, Let's find out, Back to today).
constexpr Box WIDE = {37, 121, 86, 22};
// A button's label, as the LVGL build laid it out: 14 px narrower than the button, 5 px in from its top and bottom,
// centered, line space 0.
constexpr Label buttonLabel(const Box& b, const font::Font* f) {
  return {{f, (int16_t)(b.w * 3 - 14), (int16_t)(b.h * 3 - 10), 0, Align::CENTER}, (int16_t)(b.x * 3 + 7), (int16_t)(b.y * 3 + 5), true};
}

// Lists: a pair of buttons under the title, then rows (three, or the pair and two) above Previous / Next.
constexpr Box PAIR[2] = {{28, 49, 51, 22}, {81, 49, 51, 22}};
constexpr int ROWS = 3;
constexpr Box ROW[ROWS] = {{28, 49, 104, 22}, {28, 73, 104, 22}, {28, 97, 104, 22}};
inline constexpr Label ROW_LABEL = buttonLabel(ROW[2], &FONT20);            // the lowest row: the tightest chord
inline constexpr Label PAIR_LABEL = buttonLabel(PAIR[0], &FONT20);
// "n / m" between Previous and Next; "n/m" when that is wider than its box (a long notebook).
inline void pageNumber(char (&s)[16], int page, int count) {
  snprintf(s, sizeof s, "%d / %d", page + 1, count);
  if (font::textWidth(*PAGE_NUMBER.box.font, s) > PAGE_NUMBER.box.w) snprintf(s, sizeof s, "%d/%d", page + 1, count);
}

// Story, ending and discovery text, as the old firmware read it: a story's pages up to the choice, an ending's or a
// discovery's are filled with the names, joined into one text (READ_BYTES at most) and flowed onto screens of this
// box by font::pageBreaks (READ_SCREENS at most), so a screen ends where the next word no longer fits, not where a
// content page does. The counter, "n / m", counts the screens. The content gate holds every text to both limits.
// A text's last screen gets at least MIN_LAST_WORDS words, moved off the end of the screen before: 4 turns every
// one- to three-word tail ("happening.") into a short phrase, and the screen that gives them up (measured: 16 words
// or more today) keeps at least 13. The gate holds every text to it, at the widest names and at short ones.
inline constexpr Label PAGE = {{&FONT24, 352, 176, 4, Align::LEFT}, 64, 146, false};
constexpr int READ_BYTES = 2048, READ_SCREENS = 16, MIN_LAST_WORDS = 4;
// The most pages any list or reader counts: the notebook with every discovery kept, two to a page.
constexpr int MAX_PAGES = 48;
// Library: Discoveries and Notebook, then two story rows: "Day N: <title>" while locked, "<title> *" once read.
inline constexpr Label STORY_BUTTON = ROW_LABEL;
// Not drawn by the LVGL firmware (only its web version showed it, under the title): one small line, button wide.
inline constexpr Label SUBTITLE = {{&FONT16, 346, 17, 0, Align::CENTER}, 67, 284, false};
// Choice: the prompt above two choice rows, lower than the list rows (nothing else is at the bottom).
constexpr Box CHOICE[2] = {{28, 86, 104, 22}, {28, 110, 104, 22}};
inline constexpr Label PROMPT = {{&FONT24, 352, 108, 4, Align::CENTER}, 64, 148, false};
inline constexpr Label CHOICE_BUTTON = buttonLabel(CHOICE[1], &FONT20);
// A discovery's cover: its title (FONT24 when that takes two lines at most, else FONT20), the picture at 3x under
// it, Let's find out. coverTitle() is the label the screen draws and the gate measures.
inline constexpr Label FACT_TITLE = {{&FONT20, 348, 76, 4, Align::CENTER}, 66, 142, false};
inline Label coverTitle(const char* s) {
  Label l = FACT_TITLE;
  if (font::textHeight(FONT24, s, l.box.w, l.box.spacing) <= l.box.h) l.box.font = &FONT24;
  return l;
}
constexpr int COVER_X = 96, COVER_Y = 218;
// Discovery and topic rows: the picture (96x48) at the left, the text in the rest, FONT20 while it takes two lines,
// else FONT16. rowText() is that label in the lowest row, as the screens draw it and the gate measures it.
constexpr int ROW_PICTURE_X = 7, ROW_PICTURE_Y = 9;   // in the row, physical
inline constexpr Label ROW_TEXT = {{&FONT16, 196, 56, 0, Align::CENTER}, 193, 296, true};
inline Label rowText(const char* s) {
  Label l = ROW_TEXT;
  if (font::textHeight(FONT20, s, l.box.w, 0) <= 2 * FONT20.lineHeight) l.box.font = &FONT20;
  return l;
}
// The notebook before anything is kept.
inline constexpr Label NOTEBOOK_EMPTY = {{&FONT24, 340, 90, 4, Align::CENTER}, 70, 226, false};
// The wonder page: "I wonder..." over the question, Source / Keep where Previous / Next are.
inline constexpr Label WONDER_TITLE = {{&FONT28, 328, 29, 4, Align::CENTER}, 76, 148, false};
inline constexpr Label WONDER = {{&FONT24, 352, 176, 4, Align::CENTER}, 64, 182, false};
// The source page: its name, then its address, then Back to our book.
inline constexpr Label SOURCE_NAME = {{&FONT24, 352, 64, 4, Align::CENTER}, 64, 156, false};
inline constexpr Label SOURCE_URL = {{&FONT16, 332, 138, 4, Align::LEFT}, 74, 220, false};
// Today: the adventure's description, then its three activities and A lovely word, two by two.
inline constexpr Label ADVENTURE = {{&FONT16, 350, 81, 4, Align::CENTER}, 65, 147, false};
constexpr Box TODAY[4] = {{28, 80, 51, 22}, {81, 80, 51, 22}, {28, 104, 51, 22}, {81, 104, 51, 22}};
inline constexpr Label TODAY_BUTTON = buttonLabel(TODAY[2], &FONT20);
// The pocket word's meaning, above Back to today.
inline constexpr Label MEANING = {{&FONT24, 352, 206, 4, Align::LEFT}, 64, 150, false};
// Sticker album: three rows 56 px apart from y 164, one line each.
constexpr int STICKER_ROWS = 3, STICKER_STEP = 56;
inline constexpr Label STICKER = {{&FONT24, 320, 52, 4, Align::CENTER}, 80, 164, false};

// ---- Home
// The way out to the launcher: the shell's orange home button (os/ui.h), top center, drawn at 3x.
constexpr int HOME_CX = 80, HOME_CY = 12, HOME_HIT_R = 16;   // logical, like ui::back
// The pup's name in capitals, left of the mood line ("Day 3 · puppy"), one line each: the name gets what the mood
// leaves (less a gap), in FONT20, else FONT16, else FONT16 as typed.
inline constexpr Label HOME_NAME = {{&FONT20, 150, 22, 4, Align::LEFT}, 96, 72, false};
inline constexpr Label HOME_MOOD = {{&FONT16, 212, 17, 4, Align::RIGHT}, 180, 74, false};
constexpr int HOME_NAME_GAP = 10;
constexpr int NEED_X[3] = {72, 188, 304}, NEED_Y = 100, NEED_W = 104, NEED_H = 34;
inline constexpr Label NEED_VALUE = {{&FONT16, 40, 17, 4, Align::CENTER}, 58, 6, false};   // inside the card
constexpr int BUBBLE_X = 72, BUBBLE_Y = 304, BUBBLE_W = 336, BUBBLE_H = 42;
inline constexpr Label BUBBLE = {{&FONT16, 322, 38, 4, Align::CENTER}, 79, 306, true};   // two lines at most
// Feed, Play, Pet, More: 72x66 physical each, low on the glass
constexpr Box ACTIONS[4] = {{28, 118, 24, 22}, {55, 118, 24, 22}, {82, 118, 24, 22}, {109, 118, 24, 22}};
// An action's label: one line under its icon, as wide as the button, 5 px off its bottom.
inline Label actionLabel(const Box& b, const font::Font* f) {
  return {{f, (int16_t)(b.w * 3), f->lineHeight, 0, Align::CENTER}, (int16_t)(b.x * 3), (int16_t)(b.y * 3 + b.h * 3 - 5 - f->lineHeight), false};
}
// The room: the bookshelf, the window (nap), the fern, and the pup itself
constexpr Box SHELF = {16, 49, 36, 49}, WINDOW = {110, 46, 32, 32}, FERN = {117, 80, 25, 22}, PUP = {54, 56, 53, 39};
// Fetch: the ball's five spots, the score and the way out
constexpr Box BALL[5] = {{34, 58, 27, 24}, {100, 66, 27, 24}, {66, 90, 27, 24}, {32, 93, 27, 24}, {98, 94, 27, 24}};
inline constexpr Label FETCH_SCORE = {{&FONT20, 350, 22, 4, Align::CENTER}, 65, 140, false};
constexpr Box ALL_DONE = {50, 124, 60, 22};

// ---- World
constexpr Box WORLD_BACK = {31, 18, 24, 22};
inline constexpr Label WORLD_NAMES = {{&FONT16, 222, 17, 4, Align::CENTER}, 172, 64, false};   // just the pup's if wider
inline constexpr Label WORLD_LINE = {{&FONT16, 352, 17, 4, Align::CENTER}, 64, 124, false};    // day, stage, stars
constexpr Box WORLD_TRICKS = {22, 48, 57, 32}, WORLD_NAP = {81, 48, 57, 32}, WORLD_BOOK = {22, 83, 57, 32},
              WORLD_TODAY = {81, 83, 57, 32};
inline constexpr Label TILE_TITLE = {{&FONT20, 118, 44, 0, Align::LEFT}, 42, 10, false};   // inside the tile
inline constexpr Label TILE_DETAIL = {{&FONT16, 150, 34, 0, Align::LEFT}, 10, 58, false};


// ---- Training: watch the cues, then tap them on the pad
inline constexpr Label TRAIN_HINT = {{&FONT24, 352, 60, 4, Align::CENTER}, 64, 160, false};
inline constexpr Label TRAIN_CUES = {{&FONT28, 348, 100, 4, Align::CENTER}, 66, 250, false};
constexpr Box CUE_KEY[5] = {   // indexed by Cue: Left, Up, Right, Down, Paw
  {31, 73, 30, 22}, {65, 49, 30, 22}, {99, 73, 30, 22}, {65, 97, 30, 22}, {65, 73, 30, 22}};

// ---- Scrapbook (the profile page): Our stickers and Rename pup where Previous and Next are
inline constexpr Label BOOK_BADGE = {{&FONT24, 330, 30, 4, Align::CENTER}, 75, 150, false};
inline constexpr Label BOOK_LINES = {{&FONT20, 340, 130, 4, Align::CENTER}, 70, 196, false};   // 5 lines: long names wrap
}  // namespace biscuit
