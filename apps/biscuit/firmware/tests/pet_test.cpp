// c++ -std=c++17 -Wall -Wextra -Werror -Ifirmware/include firmware/tests/pet_test.cpp -o /tmp/pet_test && /tmp/pet_test
#include "pet.h"

#include <cassert>
#include <iostream>
#include <limits>

using pet::Action;
constexpr std::uint64_t Now = 1'814'400'000;
constexpr std::int32_t Day = 21'000;

static void careAndElapsedTime() {
  auto p = pet::create(Now, Day);
  assert(pet::valid(p));
  pet::act(p, Action::Feed, Now, Day);
  assert(p.fullness == 96 && p.careCounts[0] == 1 && p.friendship == 2);
  pet::act(p, Action::Feed, Now, Day);
  assert(p.fullness == 100 && p.careCounts[0] == 2 && p.friendship == 2);

  auto absent = pet::create(Now, Day);
  pet::tick(absent, Now + 30 * 86400, Day + 30);
  assert(absent.fullness == 46 && absent.happiness == 70 && absent.energy == 56);
  assert(absent.daysTogether == 2);  // Count visits, not missed calendar days.
  pet::tick(absent, Now, Day);
  assert(absent.fullness == 46 && absent.updatedAt == Now + 30 * 86400 && absent.daysTogether == 2);
  for (unsigned i = 1; i <= 10; i++) pet::tick(absent, Now + (30 + i) * 86400, Day + 30 + i);
  assert(absent.fullness == 20 && absent.happiness == 20 && absent.energy == 20);
  assert(pet::valid(absent));

  pet::toggleSleep(p, Now, Day);
  const auto friendship = p.friendship;
  pet::act(p, Action::Play, Now, Day);
  pet::finishStory(p, 0, Now, Day);
  pet::discover(p, 0, Now, Day);
  pet::practice(p, 0, Now, Day);
  assert(p.careCounts[1] == 0 && p.stories == 0 && !pet::discovered(p, 0) && p.tricks[0] == 0);
  assert(p.friendship == friendship);
  pet::tick(p, Now + 3600, Day);
  assert(p.fullness == 98 && p.energy == 100 && p.happiness == 86);
  pet::toggleSleep(p, Now + 3600, Day);
  assert(!p.sleeping && pet::valid(p));
}

static void permanentReadingProgress() {
  auto p = pet::create(Now, Day);
  pet::finishStory(p, 0, Now, Day);
  const auto happiness = p.happiness;
  pet::finishStory(p, 0, Now, Day);
  assert(pet::stars(p) == 3 && p.happiness == happiness && p.friendship == 2);
  for (std::uint8_t id = 1; id < 7; id++) pet::finishStory(p, id, Now, Day);
  assert(pet::stars(p) == 21);
  for (std::uint8_t id = 0; id < 96; id++) pet::discover(p, id, Now, Day);
  for (auto bits : p.discoveries) assert(bits == 0xffffffffU);
  pet::discover(p, 95, Now, Day);
  pet::discover(p, 96, Now, Day);
  pet::finishStory(p, 7, Now, Day);
  assert(p.friendship == 2 && pet::stars(p) == 21 && !pet::discovered(p, 96));

  // Copy/check fields rather than assuming a raw byte representation for equality.
  const pet::Pet saved = p;
  pet::tick(p, Now + 86400, Day + 1);
  assert(p.discoveries == saved.discoveries && p.stories == saved.stories && p.dailyCompleted == 0);
  pet::discover(p, 95, Now + 86400, Day + 1);
  assert(p.friendship == 4);  // Re-reading counts toward today's reading, once.
  pet::discover(p, 0, Now + 86400, Day + 1);
  assert(p.friendship == 4 && pet::valid(p));
}

static void adventuresTricksAndGrowth() {
  auto p = pet::create(Now, Day);
  assert(pet::stage(p) == pet::Stage::Puppy);
  pet::practice(p, 2, Now, Day);
  assert(p.tricks[2] == 0 && p.friendship == 0);
  pet::practice(p, 0, Now, Day);
  pet::practice(p, 0, Now, Day);
  pet::practice(p, 0, Now, Day);
  pet::practice(p, 0, Now, Day);
  assert(p.tricks[0] == 3 && p.friendship == 2);

  for (std::int32_t visit = 0; visit < 12; visit++) {
    const auto now = Now + static_cast<std::uint64_t>(visit) * 86400;
    const auto day = Day + visit;
    pet::act(p, Action::Feed, now, day);
    pet::act(p, Action::Play, now, day);
    pet::act(p, Action::Petting, now, day);
    pet::recordReading(p, now, day);
    pet::practice(p, 0, now, day);
    pet::toggleSleep(p, now, day);
    pet::toggleSleep(p, now, day);
    assert(p.dailyCompleted == 63 && p.dailyClaimed == 1);
    assert(p.friendship == static_cast<std::uint32_t>(visit + 1) * 16);
    pet::act(p, Action::Feed, now, day);
    assert(p.friendship == static_cast<std::uint32_t>(visit + 1) * 16);
    assert(pet::valid(p));
    if (visit == 2) assert(pet::stage(p) == pet::Stage::YoungPup);
  }
  assert(p.stickers == 0xfff && p.daysTogether == 12 && pet::stage(p) == pet::Stage::StoryDog);
  for (std::uint8_t id = 0; id < 6; id++) {
    auto locked = pet::create(Now, Day);
    locked.daysTogether = pet::trickUnlockDay(id) - 1;
    if (locked.daysTogether) { pet::practice(locked, id, Now, Day); assert(locked.tricks[id] == 0); }
    locked.daysTogether = pet::trickUnlockDay(id);
    pet::practice(locked, id, Now, Day);
    assert(locked.tricks[id] == 1);
    assert(pet::lesson(id, 0).length == 3 && pet::lesson(id, 1).length == 4 && pet::lesson(id, 2).length >= 5);
    assert(pet::lesson(id, 3).cues == pet::lesson(id, 2).cues);
  }
  assert(pet::lesson(0, 0).cues[0] == pet::Cue::Paw && pet::lesson(6, 0).length == 0);
  assert(pet::dailyMask(-1) == pet::dailyMask(6) && pet::dailySticker(-1) == 11);
}

static void rejectsBrokenSnapshots() {
  const auto good = pet::create(Now, Day);
  auto p = good; p.version++; assert(!pet::valid(p));
  p = good; p.energy = std::numeric_limits<float>::quiet_NaN(); assert(!pet::valid(p));
  p = good; p.tricks[5] = 4; assert(!pet::valid(p));
  p = good; p.updatedAt = Now - 1; assert(!pet::valid(p));
  p = good; p.dailyCompleted = pet::dailyMask(Day); assert(!pet::valid(p));
  p = good; p.sleeping = 2; assert(!pet::valid(p));
}

int main() {
  careAndElapsedTime();
  permanentReadingProgress();
  adventuresTricksAndGrowth();
  rejectsBrokenSnapshots();
  std::cout << "Pet model: care, time, reading, discoveries, daily rewards, tricks, growth and save validation passed.\n";
}
