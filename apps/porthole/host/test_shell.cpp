// Self-check for the shell's profile store: records, migration from Pets Club houses, delete, rest budget.
// Run: make test
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "pet.h"
#include "profiles.h"

// In-memory stand-in for NVS: (namespace, key) -> bytes.
struct MemStore : shell::Store {
  struct Entry { char ns[16], key[16]; uint8_t data[shell::BLOB_MAX]; size_t len; bool used; };
  Entry e[32] = {};
  Entry* find(const char* ns, const char* key) {
    for (auto& x : e) if (x.used && !strcmp(x.ns, ns) && !strcmp(x.key, key)) return &x;
    return nullptr;
  }
  size_t load(const char* ns, const char* key, void* buf, size_t max) override {
    Entry* x = find(ns, key);
    if (!x || x->len > max) return 0;
    memcpy(buf, x->data, x->len); return x->len;
  }
  void save(const char* ns, const char* key, const void* data, size_t len) override {
    Entry* x = find(ns, key);
    for (int i = 0; !x && i < 32; i++) if (!e[i].used) x = &e[i];
    assert(x && len <= shell::BLOB_MAX);
    snprintf(x->ns, sizeof x->ns, "%s", ns); snprintf(x->key, sizeof x->key, "%s", key);
    memcpy(x->data, data, len); x->len = len; x->used = true;
  }
  void erase(const char* ns, const char* key) override { if (Entry* x = find(ns, key)) x->used = false; }
  bool has(const char* ns, const char* key) { return find(ns, key) != nullptr; }
};

static const uint32_t NOW = 1790000000u - (1790000000u % 86400u) + 10 * 3600;

static Save house(const char* kid, const char* dog, uint8_t age, uint16_t pin) {
  Save s; pet::adopt(s, NOW - 86400, kid, dog);
  s.kidAge = age; s.pin = pin; s.muted = 1; s.restUntil = NOW + 60; s.playSec = 42; pet::seal(s);
  return s;
}
static shell::Record draft(const char* name, uint8_t age, uint16_t pin) {
  shell::Record r = {}; snprintf(r.name, sizeof r.name, "%s", name); r.age = age; r.pin = pin; return r;
}

static void records() {
  shell::Record r = draft("Sam", 8, 1234); shell::seal(r);
  shell::Record out;
  assert(shell::loadRecord(&r, sizeof r, out) && !strcmp(out.name, "Sam") && out.pin == 1234);
  shell::Record bad = r; bad.age ^= 1;
  assert(!shell::loadRecord(&bad, sizeof bad, out));          // CRC mismatch
  assert(!shell::loadRecord(&r, sizeof r - 1, out));          // truncated
}

static void createAndLimit() {
  MemStore st; shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 0);
  for (int i = 0; i < MAX_PROFILES; i++) assert(shell::create(st, p, draft("Kid", 7, 0)) == i);
  assert(shell::create(st, p, draft("Extra", 7, 0)) == -1);   // four is the limit
  assert(st.has(shell::NS, "p3"));
  shell::Profiles again; shell::loadAll(st, again);            // round trip through the store
  assert(again.count() == MAX_PROFILES && again.rec[2].age == 7 && again.nth(3) == 3 && again.nth(4) == -1);
}

// Delete wipes the record and that id's save in every app store, keeps everyone else, and frees the id.
static void removal() {
  MemStore st; shell::Profiles p; shell::loadAll(st, p);
  shell::create(st, p, draft("Sam", 8, 0)); shell::create(st, p, draft("Kai", 6, 1234));
  Save s = house("Kai", "Rex", 6, 0);
  st.save("crago", "s0", &s, sizeof s); st.save("crago", "s1", &s, sizeof s); st.save("other", "s1", &s, 4);
  const char* stores[] = {"crago", "other"};
  shell::removeProfile(st, p, 1, stores, 2);
  assert(!p.used[1] && p.count() == 1 && p.nth(0) == 0 && p.nth(1) == -1);
  assert(!st.has(shell::NS, "p1") && !st.has("crago", "s1") && !st.has("other", "s1"));
  assert(st.has(shell::NS, "p0") && st.has("crago", "s0"));
  shell::Profiles again; shell::loadAll(st, again);
  assert(again.count() == 1);                                 // no migration: p0 still exists
  assert(shell::create(st, p, draft("Ava", 5, 0)) == 1);      // the freed id is reused
}

