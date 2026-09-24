#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace pet {

inline constexpr std::uint32_t SaveVersion = 1;
inline constexpr std::uint32_t MaxCount = 1'000'000;
inline constexpr std::uint64_t MaxTimestamp = 253'402'214'400ULL;

enum class Action : std::uint8_t { Feed = 1, Play = 2, Petting = 4, Read = 8, Train = 16, Rest = 32 };
enum class Stage : std::uint8_t { Puppy, YoungPup, StoryDog };
enum class Cue : std::uint8_t { Left, Up, Right, Down, Paw };

// Fixed-width NVS snapshot. Version/checksum/storage ownership belongs to the
// caller; this native-endian structure is not a cross-platform wire format.
struct Pet {
  std::uint64_t createdAt;
  std::uint64_t updatedAt;
  std::uint32_t version;
  std::int32_t lastVisitDay;
  float fullness;
  float happiness;
  float energy;
  std::uint32_t friendship;
  std::uint32_t daysTogether;
  std::array<std::uint32_t, 3> careCounts;  // Feed, Play, Petting.
  std::array<std::uint32_t, 3> discoveries;
  std::uint16_t stickers;
  std::uint8_t stories;
  std::array<std::uint8_t, 6> tricks;
  std::uint8_t dailyCompleted;  // Applies to lastVisitDay, including clock rollback.
  std::uint8_t dailyClaimed;
  std::uint8_t sleeping;
};
static_assert(std::is_trivially_copyable<Pet>::value && std::is_standard_layout<Pet>::value);
static_assert(sizeof(float) == 4 && sizeof(Pet) == 80, "Review the save version if the snapshot layout changes");

struct Lesson {
  std::array<Cue, 6> cues;
  std::uint8_t length;
};

inline constexpr std::uint8_t trickUnlockDay(std::uint8_t id) {
  constexpr std::uint8_t days[] = {1, 1, 2, 3, 5, 7};
  return id < 6 ? days[id] : 255;
}

inline Lesson lesson(std::uint8_t id, std::uint8_t practice) {
  using C = Cue;
  static constexpr Lesson lessons[6][3] = {
    {{{C::Paw, C::Down, C::Paw}, 3}, {{C::Up, C::Paw, C::Down, C::Paw}, 4}, {{C::Up, C::Paw, C::Left, C::Down, C::Paw}, 5}},
    {{{C::Left, C::Paw, C::Right}, 3}, {{C::Left, C::Paw, C::Right, C::Paw}, 4}, {{C::Left, C::Paw, C::Up, C::Right, C::Paw}, 5}},
    {{{C::Left, C::Up, C::Right}, 3}, {{C::Left, C::Up, C::Right, C::Down}, 4}, {{C::Left, C::Up, C::Right, C::Down, C::Left, C::Paw}, 6}},
    {{{C::Up, C::Down, C::Paw}, 3}, {{C::Up, C::Paw, C::Down, C::Paw}, 4}, {{C::Up, C::Left, C::Paw, C::Down, C::Right, C::Paw}, 6}},
    {{{C::Down, C::Up, C::Paw}, 3}, {{C::Down, C::Up, C::Up, C::Paw}, 4}, {{C::Left, C::Down, C::Up, C::Right, C::Up, C::Paw}, 6}},
    {{{C::Left, C::Down, C::Right}, 3}, {{C::Left, C::Down, C::Right, C::Up}, 4}, {{C::Paw, C::Left, C::Down, C::Right, C::Up, C::Paw}, 6}},
  };
  return id < 6 ? lessons[id][std::min<std::uint8_t>(practice, 2)] : Lesson{};
}

inline constexpr std::uint8_t cycle(std::int32_t day, std::uint8_t length) {
  return static_cast<std::uint8_t>((day % length + length) % length);
}

inline constexpr std::uint8_t dailyMask(std::int32_t day) {
  constexpr std::uint8_t masks[] = {
    1 | 8 | 2, 16 | 4 | 32, 8 | 1 | 4, 2 | 16 | 8,
    4 | 1 | 32, 16 | 8 | 32, 2 | 4 | 16,
  };
  return masks[cycle(day, 7)];
}

inline constexpr std::uint8_t dailySticker(std::int32_t day) { return cycle(day, 12); }
inline float clampNeed(float value) { return std::clamp(value, 20.0f, 100.0f); }

inline Pet create(std::uint64_t now, std::int32_t day) {
  Pet p{};
  p.version = SaveVersion;
  p.createdAt = p.updatedAt = now;
  p.lastVisitDay = day;
  p.fullness = 78;
  p.happiness = 86;
  p.energy = 80;
  p.daysTogether = 1;
  return p;
}

