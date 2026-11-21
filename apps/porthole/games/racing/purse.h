// Every star Rally touches, in one place: the kid's balance, a garage purchase and its Undo, and what a finish pays.
// The shell keeps the wallet (os/stars.h, AppEnter::stars); the skills engine caps what games that teach nothing pay a
// day (os/skills.h earnForPlay, PLAY_DAY across all of them, kept in the kid's AppEnter::book). Header-only.
#pragma once
#include <stdint.h>
#include "day.h"
#include "skills.h"
#include "stars.h"
#include "together.h"

namespace racing::purse {
inline uint32_t balance(const uint32_t* wallet) { return stars::balance(wallet); }
inline bool spend(uint32_t* wallet, uint32_t n) { return stars::spend(wallet, n); }   // false: too few, nothing taken
inline void refund(uint32_t* wallet, uint32_t n) { stars::award(wallet, n); }          // an Undo in the garage
inline void fixture(uint32_t* wallet, uint32_t n) { if (wallet) *wallet = n < stars::MAX ? n : stars::MAX; }   // dbg only

// Rally's pay (a user decision of 2026-11-21): a solo finish pays SOLO_STARS for the first SOLO_RACES a day, a finish
// together together::STARS for the first together::STAR_RACES, any place; the engine stops all play stars at
// skills::PLAY_DAY a day. The race counts are the clock's (UTC) day and a reboot forgets them; the engine's cap is
// saved with the book.
constexpr uint32_t SOLO_STARS = 1, SOLO_RACES = 5;
struct Day { uint32_t day = 0; uint8_t solo = 0, joint = 0; };
inline uint32_t payFinish(skills::Book* book, uint32_t* wallet, Day& d, uint32_t nowSec, bool together) {
  const uint32_t today = os::dayOf(nowSec);
  if (os::newDay(today, d.day)) d = {today};
  uint8_t& races = together ? d.joint : d.solo;
  if (races < 255) races++;
  const uint32_t first = together ? (uint32_t)together::STAR_RACES : SOLO_RACES;
  const uint32_t pay = races > first ? 0 : together ? (uint32_t)together::STARS : SOLO_STARS;
  return pay ? skills::earnForPlay(book, wallet, nowSec, pay) : 0;
}
}  // namespace racing::purse
