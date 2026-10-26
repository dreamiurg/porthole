// Rule/save contracts; ordinary-input play and native-size visual review are separate.
#include <assert.h>
#include <initializer_list>
#include <stdio.h>
#include "games/toy-town-life/content.h"
using namespace toytown;
static_assert(MEMORY == 23 && STOP == 24 && BOOK == 25 && MEMORY_EDIT == 26,
  "Persisted V1 page ids must remain stable when new pages are appended");
namespace {
void budget() {
  // Exhaust every editable tray, including an exact budget and every missing lunch component.
  for (int a = 0; a <= 3; ++a) for (int b = 0; b <= 3; ++b)
    for (int c = 0; c <= 3; ++c) for (int d = 0; d <= 3; ++d) {
      State s = fresh(); s.cart[0] = a; s.cart[1] = b; s.cart[2] = c; s.cart[3] = d;
      const int cost = 5 * a + 3 * b + c + 2 * d;
      assert(total(s) == cost);
      assert(canOrder(s) == (cost <= 10 && a + b > 0 && c > 0));
    }
}
void schoolConsequences() {
  const uint8_t expected[2][3] = {{1, 3, 4}, {1, 2, 4}};
  for (int listened = 0; listened <= 1; ++listened) for (int plan = 0; plan < 3; ++plan) {
    State s = fresh(); start(s, SCHOOL); s.listened = listened; finishSchool(s, plan);
    assert(s.page == SCHOOL_RESULT && s.schoolOutcome == expected[listened][plan]);
    assert(SCHOOL_ENDINGS[s.schoolOutcome][0]);
    s.caption = 2; s.sticker = 1; complete(s);
    assert(s.entries[0].place == SCHOOL && s.entries[0].outcome == expected[listened][plan]);
    assert(s.entries[0].caption == 2 && s.entries[0].sticker == 1);
  }
  assert(strstr(SCHOOL_ENDINGS[1], "Theo")); assert(strstr(SCHOOL_ENDINGS[2], "Theo"));
  assert(strstr(SCHOOL_ENDINGS[3], "first words")); assert(strstr(SCHOOL_ENDINGS[4], "waits"));
}
void assertNewOuting(const State& s) {
  assert(s.active && !s.recorded && s.page == PARK_INTRO);
  assert(!s.listened && !s.repaired && !s.revised && !s.clues && !s.schoolOutcome);
  assert(!s.ordered && !s.compared && !s.caption && !s.sticker);
  assert(!total(s) && !memcmp(s.cart, s.compare, 4));
}
void replayAndMemories() {
  State s = fresh(); s.doll = 3; s.hair = 2; s.bag = 21;
  for (int n = 0; n < 20; ++n) {
    s.listened = s.repaired = s.revised = s.clues = s.recorded = 1;
    s.schoolOutcome = s.ordered = s.compared = s.caption = s.sticker = 1;
    memset(s.cart, 3, 4); memset(s.compare, 3, 4); start(s, PARK);
    assertNewOuting(s);
    assert(s.doll == 3 && s.hair == 2 && s.bag == 21);
    s.caption = n % 3; s.sticker = (n + 1) % 3; complete(s);
    assert(s.count == (n < 8 ? n + 1 : 8) && s.recorded && !s.active);
    assert(s.completed == (1 << PARK));
    const State once = s; complete(s); assert(!memcmp(&once, &s, sizeof s));
    const int first = n < 8 ? 0 : n - 7;
    for (int j = 0; j < s.count; ++j) {
      assert(s.entries[j].place == PARK && s.entries[j].caption == (first + j) % 3);
      assert(s.entries[j].sticker == (first + j + 1) % 3);
    }
  }
  start(s, SCHOOL); finishSchool(s, 2); complete(s);
  start(s, CAFE); s.cart[1] = 1; s.cart[2] = 1; complete(s);
  assert(s.entries[7].outcome == 6 && s.completed == 14);
}
void coherentSeeds() {
  State a = fresh(), b = fresh(); bool seen[3]{};
  const char* places[] = {"bench", "fountain", "path"};
  for (int n = 0; n < 1000; ++n) {
    start(a, PARK); start(b, PARK);
    assert(a.seed == b.seed && a.variant == b.variant && a.variant < 3); seen[a.variant] = true;
    assert(strstr(LAST_SEEN[a.variant], places[a.variant]));
    assert(strstr(CLUES[a.variant], places[a.variant]));
    assert(strstr(CLUES[a.variant], "pink ribbon"));
    assert(strstr(EMPTY_CLUES[a.variant], "No pink ribbon"));
  }
  assert(seen[0] && seen[1] && seen[2]);
  for (int bits = 0; bits < 32; ++bits) {
    a.bag = bits; int count = 0;
    for (int i = 0; i < 5; ++i) if (bits & (1 << i)) ++count;
    assert(slots(a) == count);
  }
}
void rejected(const State& candidate, size_t bytes = sizeof(State)) {
  State out = fresh(); out.doll = 2; const State before = out;
  assert(!load(&candidate, bytes, out));
  assert(!memcmp(&out, &before, sizeof out)); // rejection never partially replaces progress
}
void invalidRanges() {
  struct Range { uint8_t State::*member; uint8_t invalid; };
  const Range ranges[] = {
    {&State::page, PAGE_COUNT}, {&State::doll, 4}, {&State::hair, 3}, {&State::skin, 4},
    {&State::outfit, 3}, {&State::shoes, 3}, {&State::accessory, 3}, {&State::color, 4},
    {&State::dollName, 4}, {&State::category, 8}, {&State::destination, 4}, {&State::bag, 32},
    {&State::packItem, 5}, {&State::menuItem, 4}, {&State::count, 9}, {&State::variant, 3},
    {&State::clue, 3}, {&State::guess, 3}, {&State::resumePage, PAGE_COUNT}, {&State::bookIndex, 8},
    {&State::caption, 3}, {&State::sticker, 3}, {&State::schoolOutcome, 5},
    {&State::listened, 2}, {&State::repaired, 3}, {&State::compared, 2}, {&State::ordered, 2},
    {&State::revised, 2}, {&State::active, 2}, {&State::recorded, 2}, {&State::clues, 8},
    {&State::completed, 1}
  };
  for (const auto& range : ranges) { State s = fresh(); s.*range.member = range.invalid; seal(s); rejected(s); }
  for (int i = 0; i < 4; ++i) {
    State s = fresh(); s.cart[i] = 4; seal(s); rejected(s);
    s = fresh(); s.compare[i] = 4; seal(s); rejected(s);
  }
  State full = fresh(); full.bag = 31; seal(full); rejected(full);
  for (int field = 0; field < 4; ++field) {
    State s = fresh(); s.count = 1; s.entries[0] = {SCHOOL, 1, 0, 0};
    if (field == 0) s.entries[0].place = HOME;
    if (field == 1) s.entries[0].place = PARK + 1;
    if (field == 2) s.entries[0].caption = 3;
    if (field == 3) s.entries[0].sticker = 3;
    seal(s); rejected(s);
  }
  for (Entry entry : {Entry{SCHOOL,0,0,0},Entry{SCHOOL,5,0,0},Entry{CAFE,7,0,0},Entry{PARK,3,0,0}}) {
    State s = fresh(); s.count = 1; s.entries[0] = entry; seal(s); rejected(s);
  }
}
void richRoundtrip() {
  State s = fresh(), out{};
  s.page = BOOK; s.resumePage = MEMORY; s.doll = 3; s.skin = 3; s.hair = 2; s.category = 7;
  s.outfit = s.shoes = s.accessory = 2; s.color = s.dollName = 3; s.bag = 21;
  s.listened = s.compared = s.ordered = s.revised = s.recorded = 1; s.repaired = 2;
  s.clues = 7; s.completed = 14; s.count = 8; s.bookIndex = 7; s.seed = 0xffffffffu;
  memset(s.cart,3,4); memset(s.compare,3,4);
  for (int i = 0; i < 8; ++i) s.entries[i] = {CAFE,6,2,2};
  seal(s); assert(load(&s,sizeof s,out) && !memcmp(&s,&out,sizeof s));
}
void saves() {
  State s = fresh(), out{}; seal(s);
  assert(load(&s, sizeof s, out) && !memcmp(&s, &out, sizeof s));
  assert(!load(nullptr, sizeof s, out));
  rejected(s, 0); rejected(s, sizeof s - 1); rejected(s, sizeof s + 1);
  s.magic ^= 1; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
  s.version = 0; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
  s.version = VERSION + 1; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
  s.size -= 1; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
  s.crc ^= 1; rejected(s); seal(s);
  for (size_t i = 0; i < sizeof s; ++i) { State bad = s; reinterpret_cast<uint8_t*>(&bad)[i] ^= 1; rejected(bad); }
  for (uint8_t page = HUB; page < PAGE_COUNT; ++page) {
    s.page = page; s.resumePage = page; s.seed = 0xfedcba98; seal(s);
    assert(load(&s, sizeof s, out) && !memcmp(&s, &out, sizeof s));
  }
  invalidRanges();
}
} // namespace
int main() {
  budget(); schoolConsequences(); replayAndMemories(); coherentSeeds(); saves(); richRoundtrip();
  puts("Toy Town rules/save contracts passed");
}
