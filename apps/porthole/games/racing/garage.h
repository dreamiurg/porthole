// Rally's garage (a user decision of 2026-11-21; design: docs/superpowers/specs/porthole-racing/garage.md): what a
// kid buys with stars for their own car. Power parts (tune.h KIT_*: five parts, three tiers each, bought in order,
// the second and third tier opened by the kid's rank), paint and stripes (looks only). Every price shows before a buy
// and never changes; an Undo gives every star back until the kid leaves the garage (Visit, never saved); nothing is
// ever lost or sold. Stars go through purse.h only. Header-only.
#pragma once
#include <stdint.h>
#include "purse.h"
#include "save.h"

namespace racing::garage {
constexpr uint16_t PART_PRICE[TIERS] = {10, 25, 50};   // by tier, the same for every part
constexpr const char* PART_NAME[PARTS] = {"Engine", "Turbo", "Tires", "Suspension", "Nitro tank"};
constexpr const char* PART_GAIN[PARTS] = {"Top speed", "Pickup", "Grip", "Bumps", "Boost time"};   // "Grip 2/3"
constexpr const char* PART_LINE[PARTS] = {"A bit faster on the road", "Speeds up quicker", "Turns faster, slides less",
                                          "Less slow in mud and grass", "Boosts last longer"};
constexpr const char* PAINT_NAME[PAINTS] = {"Red", "Blue", "Lime", "Purple", "Pink", "White", "Black", "Gold", "Chrome"};
constexpr uint16_t PAINT_PRICE[PAINTS] = {0, 5, 5, 5, 10, 10, 10, 20, 30};
constexpr const char* STRIPE_NAME[STRIPES] = {"No stripe", "Racing stripe", "Flames", "Lightning"};
constexpr uint16_t STRIPE_PRICE[STRIPES] = {0, 10, 15, 15};
// Ranks: any finish counts, any place, any mode, together too. A part's tier n needs rank n - 1. Never called "Level"
// on screen: that is the rivals' hidden level.
enum Rank : uint8_t { ROOKIE, RACER, CHAMPION, RANKS };
constexpr const char* RANK_NAME[RANKS] = {"Rookie", "Racer", "Champion"};
constexpr uint8_t RANK_AT[RANKS] = {0, 5, 15};   // races finished
// The rival color a paint would be mistaken for (render.cpp: that rival drives stock red instead); 0 = none.
constexpr uint8_t RIVAL_LOOKALIKE[PAINTS] = {0, 1, 3, 0, 0, 0, 0, 2, 0};   // Blue, Lime (green), Gold

inline int rank(int finishes) { return finishes >= RANK_AT[CHAMPION] ? CHAMPION : finishes >= RANK_AT[RACER] ? RACER : ROOKIE; }
inline int tier(const Save& s, int part) { return s.tiers >> (2 * part) & 3; }
inline void setTier(Save& s, int part, int t) { s.tiers = (s.tiers & ~(3u << 2 * part)) | (unsigned)t << 2 * part; }
inline int power(const Save& s) { int p = 0; for (int i = 0; i < PARTS; i++) p += tier(s, i); return p; }   // 0..15
// What the kid races with: the parts, or none with the Stock switch on (looks stay either way).
inline void kit(const Save& s, uint8_t out[PARTS]) { for (int i = 0; i < PARTS; i++) out[i] = s.stockOn ? 0 : (uint8_t)tier(s, i); }
inline int racePower(const Save& s) { return s.stockOn ? 0 : power(s); }

enum Group : uint8_t { PAINT, STRIPE, PART, GROUPS };
struct Item { uint8_t group = PAINT, index = 0; };
constexpr int count(int group) { return group == PAINT ? PAINTS : group == STRIPE ? STRIPES : PARTS; }
inline const char* name(Item it) { return it.group == PAINT ? PAINT_NAME[it.index] : it.group == STRIPE ? STRIPE_NAME[it.index] : PART_NAME[it.index]; }
inline uint16_t lookBit(Item it) { return it.group == PAINT ? paintBit(it.index) : it.group == STRIPE ? stripeBit(it.index) : 0; }
// A part's price is its next tier's.
inline uint16_t price(const Save& s, Item it) {
  if (it.group == PAINT) return PAINT_PRICE[it.index];
  if (it.group == STRIPE) return STRIPE_PRICE[it.index];
  const int t = tier(s, it.index);
  return t < TIERS ? PART_PRICE[t] : 0;
}
inline bool owned(const Save& s, Item it) {   // a look on hand (stock red and no stripe always are); a part fully bought
  if (it.group == PART) return tier(s, it.index) >= TIERS;
  return !it.index || (s.looks & lookBit(it));
}
inline bool worn(const Save& s, Item it) { return it.group == PAINT ? s.paint == it.index : it.group == STRIPE && s.stripe == it.index; }
// The races a part's next tier still waits for (0: open). Looks are open from the start.
inline int unlockAt(const Save& s, Item it) {
  if (it.group != PART || owned(s, it)) return 0;
  const int need = RANK_AT[tier(s, it.index)];
  return s.finishes >= need ? 0 : need;
}
enum class State : uint8_t { Buy, Short, Locked, Full, Equip, Wearing };   // Full: a part with every tier
inline State state(const Save& s, uint32_t stars, Item it) {
  if (it.group == PART && owned(s, it)) return State::Full;
  if (owned(s, it)) return worn(s, it) ? State::Wearing : State::Equip;
  if (unlockAt(s, it)) return State::Locked;
  return stars >= price(s, it) ? State::Buy : State::Short;
}
// Something new the kid can buy right now (the gold dot on the Car button).
inline bool anyToBuy(const Save& s, uint32_t stars) {
  for (int g = 0; g < GROUPS; g++)
    for (int i = 0; i < count(g); i++) if (state(s, stars, {(uint8_t)g, (uint8_t)i}) == State::Buy) return true;
  return false;
}

// This garage visit's purchases, for Undo (memory only: leaving the garage keeps them for good).
struct Visit { uint16_t looks = 0; uint8_t parts[PARTS] = {}; uint8_t paint = 0, stripe = 0; };   // paint, stripe: worn on entry
inline Visit begin(const Save& s) { Visit v; v.paint = s.paint; v.stripe = s.stripe; return v; }
inline void wear(Save& s, Item it) {   // an owned look goes on the car
  if (!owned(s, it)) return;
  if (it.group == PAINT) s.paint = it.index;
  else if (it.group == STRIPE) s.stripe = it.index;
}
// Buys the item (a part's next tier) and puts a look on: false, and nothing taken, unless its state is Buy.
inline bool buy(Save& s, uint32_t* wallet, Item it, Visit& v) {
  const uint16_t cost = price(s, it);
  if (state(s, purse::balance(wallet), it) != State::Buy || !purse::spend(wallet, cost)) return false;
  if (it.group == PART) { setTier(s, it.index, tier(s, it.index) + 1); v.parts[it.index]++; return true; }
  s.looks |= lookBit(it); v.looks |= lookBit(it);
  wear(s, it);
  return true;
}
// Undo: a part's top tier bought this visit, or a look bought this visit while it is on the car.
inline bool canUndo(const Save& s, Item it, const Visit& v) {
  if (it.group == PART) return v.parts[it.index] > 0;
  return (v.looks & lookBit(it)) && worn(s, it);
}
inline bool undo(Save& s, uint32_t* wallet, Item it, Visit& v) {
  if (!canUndo(s, it, v)) return false;
  if (it.group == PART) {
    const int t = tier(s, it.index) - 1;
    setTier(s, it.index, t); v.parts[it.index]--;
    purse::refund(wallet, PART_PRICE[t]);
    return true;
  }
  purse::refund(wallet, price(s, it));
  s.looks &= ~lookBit(it) & 0x7FFu; v.looks &= (uint16_t)~lookBit(it);
  const Item back{it.group, it.group == PAINT ? v.paint : v.stripe};   // what the car wore coming in, if still owned
  if (it.group == PAINT) s.paint = owned(s, back) ? back.index : 0;
  else s.stripe = owned(s, back) ? back.index : 0;
  return true;
}
}  // namespace racing::garage
