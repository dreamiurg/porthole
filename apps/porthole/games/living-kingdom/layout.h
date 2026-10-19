#pragma once
#include "ui.h"

namespace kingdom::layout {
constexpr ui::Box HOME = {40, 126, 25, 22};
constexpr ui::Box PREV = {36, 122, 28, 22};
constexpr ui::Box MENU = {66, 122, 28, 22};
constexpr ui::Box NEXT = {96, 122, 28, 22};
constexpr int TEXT_X = 66, TEXT_Y = 126, TEXT_W = 348, TEXT_H = 222;
constexpr int FIRST_TEXT_Y = 216, FIRST_TEXT_H = 126;
constexpr int JOURNAL_TEXT_Y = 192, JOURNAL_TEXT_H = 162;
constexpr ui::Box CARD0 = {26, 33, 108, 27};
constexpr ui::Box CARD1 = {26, 62, 108, 27};
constexpr ui::Box CARD2 = {26, 91, 108, 27};
constexpr int CARD_PAD = 12, CAPTION_Y = 4, DETAIL_Y = 30;
constexpr ui::Box START = {68, 126, 52, 22};
constexpr ui::Box TITLE_EXIT = HOME;
constexpr ui::Box TOOL0 = {26, 42, 50, 24};
constexpr ui::Box TOOL1 = {80, 42, 50, 24};
constexpr ui::Box TOOL2 = {26, 70, 50, 24};
constexpr ui::Box TOOL3 = {80, 70, 50, 24};
constexpr ui::Box TOOL4 = {32, 98, 44, 22};
constexpr ui::Box TOOL5 = {80, 98, 44, 22};
constexpr ui::Box PARTY0 = {32, 37, 28, 49};
constexpr ui::Box PARTY1 = {66, 37, 28, 49};
constexpr ui::Box PARTY2 = {100, 37, 28, 49};
constexpr ui::Box SLOT0 = {26, 42, 34, 34};
constexpr ui::Box SLOT1 = {63, 42, 34, 34};
constexpr ui::Box SLOT2 = {100, 42, 34, 34};
constexpr ui::Box SLOT3 = {26, 82, 34, 34};
constexpr ui::Box SLOT4 = {63, 82, 34, 34};
constexpr ui::Box SLOT5 = {100, 82, 34, 34};
constexpr ui::Box PLACES[] = {{32,69,40,22}, {36,98,38,22}, {96,70,38,22},
                             {22,42,34,22}, {96,98,38,22}, {100,42,34,22}};
} // namespace kingdom::layout
