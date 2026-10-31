// Rule/save contracts; ordinary-input play and native-size visual review are separate.
#include <assert.h>
#include <initializer_list>
#include <stdio.h>
#include "games/toy-town-life/content.h"
using namespace toytown;
static_assert(CAFE_QUANTITY == 27 && SCHOOL_HANG == 28, "Append V3 pages");
static_assert(MEMORY == 23 && STOP == 24 && BOOK == 25 && MEMORY_EDIT == 26,
  "Persisted V1 page ids must remain stable when new pages are appended");
namespace {
// Frozen bytes captured from the shipped V1 State/seal implementation, not today's struct.
// Appearance, an in-progress Park memory, two trays and eight mixed memories; CRC 0xcd77f611.
constexpr uint8_t V1_FIXTURE[] = {
  0x54,0x59,0x4f,0x54,0x01,0x00,0x58,0x00,0x1a,0x03,0x02,0x01,
  0x01,0x02,0x01,0x03,0x02,0x07,0x03,0x15,0x04,0x01,0x04,0x02,
  0x01,0x02,0x03,0x00,0x00,0x03,0x02,0x01,0x03,0x01,0x01,0x02,
  0x07,0x01,0x02,0x01,0x01,0x00,0x08,0x06,0x0e,0x02,0x01,0x17,
  0x01,0x01,0x00,0x01,0x02,0x06,0x01,0x02,0x03,0x02,0x02,0x00,
  0x01,0x04,0x00,0x02,0x02,0x00,0x02,0x01,0x03,0x01,0x01,0x00,
  0x01,0x03,0x02,0x02,0x02,0x04,0x01,0x01,0x98,0xba,0xdc,0xfe,
  0x11,0xf6,0x77,0xcd
};
static_assert(sizeof V1_FIXTURE == V1_SIZE && V1_SIZE == 88, "Keep the shipped V1 fixture frozen");
// Frozen shipped V2 layout with independent hair color 3.
constexpr uint8_t V2_FIXTURE[] = {
  0x54,0x59,0x4f,0x54,0x02,0x00,0x5c,0x00,0x1a,0x03,0x02,0x01,
  0x01,0x02,0x01,0x03,0x02,0x07,0x03,0x15,0x04,0x01,0x04,0x02,
  0x01,0x02,0x03,0x00,0x00,0x03,0x02,0x01,0x03,0x01,0x01,0x02,
  0x07,0x01,0x02,0x01,0x01,0x00,0x08,0x06,0x0e,0x02,0x01,0x17,
  0x01,0x01,0x00,0x01,0x02,0x06,0x01,0x02,0x03,0x02,0x02,0x00,
  0x01,0x04,0x00,0x02,0x02,0x00,0x02,0x01,0x03,0x01,0x01,0x00,
  0x01,0x03,0x02,0x02,0x02,0x04,0x01,0x01,0x98,0xba,0xdc,0xfe,
  0x03,0x00,0x00,0x00,0x91,0x76,0xd7,0x92,
};
void sceneRules() {
  State s = fresh();
  assert(s.worldX == 80 && s.worldY == 80 && s.worldPlace == HOME && !hasPaper(s));
  s.bag = 14; start(s, SCHOOL); const State before = s;
  borrowPaper(s);
  assert(hasPaper(s) && s.schoolPaper && s.bag == before.bag && slots(s) == 3);
  assert(s.page == before.page && !s.schoolOutcome && s.active && !s.recorded);
  borrowPaper(s); assert(s.schoolPaper == 1);
  start(s, CAFE); assert(!s.schoolPaper && s.worldPlace == CAFE);
  s.bag = 1; assert(hasPaper(s));
}
void displayRules() {
  for (uint8_t first : {1,2}) {
    State s = fresh(); repairSchool(s, first); repairSchool(s, 3 - first);
    assert(s.repaired == 3); repairSchool(s, first); assert(s.repaired == 3);
    start(s, SCHOOL); assert(!s.repaired); beginDisplay(s, false);
    assert(s.page == SCHOOL_HANG && s.schoolTask == 1 && !displayReady(s));
    assert(!finishDisplay(s) && !s.schoolOutcome);
    for (int slot = 0; slot < 3; ++slot) { s.schoolPicture = slot; assert(hangPicture(s, slot)); }
    assert(displayReady(s) && s.page == SCHOOL_HANG && !s.schoolOutcome);
    s.schoolPicture = 2; assert(hangPicture(s, 0)); assert((s.schoolPictures & 3) == 2);
    assert(finishDisplay(s) && s.schoolOutcome == 4 && s.page == SCHOOL_RESULT);
    repairSchool(s, 2); beginDisplay(s, true); assert(s.repaired == 2);
    for (int slot = 0; slot < 3; ++slot) assert(hangPicture(s, slot));
    assert(finishDisplay(s) && s.repaired == 3 && s.page == SCHOOL_REPAIR);
    assert(finishDisplay(s) && s.repaired == 3);
  }
}
void displayReopen() {
  State s = fresh(), out{}; start(s, SCHOOL); beginDisplay(s, false);
  for (int slot = 0; slot < 3; ++slot) { s.schoolPicture = slot; assert(hangPicture(s, slot)); }
  assert(finishDisplay(s)); const uint8_t pictures = s.schoolPictures;
  beginDisplay(s, true);
  assert(s.schoolHangMask == 7 && s.schoolPictures == pictures && s.schoolPicture == 2);
  s.schoolPicture = 0; assert(hangPicture(s, 1));
  assert((s.schoolPictures & 3) == 0 && ((s.schoolPictures >> 2) & 3) == 0);
  assert(((s.schoolPictures >> 4) & 3) == 2);
  seal(s); assert(load(&s, sizeof s, out)); beginDisplay(out, true);
  assert(out.schoolHangMask == 7 && out.schoolPictures == s.schoolPictures && out.schoolPicture == 0);
  start(out, SCHOOL);
  assert(!out.schoolHangMask && !out.schoolPictures && !out.schoolPicture && !out.schoolTask);
}
void displayResume() {
  State s = fresh(), out{}; start(s, SCHOOL); beginDisplay(s, true);
  s.schoolPicture = 1; assert(hangPicture(s, 2)); seal(s);
  assert(load(&s, sizeof s, out) && !memcmp(&s, &out, sizeof s));
  const State before = s;
  assert(!hangPicture(s, -1) && !memcmp(&s, &before, sizeof s));
  assert(!hangPicture(s, 3) && !memcmp(&s, &before, sizeof s));
  s.schoolPicture = 3; const State bad = s;
  assert(!hangPicture(s, 0) && !memcmp(&s, &bad, sizeof s));
  start(s, PARK); assert(!s.schoolHangMask && !s.schoolPicture && !s.schoolPictures && !s.schoolTask);
}
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
  State s = fresh(); s.doll = 3; s.hair = 2; s.hairColor = 3; s.bag = 21;
  for (int n = 0; n < 20; ++n) {
    s.listened = s.repaired = s.revised = s.clues = s.recorded = 1;
    s.schoolOutcome = s.ordered = s.compared = s.caption = s.sticker = 1;
    memset(s.cart, 3, 4); memset(s.compare, 3, 4); start(s, PARK);
    assertNewOuting(s);
    assert(s.doll == 3 && s.hair == 2 && s.hairColor == 3 && s.bag == 21);
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
void rejectedBlob(const void* candidate, size_t bytes) {
  State out = fresh(); out.doll = 2; const State before = out;
  assert(!load(candidate, bytes, out));
  assert(!memcmp(&out, &before, sizeof out)); // rejection never partially replaces progress
}
void rejected(const State& candidate, size_t bytes = sizeof(State)) { rejectedBlob(&candidate, bytes); }
void assertV1Progress(const State& s) {
  assert(s.page == MEMORY_EDIT); assert(s.resumePage == MEMORY); assert(s.destination == PARK);
  assert(s.doll == 3); assert(s.hair == 2); assert(s.skin == 1); assert(s.outfit == 1); assert(s.shoes == 2);
  assert(s.accessory == 1); assert(s.color == 3); assert(s.dollName == 2); assert(s.category == 7);
  assert(s.bag == 21); assert(s.packItem == 4); assert(s.listened == 1); assert(s.schoolOutcome == 4); assert(s.repaired == 2);
  const uint8_t cart[] = {1,2,3,0}, compare[] = {0,3,2,1};
  assert(!memcmp(s.cart, cart, sizeof cart) && !memcmp(s.compare, compare, sizeof compare));
  assert(s.menuItem == 3); assert(s.compared == 1); assert(s.ordered == 1); assert(s.variant == 2);
  assert(s.clues == 7); assert(s.clue == 1); assert(s.guess == 2); assert(s.revised == 1);
  assert(s.active == 1); assert(s.recorded == 0); assert(s.count == 8); assert(s.bookIndex == 6);
  assert(s.completed == 14); assert(s.caption == 2); assert(s.sticker == 1); assert(s.seed == 0xfedcba98u);
  const Entry entries[] = {{SCHOOL,1,0,1},{CAFE,6,1,2},{PARK,2,2,0},{SCHOOL,4,0,2},
    {CAFE,0,2,1},{PARK,1,1,0},{SCHOOL,3,2,2},{CAFE,4,1,1}};
  assert(!memcmp(s.entries, entries, sizeof entries));
}
void assertDisplayDefaults(const State& s) {
  assert(!s.schoolHangMask && !s.schoolPicture && !s.schoolPictures && !s.schoolTask);
}
void frozenV1Migration() {
  static_assert(V2_SIZE == 92 && sizeof(State) == V3_SIZE, "V3 retains historical V2 size constant");
  static_assert(offsetof(State, seed) == 80 && offsetof(State, hairColor) == 84 && offsetof(State, worldX) == 88 && offsetof(State, schoolHangMask) == 92 && offsetof(State, crc) == 96,
    "Every V1 field through seed keeps its byte offset");
  assert(os::crc32(V1_FIXTURE, V1_SIZE - 4) == 0xcd77f611u);
  State migrated = fresh(), out{};
  assert(load(V1_FIXTURE, sizeof V1_FIXTURE, migrated));
  assertV1Progress(migrated);
  assertDisplayDefaults(migrated);
  // Compare every old player field, array byte and seed against the original record.
  assert(!memcmp(reinterpret_cast<const uint8_t*>(&migrated) + 8, V1_FIXTURE + 8, V1_SIZE - 12));
  assert(migrated.magic == MAGIC && migrated.version == VERSION && migrated.size == V3_SIZE);
  assert(migrated.hairColor == 0 && !migrated.reserved[0] && !migrated.reserved[1] && !migrated.reserved[2]);
  seal(migrated); assert(load(&migrated, sizeof migrated, out));
  assert(!memcmp(&migrated, &out, sizeof out));
  assertV1Progress(out);
  for (uint8_t hair = 0; hair < 3; ++hair) for (uint8_t color = 0; color < 4; ++color) {
    migrated.hair = hair; migrated.hairColor = color; migrated.category = 8; seal(migrated);
    assert(load(&migrated, sizeof migrated, out) && !memcmp(&migrated, &out, sizeof out));
  }
}
void invalidV1Saves() {
  rejectedBlob(nullptr, V1_SIZE); rejectedBlob(V1_FIXTURE, V1_SIZE - 1);
  uint8_t bytes[V1_SIZE + 1]{};
  memcpy(bytes, V1_FIXTURE, V1_SIZE); rejectedBlob(bytes, sizeof bytes);
  for (size_t i = 0; i < V1_SIZE; ++i) {
    memcpy(bytes, V1_FIXTURE, V1_SIZE); bytes[i] ^= 1; rejectedBlob(bytes, V1_SIZE);
  }
  // Recompute CRC so these prove header/range rejection independently of checksum rejection.
  for (size_t offset : {0u,4u,5u,6u,7u,17u}) {
    memcpy(bytes, V1_FIXTURE, V1_SIZE);
    bytes[offset] = offset == 17 ? 8 : bytes[offset] ^ 1; // category 8 belongs only to V2
    const uint32_t crc = os::crc32(bytes, V1_SIZE - 4); memcpy(bytes + V1_SIZE - 4, &crc, 4);
    rejectedBlob(bytes, V1_SIZE);
  }
  for (uint8_t version : {2,3,255}) {
    memcpy(bytes, V1_FIXTURE, V1_SIZE); bytes[4] = version;
    const uint32_t crc = os::crc32(bytes, V1_SIZE - 4); memcpy(bytes + V1_SIZE - 4, &crc, 4);
    rejectedBlob(bytes, V1_SIZE);
  }
  memcpy(bytes, V1_FIXTURE, V1_SIZE); bytes[6] = V2_SIZE;
  const uint32_t crc = os::crc32(bytes, V1_SIZE - 4); memcpy(bytes + V1_SIZE - 4, &crc, 4);
  rejectedBlob(bytes, V1_SIZE);
}
void frozenV2Migration() {
  static_assert(sizeof V2_FIXTURE == V2_SIZE && V2_SIZE == 92, "Keep V2 fixture frozen");
  State s = fresh(); assert(load(V2_FIXTURE, sizeof V2_FIXTURE, s));
  assertV1Progress(s);
  assertDisplayDefaults(s);
  assert(!memcmp(reinterpret_cast<const uint8_t*>(&s) + 8, V2_FIXTURE + 8, V2_SIZE - 12));
  assert(s.hairColor == 3 && s.worldX == 80 && s.worldY == 80 && s.worldPlace == HOME && !s.schoolPaper);
}
void invalidV2Saves() {
  uint8_t bytes[V2_SIZE];
  for (size_t i = 0; i < V2_SIZE; ++i) {
    memcpy(bytes, V2_FIXTURE, V2_SIZE); bytes[i] ^= 1; rejectedBlob(bytes, V2_SIZE);
  }
  for (size_t offset : {0u,4u,5u,6u,7u,84u,85u,86u,87u}) {
    memcpy(bytes, V2_FIXTURE, V2_SIZE); bytes[offset] = offset == 84 ? 4 : bytes[offset] ^ 1;
    const uint32_t crc = os::crc32(bytes, V2_SIZE - 4); memcpy(bytes + V2_SIZE - 4, &crc, 4);
    rejectedBlob(bytes, V2_SIZE);
  }
}
void profileScenes() {
  State profiles[4], out{};
  for (uint8_t id = 0; id < 4; ++id) {
    State& s = profiles[id]; s = fresh(); s.doll = id; s.page = CAFE_QUANTITY; s.resumePage = CAFE_QUANTITY;
    s.schoolPaper = id % 2; assert(moveTo(s, WORLD_MIN_X + id, WORLD_MAX_Y - id, id)); seal(s);
  }
  for (int id = 3; id >= 0; --id) {
    assert(load(&profiles[id], sizeof(State), out)); assert(!memcmp(&out, &profiles[id], sizeof out));
  }
  State s = profiles[0]; const State before = s;
  assert(!moveTo(s, WORLD_MIN_X - 1, 80, HOME)); assert(!memcmp(&s, &before, sizeof s));
  assert(!moveTo(s, 80, WORLD_MAX_Y + 1, HOME)); assert(!memcmp(&s, &before, sizeof s));
  assert(!moveTo(s, 80, 80, PARK + 1)); assert(!memcmp(&s, &before, sizeof s));
}
void historicalPages() {
  uint8_t bytes[V2_SIZE];
  for (size_t size : {V1_SIZE, V2_SIZE}) for (size_t offset : {8u, 47u, 23u}) for (uint8_t page : {CAFE_QUANTITY, SCHOOL_HANG}) {
    memcpy(bytes, size == V1_SIZE ? V1_FIXTURE : V2_FIXTURE, size);
    bytes[offset] = offset == 23 ? 3 : page;
    const uint32_t crc = os::crc32(bytes, size - 4); memcpy(bytes + size - 4, &crc, 4);
    rejectedBlob(bytes, size);
  }
}
void invalidRanges() {
  struct Range { uint8_t State::*member; uint8_t invalid; };
  const Range ranges[] = {
    {&State::page, PAGE_COUNT}, {&State::doll, 4}, {&State::hair, 3}, {&State::skin, 4},
    {&State::outfit, 3}, {&State::shoes, 3}, {&State::accessory, 3}, {&State::color, 4},
    {&State::dollName, 4}, {&State::category, 9}, {&State::hairColor, 4}, {&State::destination, 4}, {&State::bag, 32},
    {&State::packItem, 5}, {&State::menuItem, 4}, {&State::count, 9}, {&State::variant, 3},
    {&State::clue, 3}, {&State::guess, 3}, {&State::resumePage, PAGE_COUNT}, {&State::bookIndex, 8},
    {&State::caption, 3}, {&State::sticker, 3}, {&State::schoolOutcome, 5},
    {&State::listened, 2}, {&State::repaired, 4}, {&State::compared, 2}, {&State::ordered, 2},
    {&State::revised, 2}, {&State::active, 2}, {&State::recorded, 2}, {&State::clues, 8},
    {&State::completed, 1}, {&State::worldX, WORLD_MIN_X - 1}, {&State::worldX, WORLD_MAX_X + 1},
    {&State::worldY, WORLD_MIN_Y - 1}, {&State::worldY, WORLD_MAX_Y + 1},
    {&State::worldPlace, PARK + 1}, {&State::schoolPaper, 2}, {&State::schoolHangMask, 8}, {&State::schoolPicture, 3},
    {&State::schoolPictures, 3}, {&State::schoolPictures, 12}, {&State::schoolPictures, 48},
    {&State::schoolPictures, 64}, {&State::schoolTask, 3}
  };
  for (const auto& range : ranges) { State s = fresh(); s.*range.member = range.invalid; seal(s); rejected(s); }
  for (int i = 0; i < 4; ++i) {
    State s = fresh(); s.cart[i] = 4; seal(s); rejected(s);
    s = fresh(); s.compare[i] = 4; seal(s); rejected(s);
  }
  for (int i = 0; i < 3; ++i) {
    State s = fresh(); seal(s); s.reserved[i] = 1; s.crc = os::crc32(&s, sizeof s - 4); rejected(s);
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
  s.page = BOOK; s.resumePage = MEMORY; s.doll = 3; s.skin = 3; s.hair = 2; s.hairColor = 3; s.category = 8;
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
  s.version = 1; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
  s.version = VERSION + 1; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
  s.size -= 1; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
  s.size = V1_SIZE; s.crc = os::crc32(&s, sizeof s - 4); rejected(s); seal(s);
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
  sceneRules(); displayRules(); displayReopen(); displayResume(); budget(); schoolConsequences(); replayAndMemories(); coherentSeeds(); saves(); richRoundtrip();
  frozenV1Migration(); frozenV2Migration(); historicalPages(); invalidV1Saves(); invalidV2Saves(); profileScenes();
  puts("Toy Town rules/save contracts passed");
}
