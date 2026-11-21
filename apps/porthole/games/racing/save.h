// Rally's save: a kid's best race time and best lap on each track, for the leaderboard every profile shares, (v2)
// the rivals' level for this kid on each track and the kid's difficulty setting, and (v3) the garage: parts, looks,
// races finished and the Stock board's race times. Persisted
// as a raw blob per profile (racing/s<id>, written by the shell). Append-only from this first version: a new field
// goes right before crc (its zero must be a sensible default: loadBlob zero-fills it for older blobs), SAVE_VERSION
// goes up, nothing is reordered or resized. Header-only, like Marble Kick's.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "os/save.h"   // not "save.h": a game may have its own
#include "track.h"
#include "tune.h"

namespace racing {
constexpr uint32_t SAVE_MAGIC = 0x594C4C52;   // "RLLY"
constexpr uint16_t SAVE_VERSION = 3;
constexpr size_t SAVE_V1_SIZE = 44, SAVE_V2_SIZE = 52, SAVE_V3_SIZE = 64;
// by version: a blob is exactly its version's size
constexpr size_t SAVE_SIZES[] = {0, SAVE_V1_SIZE, SAVE_V2_SIZE, SAVE_V3_SIZE};
struct Best { uint32_t raceMs, lapMs; };              // 0 = not driven yet
// The kid's difficulty setting (the chip says Auto for AUTO: adaptive rivals). Persisted: values never change
// meaning; AUTO is 0, the default.
enum Mode : uint8_t { AUTO, EASY, NORMAL, HARD, MODES };
struct Save {
  uint32_t magic;
  uint16_t version, size;
  Best best[TRACKS];   // by track index (TRACK[] order: never reordered)
  int8_t level[TRACKS];   // v2: the adaptive level by track (race.h start; a rung LEVEL_SHIFT higher); 0 from v1
  uint8_t mode;           // v2: the kid's Mode (AUTO from a v1 blob or out of range)
  // v3, the garage (garage.h), in v2's three pad bytes (always zero there) and 12 more: a kid's save has to stay at
  // 64 bytes, four NVS entries (host/test_nvs_budget.cpp ratchet). Zero is stock everywhere, so an older blob loads
  // as a stock car.
  uint8_t finishes;          // races finished, any place, any mode, together too (the rank); stops at 255
  uint16_t stock[TRACKS];    // the best race run with Power 0 (the Stock board), in tenths of a second; 0 none
  uint16_t power;            // the Power each best[] time was set with, four bits a track (bit 4 * track)
  uint16_t tiers : 10;       // each Part's tier 0..TIERS, two bits a part in Part order (bit 2 * part)
  uint16_t paint : 4;        // worn: 0 = stock red; one not owned or out of range loads as 0
  uint16_t stripe : 2;       // worn: 0 = none; likewise
  uint16_t looks : 11;       // owned paints and stripes, a bit each (lookBit below; numbering only appended)
  uint16_t : 4;
  uint16_t stockOn : 1;      // the Stock switch: parts off for racing (looks stay)
  uint32_t crc;
};
// The looks (garage.h names and prices them): paint 0 is stock red and stripe 0 none, both free and always owned.
// Paint p from 1 is owned bit p - 1, stripe s from 1 bit 7 + s. Never renumbered: a new look takes the next bit.
constexpr int PAINTS = 9, STRIPES = 4;
constexpr uint16_t paintBit(int p) { return p > 0 ? (uint16_t)(1u << (p - 1)) : 0; }
constexpr uint16_t stripeBit(int s) { return s > 0 ? (uint16_t)(1u << (7 + s)) : 0; }
static_assert(paintBit(PAINTS - 1) < stripeBit(1) && stripeBit(STRIPES - 1) < 1u << 11, "looks fit Save::looks");
static_assert(PAINTS <= 16 && STRIPES <= 4, "paint and stripe fit their bits");
// 64 bytes keeps a kid's save at 4 NVS entries (host/test_nvs_budget.cpp): 65 would take a fifth.
static_assert(sizeof(Save) == SAVE_V3_SIZE, "Save is persisted: append before crc, never resize");
static_assert(offsetof(Save, mode) == 44 && offsetof(Save, finishes) == 45 && offsetof(Save, stock) == 46 &&
              offsetof(Save, power) == 54 && offsetof(Save, crc) == 60, "the v3 layout");
static_assert(PARTS * 2 <= 10, "two bits a part in Save::tiers");
constexpr uint16_t tenths(uint32_t ms) { return (uint16_t)(ms / 100 < 0xFFFF ? ms / 100 : 0xFFFF); }
static_assert(sizeof SAVE_SIZES / sizeof SAVE_SIZES[0] == SAVE_VERSION + 1, "one size per version");

inline void seal(Save& s) { os::save::seal(&s, sizeof s, SAVE_MAGIC, SAVE_VERSION); }
// Any version up to this one: a blob is its own size with its crc in the last four bytes. Anything else is ignored:
// the kid simply has no times yet, never a half-loaded record.
inline bool loadBlob(const void* data, size_t n, Save& out) {
  os::save::Head h;
  if (n > sizeof(Save) || !os::save::mine(data, n, SAVE_MAGIC, SAVE_VERSION, &h) || n != SAVE_SIZES[h.version]) return false;
  Save s{};
  memcpy(&s, data, n - sizeof s.crc);
  if (s.mode >= MODES) s.mode = AUTO;   // a value from a later firmware: the default
  if (h.version < 3) {   // every time before the garage was run stock
    s.finishes = 0;
    for (int t = 0; t < TRACKS; t++) s.stock[t] = tenths(s.best[t].raceMs);
  }
  if (s.paint >= PAINTS || (s.paint && !(s.looks & paintBit(s.paint)))) s.paint = 0;   // wear only what is owned
  if (s.stripe >= STRIPES || (s.stripe && !(s.looks & stripeBit(s.stripe)))) s.stripe = 0;
  seal(s);
  out = s;
  return true;
}
// Keep the better of each time; says which improved (0 = neither), so a faster lap in a slower race is not called a
// best race.
enum : uint8_t { BEST_RACE = 1, BEST_LAP = 2 };
inline uint8_t record(Best& b, uint32_t raceMs, uint32_t lapMs) {
  uint8_t better = 0;
  if (raceMs && (!b.raceMs || raceMs < b.raceMs)) { b.raceMs = raceMs; better |= BEST_RACE; }
  if (lapMs && (!b.lapMs || lapMs < b.lapMs)) { b.lapMs = lapMs; better |= BEST_LAP; }
  return better;
}
}  // namespace racing