static bool same(const shell::Record& r, const char* name, int age, int avatar, int pin) {
  return !strcmp(r.name, name) && r.age == age && r.avatar == avatar && r.pin == pin;
}
static const char* petIn(MemStore& st, const char* key) {   // the pup name in a Pets Club save, "" if none
  static Save got; uint8_t blob[shell::BLOB_MAX];
  size_t n = st.load("crago", key, blob, sizeof blob);
  return n && pet::loadBlob(blob, n, got) ? got.petName : "";
}
// v2 houses in s0 and s2 (s1 missing): profiles 0 and 1, saves moved to s0/s1, s2 erased.
static void migrateGap() {
  MemStore st;
  Save a = house("Sam", "Biscuit", 8, 0), b = house("Kai", "Rex", 6, 1234);
  st.save("crago", "s0", &a, sizeof a); st.save("crago", "s2", &b, sizeof b);
  shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 2 && p.used[0] && p.used[1] && !p.used[2]);
  assert(same(p.rec[0], "Sam", 8, 0, 0) && same(p.rec[1], "Kai", 6, 1, 1234));
  assert(p.rec[1].muted == 1 && p.rec[1].restUntil == NOW + 60 && p.rec[1].playSec == 42);
  assert(!strcmp(petIn(st, "s0"), "Biscuit") && !strcmp(petIn(st, "s1"), "Rex"));
  assert(!st.has("crago", "s2") && st.has(shell::NS, "p0") && st.has(shell::NS, "p1"));
  shell::Profiles again; shell::loadAll(st, again);           // second boot: records, no second migration
  assert(again.count() == 2 && !strcmp(again.rec[1].name, "Kai"));
}

// A v1 blob (140 bytes, before names and ages were per house) still becomes a profile; age stays unknown.
static void migrateV1() {
  MemStore st;
  Save s = house("Zoe", "Pip", 0, 0);
  uint8_t v1[SAVE_V1_SIZE]; memcpy(v1, &s, SAVE_V1_SIZE - 4);
  uint16_t ver = 1, size = (uint16_t)SAVE_V1_SIZE; memcpy(v1 + 4, &ver, 2); memcpy(v1 + 6, &size, 2);
  uint32_t crc = pet::crc32(v1, SAVE_V1_SIZE - 4); memcpy(v1 + SAVE_V1_SIZE - 4, &crc, 4);
  st.save("crago", "s0", v1, sizeof v1);
  shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 1 && !strcmp(p.rec[0].name, "Zoe") && p.rec[0].age == 0 && p.rec[0].pin == 0);
  assert(p.rec[0].restUntil == 0 && p.rec[0].playSec == 0);
  assert(st.load("crago", "s0", v1, sizeof v1) == SAVE_V1_SIZE);   // same key: left as it was, Pets Club reads v1
}

// The pre-house single "save" key becomes profile 0 in s0.
static void migrateLegacyKey() {
  MemStore st;
  Save s = house("Sam", "Biscuit", 8, 0);
  st.save("crago", "save", &s, sizeof s);
  shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 1 && !strcmp(p.rec[0].name, "Sam"));
  assert(st.has("crago", "s0") && !st.has("crago", "save"));
  MemStore empty; shell::Profiles none; shell::loadAll(empty, none);
  assert(none.count() == 0);                                  // a fresh device has nothing to migrate
}

static void restBudget() {
  shell::Record r = draft("Sam", 8, 0);
  uint32_t t = NOW;
  for (int i = 0; i < 1000; i++) assert(!shell::play(r, t += 1, 1, 1));   // one profile: nobody to hand over to
  assert(r.playSec == 0 && r.lastPlayed == t);
  assert(!shell::play(r, t += 60, 60, 2) && r.playSec == 5);               // a clock jump counts 5 s at most
  r.playSec = shell::SESSION_SEC - 1;
  assert(shell::play(r, t += 1, 1, 2));                                    // the budget runs out: rest begins
  assert(r.playSec == 0 && shell::resting(r, t) && shell::resting(r, t + shell::REST_SEC - 1));
  assert(!shell::play(r, t += 1, 1, 2) && r.playSec == 0);                 // resting time does not count
  assert(!shell::resting(r, r.restUntil));
  shell::Record b = draft("Kai", 6, 0);
  b.playSec = 100; b.lastPlayed = t;
  shell::recharge(b, t + shell::REST_SEC - 1); assert(b.playSec == 100);   // a short break keeps the budget
  shell::recharge(b, t + shell::REST_SEC); assert(b.playSec == 0);         // a real break refills it
}

int main() {
  records();
  createAndLimit();
  removal();
  migrateGap();
  migrateV1();
  migrateLegacyKey();
  restBudget();
  printf("test_shell: all checks passed (sizeof Record = %zu)\n", sizeof(shell::Record));
  return 0;
}
