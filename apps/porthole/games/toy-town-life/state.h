#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "crc32.h"

namespace toytown {
// Persisted ids: append only. No clock-driven state or reward economy.
enum Place : uint8_t { HOME, SCHOOL, CAFE, PARK };
enum Page : uint8_t {
  HUB, DOLL, STYLE, MAP, PACK, SCHOOL_INTRO, SCHOOL_LISTEN, SCHOOL_PLAN,
  SCHOOL_OTHER, SCHOOL_PAPER, SCHOOL_RESULT, SCHOOL_REPAIR, CAFE_INTRO,
  CAFE_MENU, CAFE_CART, CAFE_COMPARE, CAFE_RESULT, PARK_INTRO, PARK_SEARCH,
  PARK_CLUE, PARK_GUESS, PARK_WRONG, PARK_FOUND, MEMORY, STOP, BOOK, MEMORY_EDIT, PAGE_COUNT
};
struct Entry { uint8_t place, outcome, caption, sticker; };
struct State {
  uint32_t magic;
  uint16_t version, size;
  uint8_t page, doll, hair, skin, outfit, shoes, accessory, color;
  uint8_t dollName, category, destination, bag, packItem, listened, schoolOutcome, repaired;
  uint8_t cart[4], compare[4];
  uint8_t menuItem, compared, ordered, variant, clues, clue, guess, revised;
  uint8_t active, recorded, count, bookIndex, completed, caption, sticker, resumePage;
  Entry entries[8];
  uint32_t seed, crc;
};
constexpr uint32_t MAGIC = 0x544f5954;
constexpr uint16_t VERSION = 1;
constexpr size_t V1_SIZE = 88;
static_assert(sizeof(State) == V1_SIZE, "Save: append fields before crc, bump version and keep historical sizes");
inline State fresh() { State s{}; s.seed = 17; return s; }
inline int total(const State& s) {
  return s.cart[0] * 5 + s.cart[1] * 3 + s.cart[2] + s.cart[3] * 2;
}
inline bool canOrder(const State& s) { return total(s) <= 10 && (s.cart[0] || s.cart[1]) && s.cart[2]; }
inline int slots(const State& s) { int n = 0; for (int i = 0; i < 5; ++i) n += (s.bag >> i) & 1; return n; }
inline void start(State& s, uint8_t place) {
  s.destination = place; s.active = 1; s.recorded = 0; s.listened = s.repaired = s.revised = s.clues = 0;
  s.schoolOutcome = s.ordered = s.compared = s.caption = s.sticker = 0;
  memset(s.cart, 0, 4); memset(s.compare, 0, 4);
  s.seed = s.seed * 1664525u + 1013904223u; s.variant = (s.seed >> 16) % 3;
  s.page = place == SCHOOL ? SCHOOL_INTRO : place == CAFE ? CAFE_INTRO : PARK_INTRO;
}
inline void finishSchool(State& s, uint8_t plan) {
  s.schoolOutcome = plan == 1 ? (s.listened ? 2 : 3) : plan == 2 ? 4 : 1;
  s.page = SCHOOL_RESULT;
}
inline void complete(State& s) {
  if (s.recorded) return;
  if (s.count == 8) { memmove(s.entries, s.entries + 1, 7 * sizeof(Entry)); --s.count; }
  uint8_t outcome = s.destination == SCHOOL ? s.schoolOutcome : s.destination == CAFE ? (uint8_t)(10 - total(s)) : s.variant;
  s.entries[s.count++] = {s.destination, outcome, s.caption, s.sticker};
  s.recorded = 1; s.active = 0; s.completed |= 1 << s.destination;
}
inline void seal(State& s) {
  s.magic = MAGIC; s.version = VERSION; s.size = sizeof s;
  s.crc = os::crc32(&s, sizeof s - 4);
}
inline bool validLook(const State& s) {
  return s.doll <= 3 && s.category <= 7 && s.skin <= 3 && s.hair <= 2 && s.outfit <= 2 &&
      s.shoes <= 2 && s.accessory <= 2 && s.color <= 3 && s.dollName <= 3;
}
inline bool validCursor(const State& s) {
  return s.page < PAGE_COUNT && s.destination <= PARK && s.bag <= 31 && s.packItem <= 4 &&
      s.menuItem <= 3 && s.count <= 8 && s.variant <= 2 && s.clue <= 2 && s.guess <= 2 &&
      s.resumePage < PAGE_COUNT && s.bookIndex <= 7 && s.caption <= 2 && s.sticker <= 2;
}
inline bool validFlags(const State& s) {
  if (slots(s) > 3 || s.listened > 1 || s.repaired > 2 || s.compared > 1 || s.ordered > 1 ||
      s.revised > 1 || s.active > 1 || s.recorded > 1 || s.clues > 7 || (s.completed & ~14u)) return false;
  return s.schoolOutcome <= 4;
}
inline bool validEntry(const Entry& e) {
  if (e.place < SCHOOL || e.place > PARK || e.caption > 2 || e.sticker > 2) return false;
  if (e.place == SCHOOL) return e.outcome >= 1 && e.outcome <= 4;
  return e.outcome <= (e.place == CAFE ? 6 : 2);
}
inline bool valid(const State& s) {
  if (!validLook(s) || !validCursor(s) || !validFlags(s)) return false;
  for (int i = 0; i < 4; ++i) if (s.cart[i] > 3 || s.compare[i] > 3) return false;
  for (int i = 0; i < s.count; ++i) if (!validEntry(s.entries[i])) return false;
  return true;
}
inline bool load(const void* data, size_t n, State& out) {
  if (!data || n != V1_SIZE) return false;
  State s{}; memcpy(&s, data, n);
  if (s.magic != MAGIC || s.version != VERSION || s.size != n || s.crc != os::crc32(&s, n - 4) || !valid(s)) return false;
  out = s; return true;
}
} // namespace toytown
