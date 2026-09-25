// Where Biscuit draws each content string: the label box (font, width, the most height the text may take, line
// spacing, alignment) and its top-left on the panel, physical px. Taken from the LVGL firmware's screens
// (biscuit-v0.1.0 main.cpp): label() is a LONG_WRAP label with line space 4; a button pads 5 on each side and
// holds a label 14 px narrower than itself with line space 0, centered. h runs to the next thing on the screen.
// host/test_biscuit_content.cpp holds every content string to these boxes and to the round glass, so the screens
// draw with these constants, not numbers of their own: change a box here and the gate follows.
#pragma once
#include "font.h"
#include "generated/fonts.h"

namespace biscuit {
// middle: the text is centered vertically in the h-tall box (a button's label), else it starts at y. For a column
// of buttons, y is the row farthest from the panel's center, where the chord is tightest.
struct Label { font::Box box; int16_t x, y; bool middle; };

// Story, ending and discovery pages. The screen splits a content page with font::pageBreaks into at most
// PAGE_SCREENS screens and shows "n / m" under it, as the LVGL build did ("1 / 2" on a discovery).
inline constexpr Label PAGE = {{&FONT24, 352, 176, 4, font::Align::LEFT}, 64, 146, false};
constexpr int PAGE_SCREENS = 2;
// The screen title under Back and the stars: topic names, adventure titles, pocket words, trick names. One line.
inline constexpr Label HEADER = {{&FONT24, 340, 46, 4, font::Align::CENTER}, 70, 100, false};
// Library: two story buttons (60, 213 + 66j, 360x60), "Day N: <title>" while locked, "<title> *" once read.
inline constexpr Label STORY_BUTTON = {{&FONT20, 346, 50, 0, font::Align::CENTER}, 67, 284, true};
// Not drawn by the LVGL firmware (only its web version showed it, under the title): one small line, button wide.
inline constexpr Label SUBTITLE = {{&FONT16, 346, 17, 0, font::Align::CENTER}, 67, 284, false};
// Choice: the prompt above two choice buttons (78, 256 + 80j, 324x68).
inline constexpr Label PROMPT = {{&FONT24, 352, 108, 4, font::Align::CENTER}, 64, 148, false};
inline constexpr Label CHOICE_BUTTON = {{&FONT20, 310, 58, 0, font::Align::CENTER}, 85, 341, true};
// A discovery's first page, above its picture at y 228. The firmware used FONT24 when the title fit in 76 px, else
// this FONT20 box: the gate checks the fallback.
inline constexpr Label FACT_TITLE = {{&FONT20, 348, 82, 4, font::Align::CENTER}, 66, 146, false};
// Discovery list: two buttons (66, 210 + 70j, 348x64), the picture on the left, the title in a 230 px label at the
// right. FONT20 when the title fit in 52 px, else this FONT16 box: the gate checks the fallback.
inline constexpr Label FACT_BUTTON = {{&FONT16, 230, 54, 0, font::Align::CENTER}, 179, 285, true};
// Topics list: three buttons (82, 153 + 61j, 316x56), the picture on the left, the name in a 200 px label.
inline constexpr Label TOPIC_BUTTON = {{&FONT20, 200, 46, 0, font::Align::CENTER}, 193, 280, true};
// The wonder page, under "I wonder..." and above Source / Keep at y 374.
inline constexpr Label WONDER = {{&FONT24, 352, 180, 4, font::Align::CENTER}, 64, 194, false};
// The source page: its name, then its address.
inline constexpr Label SOURCE_NAME = {{&FONT24, 352, 64, 4, font::Align::CENTER}, 64, 156, false};
inline constexpr Label SOURCE_URL = {{&FONT16, 332, 154, 4, font::Align::LEFT}, 74, 220, false};
// Today: the adventure's description above its three action buttons at y 228.
inline constexpr Label ADVENTURE = {{&FONT16, 350, 81, 4, font::Align::CENTER}, 65, 147, false};
// The pocket word's meaning, above Back to today at y 374.
inline constexpr Label MEANING = {{&FONT24, 352, 218, 4, font::Align::LEFT}, 64, 156, false};
// Tricks list: three buttons (82, 153 + 61j, 316x56), "<name>   N/3", or "<name>   Day N" while locked.
inline constexpr Label TRICK_BUTTON = {{&FONT20, 302, 46, 0, font::Align::CENTER}, 89, 280, true};
// Sticker album: three rows 56 px apart from y 164, one line each.
inline constexpr Label STICKER = {{&FONT24, 320, 52, 4, font::Align::CENTER}, 80, 164, false};
}  // namespace biscuit
