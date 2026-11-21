// The skills engine (os/skills.h): ratings, calibration, pay by difficulty, repeats, the day's star rules, and the cap
// on stars from games that teach nothing. The numbers are the approved design's (the "Porthole Star Economy" page).
#include "host/check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "skills.h"

static int checks = 0;
static void setTheta(skills::Book& b, skills::Skill k, int theta, int peak) {
  skills::State s = skills::get(b, k); s.theta = theta; s.peak = peak; skills::put(b, k, s);
}
static constexpr uint32_t DAY = 86400, NOW = 20000 * DAY + 10 * 3600;   // a 10:00, far from day 0

static void curve() {
  CHECK(skills::easeFor(3) >= 83 && skills::easeFor(3) <= 87);    // 3 answers: ~85 below theta is 80% right
  CHECK(skills::easeFor(4) >= 99 && skills::easeFor(4) <= 103);   // 4 answers: ~101
  CHECK(skills::expect(300, 300, 3) > 0.66f && skills::expect(300, 300, 3) < 0.67f);   // even: guess + half the rest
  CHECK(skills::expect(1000, 0, 3) > 0.99f && skills::expect(0, 1000, 3) < 0.34f);
}

// A new skill starts easy and jumps: right answers move it 1, 2, 4 levels; a miss steps back half and halves the jump,
// until the jump is one level and two answers were wrong. It pays a flat rate meanwhile and is not a milestone.
static void calibration() {
  skills::Book b{}; skills::forgetRepeats();
  skills::seed(b, skills::ADD_SUB, 0, false);
  CHECK(skills::calibrating(b, skills::ADD_SUB) && skills::target(b, skills::ADD_SUB, 10, 3) == 0);
  skills::Round r = skills::start(b, NOW);
  uint16_t key = 0;
  auto ans = [&](bool right) { skills::answer(b, r, {skills::ADD_SUB, skills::target(b, skills::ADD_SUB, 10, 3) * 100, 3, 1000, key++}, right, 5000); };
  ans(true); CHECK(skills::get(b, skills::ADD_SUB).theta == 100);
  ans(true); CHECK(skills::get(b, skills::ADD_SUB).theta == 300);
  ans(true); CHECK(skills::get(b, skills::ADD_SUB).theta == 700);
  CHECK(r.points == 3 * 40);   // a flat base each while calibrating, whatever the level
  ans(false); CHECK(skills::get(b, skills::ADD_SUB).theta == 500);   // back half of 4, the jump now 2
  ans(true); CHECK(skills::get(b, skills::ADD_SUB).theta == 700);    // +2, the jump 4 again... capped by the next miss
  ans(false); ans(false);
  for (int i = 0; i < 20 && skills::calibrating(b, skills::ADD_SUB); i++) ans(i % 2);
  CHECK(!skills::calibrating(b, skills::ADD_SUB) );
  CHECK(!r.newBest);   // where calibration lands is a start
}

// What is easy for this kid pays a quarter, what is hard up to 2.5 times; the same item again the same day half, a
// quarter, then nothing.
static void payByDifficulty() {
  skills::Book b{}; skills::forgetRepeats();
  skills::seed(b, skills::TIMES_STORY, 5, true);
  skills::Round r = skills::start(b, NOW);
  skills::answer(b, r, {skills::TIMES_STORY, 0, 3, 1000, 1}, true, 5000);   // far below
  CHECK(r.points == 10);
  setTheta(b, skills::TIMES_STORY, 500, 500);
  r = skills::Round{};
  skills::answer(b, r, {skills::TIMES_STORY, 900, 3, 1000, 2}, true, 5000);   // far above
  CHECK(r.points == 100);
  setTheta(b, skills::TIMES_STORY, 500, 500);
  r = skills::Round{};
  const int at = 500 - skills::easeFor(3);   // the target: about the base
  skills::answer(b, r, {skills::TIMES_STORY, at, 3, 1000, 3}, true, 5000);
  CHECK(r.points >= 38 && r.points <= 42);
  // repeats of key 1
  setTheta(b, skills::TIMES_STORY, 500, 500);
  r = skills::Round{}; skills::answer(b, r, {skills::TIMES_STORY, 0, 3, 1000, 1}, true, 5000); CHECK(r.points == 5);
  r = skills::Round{}; skills::answer(b, r, {skills::TIMES_STORY, 0, 3, 1000, 1}, true, 5000); CHECK(r.points == 2);
  r = skills::Round{}; skills::answer(b, r, {skills::TIMES_STORY, 0, 3, 1000, 1}, true, 5000); CHECK(r.points == 0);
  // a new day: full pay again
  r = skills::start(b, NOW + DAY); skills::answer(b, r, {skills::TIMES_STORY, 0, 3, 1000, 1}, true, 5000); CHECK(r.points == 10);
}