inline void tick(Pet& p, std::uint64_t now, std::int32_t day) {
  const auto elapsed = now > p.updatedAt ? std::min<std::uint64_t>(28'800, now - p.updatedAt) : 0;
  const float hours = static_cast<float>(elapsed) / 3600.0f;
  p.updatedAt = std::max(p.updatedAt, now);
  p.fullness = clampNeed(p.fullness - hours * (p.sleeping ? 2 : 4));
  p.happiness = clampNeed(p.happiness - hours * (p.sleeping ? 0 : 2));
  p.energy = clampNeed(p.energy + hours * (p.sleeping ? 30 : -3));
  if (day > p.lastVisitDay) {
    p.daysTogether = std::min(MaxCount, p.daysTogether + 1);
    p.lastVisitDay = day;
    p.dailyCompleted = p.dailyClaimed = 0;
  }
}

namespace detail {
inline void recordActivity(Pet& p, Action action) {
  const auto bit = static_cast<std::uint8_t>(action);
  const bool first = !(p.dailyCompleted & bit);
  p.dailyCompleted |= bit;
  const auto requested = dailyMask(p.lastVisitDay);
  const bool reward = !p.dailyClaimed && (p.dailyCompleted & requested) == requested;
  p.friendship = std::min(MaxCount, p.friendship + (first ? 2U : 0U) + (reward ? 4U : 0U));
  if (reward) {
    p.dailyClaimed = 1;
    p.stickers |= static_cast<std::uint16_t>(1U << dailySticker(p.lastVisitDay));
  }
}
}

inline void act(Pet& p, Action action, std::uint64_t now, std::int32_t day) {
  tick(p, now, day);
  if (p.sleeping) return;
  unsigned index;
  switch (action) {
    case Action::Feed: index = 0; p.fullness = clampNeed(p.fullness + 18); break;
    case Action::Play: index = 1; p.happiness = clampNeed(p.happiness + 14); p.energy = clampNeed(p.energy - 8); break;
    case Action::Petting: index = 2; p.happiness = clampNeed(p.happiness + 6); break;
    default: return;
  }
  p.careCounts[index] = std::min(MaxCount, p.careCounts[index] + 1);
  detail::recordActivity(p, action);
}

inline void toggleSleep(Pet& p, std::uint64_t now, std::int32_t day) {
  tick(p, now, day);
  p.sleeping = !p.sleeping;
  if (p.sleeping) detail::recordActivity(p, Action::Rest);
}

inline void recordReading(Pet& p, std::uint64_t now, std::int32_t day) {
  tick(p, now, day);
  if (!p.sleeping) detail::recordActivity(p, Action::Read);
}

inline std::uint8_t stars(const Pet& p) {
  std::uint8_t count = 0;
  for (std::uint8_t bit = 0; bit < 7; bit++) if (p.stories & (1U << bit)) count++;
  return count * 3;
}

inline void finishStory(Pet& p, std::uint8_t id, std::uint64_t now, std::int32_t day) {
  if (id >= 7) return;
  recordReading(p, now, day);
  if (p.sleeping || (p.stories & (1U << id))) return;
  p.stories |= static_cast<std::uint8_t>(1U << id);
  p.happiness = clampNeed(p.happiness + 10);
}

inline bool discovered(const Pet& p, std::uint8_t id) {
  return id < 96 && (p.discoveries[id / 32] & (std::uint32_t{1} << (id % 32)));
}

inline void discover(Pet& p, std::uint8_t id, std::uint64_t now, std::int32_t day) {
  if (id >= 96) return;
  recordReading(p, now, day);
  if (!p.sleeping) p.discoveries[id / 32] |= std::uint32_t{1} << (id % 32);
}

inline void practice(Pet& p, std::uint8_t id, std::uint64_t now, std::int32_t day) {
  tick(p, now, day);
  if (id >= 6 || p.sleeping || p.daysTogether < trickUnlockDay(id)) return;
  p.tricks[id] = std::min<std::uint8_t>(3, p.tricks[id] + 1);
  detail::recordActivity(p, Action::Train);
}

inline Stage stage(const Pet& p) {
  if (p.daysTogether >= 7 && p.friendship >= 60) return Stage::StoryDog;
  if (p.daysTogether >= 3 && p.friendship >= 20) return Stage::YoungPup;
  return Stage::Puppy;
}

// Call after reading a correctly sized/checksummed snapshot, before using it.
inline bool valid(const Pet& p) {
  const auto need = [](float value) { return std::isfinite(value) && value >= 20 && value <= 100; };
  if (p.version != SaveVersion || p.createdAt > p.updatedAt || p.updatedAt > MaxTimestamp
      || p.lastVisitDay < -719'162 || p.lastVisitDay > 2'932'896
      || !need(p.fullness) || !need(p.happiness) || !need(p.energy)
      || p.friendship > MaxCount || p.daysTogether < 1 || p.daysTogether > MaxCount
      || (p.stories & ~0x7fU) || (p.stickers & ~0xfffU) || (p.dailyCompleted & ~0x3fU)
      || p.dailyClaimed > 1 || p.sleeping > 1) return false;
  for (auto count : p.careCounts) if (count > MaxCount) return false;
  for (auto progress : p.tricks) if (progress > 3) return false;
  const bool complete = (p.dailyCompleted & dailyMask(p.lastVisitDay)) == dailyMask(p.lastVisitDay);
  return complete == static_cast<bool>(p.dailyClaimed)
    && (!complete || (p.stickers & (1U << dailySticker(p.lastVisitDay))));
}

}  // namespace pet