// Missing on purpose lowers the rating, but pay reads it no lower than the peak - 150; a guess faster than reading
// pays nothing, a fast wrong one costs half a base in the round, and the round never goes below 0.
static void noFarming() {
  skills::Book b{}; skills::forgetRepeats();
  skills::seed(b, skills::SPELL_EN, 6, true);
  skills::Round r = skills::start(b, NOW);
  for (int i = 0; i < 30; i++) skills::answer(b, r, {skills::SPELL_EN, 0, 3, 1000, (uint16_t)(100 + i)}, false, 5000);
  CHECK(skills::get(b, skills::SPELL_EN).theta < 300 && skills::get(b, skills::SPELL_EN).peak == 600 && r.points == 0);
  r = skills::Round{};
  skills::answer(b, r, {skills::SPELL_EN, 200, 3, 1000, 1}, true, 5000);   // easy against 450, not the sunk rating
  CHECK(r.points == 10);
  r = skills::Round{};
  skills::answer(b, r, {skills::SPELL_EN, 900, 3, 1000, 2}, true, 300);    // faster than anyone reads: a guess
  CHECK(r.points == 0);
  r.points = 50;
  skills::answer(b, r, {skills::SPELL_EN, 900, 3, 1000, 3}, false, 100);   // under half the usual time, wrong
  CHECK(r.points == 30);
  skills::answer(b, r, {skills::SPELL_EN, 900, 3, 1000, 4}, false, 100);
  skills::answer(b, r, {skills::SPELL_EN, 900, 3, 1000, 5}, false, 100);
  CHECK(r.points == 0);
  uint32_t wallet = 9; CHECK(skills::finish(b, r, NOW, &wallet) == 0 && wallet == 9);
}

// A round's points become stars at its end: 100 a star with the rest carried; the day's first 6 count twice; past 40 a
// day a quarter; a new best level +5.
static void dayRules() {
  skills::Book b{}; uint32_t wallet = 0;
  skills::Round r = skills::start(b, NOW); r.points = 250;
  CHECK(skills::finish(b, r, NOW, &wallet) == 4 && r.stars == 2 && r.bonus == 2 && b.carry == 50 && wallet == 4);
  r = skills::Round{}; r.points = 450;   // 500 with the carry: 5 stars, 4 of them still fresh
  CHECK(skills::finish(b, r, NOW, &wallet) == 9 && r.bonus == 4 && b.carry == 0 && b.learnStars == 7);
  r = skills::Round{}; r.points = 3300 + 400;   // 33 to reach 40, then 400 count as 100
  skills::finish(b, r, NOW, &wallet);
  CHECK(r.resting && r.stars == 34 && b.learnStars == 41 && r.bonus == 0);
  r = skills::Round{}; r.points = 100; r.newBest = true;
  CHECK(skills::finish(b, r, NOW, &wallet) == 5 && r.milestone == 5);   // a quarter star, carried, and the milestone
  r = skills::start(b, NOW + DAY); r.points = 100;   // a new day: fresh again
  CHECK(skills::finish(b, r, NOW + DAY, &wallet) == 2 && b.learnStars == 1);
}

// Games that teach nothing: 10 stars a day across all of them; without a book (a test), all.
static void play() {
  skills::Book b{}; uint32_t wallet = 0;
  CHECK(skills::earnForPlay(&b, &wallet, NOW, 6) == 6);
  CHECK(skills::earnForPlay(&b, &wallet, NOW, 6) == 4 && wallet == 10);
  CHECK(skills::earnForPlay(&b, &wallet, NOW, 1) == 0 && wallet == 10);
  CHECK(skills::earnForPlay(&b, &wallet, NOW + DAY, 3) == 3 && wallet == 13);
  CHECK(skills::earnForPlay(nullptr, &wallet, NOW, 20) == 20 && wallet == 33);
  CHECK(stars::spend(&wallet, 30) && wallet == 3 && !stars::spend(&wallet, 4));
}

// Peaks cool 10 a day, never below the rating; the weekly pace moves toward 1.7 stars a minute within 0.8..1.5.
static void dayAndWeek() {
  skills::Book b{};
  skills::seed(b, skills::READ_EN, 3, true);
  setTheta(b, skills::READ_EN, 200, 300);
  skills::today(b, NOW); const int p0 = skills::get(b, skills::READ_EN).peak;
  skills::today(b, NOW + 3 * DAY); CHECK(abs(skills::get(b, skills::READ_EN).peak - (p0 - 30)) < 8);   // stored in eighths
  skills::today(b, NOW + 40 * DAY); CHECK(skills::get(b, skills::READ_EN).peak == 200);
  CHECK(b.tempo == 100);
  b.weekStars = 100; b.weekSec = 600;   // 10 a minute: far too many, the pace goes down, not below 0.8
  skills::today(b, NOW + 50 * DAY); CHECK(b.tempo >= 80 && b.tempo < 100 && b.weekSec == 0);
  b.weekStars = 0; b.weekSec = 1200;    // none: up, not past 1.5
  skills::today(b, NOW + 60 * DAY); CHECK(b.tempo > 80 && b.tempo <= 150);
}

// A kid whose true level is 5 answers what the engine serves; the rating settles near it and stays.
static void converges() {
  skills::Book b{}; skills::forgetRepeats();
  skills::seed(b, skills::WORDS_UK, 0, false);
  uint32_t rng = 12345; skills::Round r = skills::start(b, NOW);
  for (int i = 0; i < 120; i++) {
    const int beta = skills::target(b, skills::WORDS_UK, 12, 3) * 100;
    rng = rng * 1103515245u + 12345u;
    const bool right = (rng >> 8) % 1000 < (uint32_t)(skills::expect(500, beta, 3) * 1000);
    skills::answer(b, r, {skills::WORDS_UK, beta, 3, 1000, (uint16_t)i}, right, 4000);
  }
  CHECK(skills::get(b, skills::WORDS_UK).theta > 350 && skills::get(b, skills::WORDS_UK).theta < 650);
}

int main() {
  curve(); calibration(); payByDifficulty(); noFarming(); dayRules(); play(); dayAndWeek(); converges();
  printf("test_skills: %d checks\n", checks);
  return 0;
}
