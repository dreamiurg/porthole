// Self-check for the shell's profile store: records, migration from Pets Club houses (and power loss during it),
// migration of Biscuit's pre-Porthole save, delete, the secret code encoding, rest budget, daily cap.
// Run: make test
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "games/biscuit/pet.h"
#include "games/pets-club/pet.h"
#include "crc32.h"
#include "profiles.h"

// In-memory stand-in for NVS: (namespace, key) -> bytes. `budget` >= 0 is power loss: after that many writes
// (saves and erases) every further write is dropped. `last` is the most recent write that landed.
struct MemStore : shell::Store {
  struct Entry { char ns[16], key[16]; uint8_t data[shell::BLOB_MAX]; size_t len; bool used; };
  Entry e[32] = {};
  int budget = -1, writes = 0; char last[32] = "";
  bool land(const char* ns, const char* key) {
    if (budget >= 0 && writes >= budget) return false;
    writes++; snprintf(last, sizeof last, "%s/%s", ns, key); return true;
  }
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
    if (!land(ns, key)) return;
    Entry* x = find(ns, key);
    for (int i = 0; !x && i < 32; i++) if (!e[i].used) x = &e[i];
    assert(x && len <= shell::BLOB_MAX);
    snprintf(x->ns, sizeof x->ns, "%s", ns); snprintf(x->key, sizeof x->key, "%s", key);
    memcpy(x->data, data, len); x->len = len; x->used = true;
  }
  void erase(const char* ns, const char* key) override { if (!land(ns, key)) return; if (Entry* x = find(ns, key)) x->used = false; }
  bool has(const char* ns, const char* key) { return find(ns, key) != nullptr; }
};

static const char* const STORES[] = {"crago", "other"};
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
  memset(r.name, 'A', sizeof r.name); shell::seal(r);          // a valid CRC over an unterminated name
  assert(shell::loadRecord(&r, sizeof r, out) && strlen(out.name) == sizeof out.name - 1);
}

// A code is stored as its value + 1 so 0 can mean "none": 0000 and 0001 are different codes.
static void pinCodes() {
  assert(shell::pinCode("") == 0 && shell::pinCode(nullptr) == 0);
  assert(shell::pinCode("0000") != 0 && shell::pinCode("0000") != shell::pinCode("0001"));
  assert(shell::pinCode("1234") == 1235 && shell::pinCode("9999") == 10000);
}

static void createAndLimit() {
  MemStore st; shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 0);
  for (int i = 0; i < MAX_PROFILES; i++) assert(shell::create(st, p, draft("Kid", 7, 0), STORES, 2) == i);
  assert(shell::create(st, p, draft("Extra", 7, 0), STORES, 2) == -1);   // four is the limit
  assert(st.has(shell::NS, "p3"));
  shell::Profiles again; shell::loadAll(st, again);            // round trip through the store
  assert(again.count() == MAX_PROFILES && again.rec[2].age == 7 && again.nth(3) == 3 && again.nth(4) == -1);
}

// Delete wipes that id's save in every app store, then the record (last: power loss mid-delete leaves a profile
// without a pup, never a pup waiting for whoever takes the id next), keeps everyone else, and frees the id.
static void removal() {
  MemStore st; shell::Profiles p; shell::loadAll(st, p);
  shell::create(st, p, draft("Sam", 8, 0), STORES, 2); shell::create(st, p, draft("Kai", 6, 1234), STORES, 2);
  Save s = house("Kai", "Rex", 6, 0);
  st.save("crago", "s0", &s, sizeof s); st.save("crago", "s1", &s, sizeof s); st.save("other", "s1", &s, 4);
  shell::removeProfile(st, p, 1, STORES, 2);
  assert(!strcmp(st.last, "porthole/p1"));
  assert(!p.used[1] && p.count() == 1 && p.nth(0) == 0 && p.nth(1) == -1);
  assert(!st.has(shell::NS, "p1") && !st.has("crago", "s1") && !st.has("other", "s1"));
  assert(st.has(shell::NS, "p0") && st.has("crago", "s0"));
  shell::Profiles again; shell::loadAll(st, again);
  assert(again.count() == 1);                                 // no migration: the marker is there
  assert(shell::create(st, p, draft("Ava", 5, 0), STORES, 2) == 1);   // the freed id is reused
}

// A new profile never inherits a save left behind under its id (a delete cut short by power loss).
static void createClearsStale() {
  MemStore st; shell::Profiles p; shell::loadAll(st, p);
  shell::create(st, p, draft("Sam", 8, 0), STORES, 2);
  Save s = house("Old", "Ghost", 6, 0);
  st.save("crago", "s1", &s, sizeof s); st.save("other", "s1", &s, 4); st.save("crago", "s0", &s, sizeof s);
  assert(shell::create(st, p, draft("Kai", 6, 0), STORES, 2) == 1);
  assert(!st.has("crago", "s1") && !st.has("other", "s1") && st.has("crago", "s0") && st.has(shell::NS, "p1"));
}

static bool same(const shell::Record& r, const char* name, int age, int avatar, int pin) {
  return !strcmp(r.name, name) && r.age == age && r.avatar == avatar && r.pin == pin;
}
static const char* petIn(MemStore& st, const char* key) {   // the pup name in a Pets Club save, "" if none
  static Save got; uint8_t blob[shell::BLOB_MAX];
  size_t n = st.load("crago", key, blob, sizeof blob);
  return n && pet::loadBlob(blob, n, got) ? got.petName : "";
}
// v2 houses in s0 and s2 (s1 missing): profiles 0 and 2, keeping their slot numbers; no save moves.
static void migrateGap() {
  MemStore st;
  Save a = house("Sam", "Biscuit", 8, 0), b = house("Kai", "Rex", 6, 1234);
  st.save("crago", "s0", &a, sizeof a); st.save("crago", "s2", &b, sizeof b);
  shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 2 && p.used[0] && !p.used[1] && p.used[2] && p.nth(1) == 2);
  assert(same(p.rec[0], "Sam", 8, 0, 0) && same(p.rec[2], "Kai", 6, 2, 1235));   // code 1234 stored as 1235
  assert(p.rec[2].muted == 1 && p.rec[2].restUntil == NOW + 60 && p.rec[2].playSec == 42);
  assert(!strcmp(petIn(st, "s0"), "Biscuit") && !strcmp(petIn(st, "s2"), "Rex") && !st.has("crago", "s1"));
  assert(st.has(shell::NS, "p0") && st.has(shell::NS, "p2") && st.has(shell::NS, "m"));
  shell::Profiles again; shell::loadAll(st, again);           // second boot: records, no second migration
  assert(again.count() == 2 && !strcmp(again.rec[2].name, "Kai"));
  assert(shell::create(st, again, draft("Ava", 5, 0), STORES, 2) == 1 && !strcmp(petIn(st, "s2"), "Rex"));
}

// Power lost after every possible write of a migration: the next boot still ends with every house a profile,
// each with its own pup.
static void setupHouses(MemStore& st) {
  Save a = house("Sam", "Biscuit", 8, 0), b = house("Kai", "Rex", 6, 1234), c = house("Ava", "Pip", 7, 0);
  st.save("crago", "s0", &a, sizeof a); st.save("crago", "s1", &b, sizeof b); st.save("crago", "s2", &c, sizeof c);
  st.save("crago", "save", &a, sizeof a);   // a stale pre-house save Pets Club never cleaned up
}
static void setupLegacy(MemStore& st) { Save a = house("Sam", "Biscuit", 8, 0); st.save("crago", "save", &a, sizeof a); }
static void crashDuringMigration(void (*setup)(MemStore&), int houses, int writes) {
  static const char* const NAMES[] = {"Sam", "Kai", "Ava"}, *const PETS[] = {"Biscuit", "Rex", "Pip"};
  MemStore clean; setup(clean); int base = clean.writes;
  shell::Profiles p; shell::loadAll(clean, p);
  int total = clean.writes - base;
  assert(total == writes);
  for (int n = 0; n <= total; n++) {
    MemStore st; setup(st); st.budget = st.writes + n;
    shell::Profiles cut; shell::loadAll(st, cut);              // the power goes out after n writes
    st.budget = -1;
    shell::Profiles boot; shell::loadAll(st, boot);            // and comes back
    // (cut between the marker and the last erase, "save" lingers: harmless, the marker keeps it from being read)
    assert(boot.count() == houses && st.has(shell::NS, "m") && (n == total - 2 || !st.has("crago", "save")));
    for (int i = 0; i < houses; i++) {
      assert(boot.used[i] && !strcmp(boot.rec[i].name, NAMES[i]) && boot.rec[i].avatar == i);
      assert(!strcmp(petIn(st, shell::key('s', i)), PETS[i]));
    }
  }
}
static void migrateCrash() {
  crashDuringMigration(setupHouses, 3, 6);   // p0 p1 p2, marker, erase "save", Biscuit's marker (no old Biscuit)
  crashDuringMigration(setupLegacy, 1, 5);   // s0 (copy of "save"), p0, marker, erase "save", Biscuit's marker
}

// Deleting every migrated profile sticks: the marker (and the erased legacy key) keep them from coming back.
static void deleteAllStaysDeleted() {
  MemStore st; setupHouses(st);
  shell::Profiles p; shell::loadAll(st, p);
  for (int id = 0; id < 3; id++) shell::removeProfile(st, p, id, STORES, 2);
  shell::Profiles boot; shell::loadAll(st, boot);
  assert(boot.count() == 0 && !st.has("crago", "s0") && !st.has("crago", "save"));
}

// Every record corrupt but the marker present: the shell starts fresh, it does not migrate the houses again.
static void corruptNoRemigrate() {
  MemStore st; Save a = house("Sam", "Biscuit", 8, 0); st.save("crago", "s0", &a, sizeof a);
  shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 1);
  uint8_t junk[sizeof(shell::Record)] = {1, 2, 3}; st.save(shell::NS, "p0", junk, sizeof junk);
  shell::Profiles boot; shell::loadAll(st, boot);
  assert(boot.count() == 0 && !boot.used[0]);
  uint8_t rec[shell::BLOB_MAX]; assert(st.load(shell::NS, "p0", rec, sizeof rec) == sizeof junk && rec[0] == 1);
}

// A v1 blob (140 bytes, before names and ages were per house) still becomes a profile; age stays unknown.
static void migrateV1() {
  MemStore st;
  Save s = house("Mia", "Pip", 0, 0);
  uint8_t v1[SAVE_V1_SIZE]; memcpy(v1, &s, SAVE_V1_SIZE - 4);
  uint16_t ver = 1, size = (uint16_t)SAVE_V1_SIZE; memcpy(v1 + 4, &ver, 2); memcpy(v1 + 6, &size, 2);
  uint32_t crc = pet::crc32(v1, SAVE_V1_SIZE - 4); memcpy(v1 + SAVE_V1_SIZE - 4, &crc, 4);
  st.save("crago", "s0", v1, sizeof v1);
  shell::Profiles p; shell::loadAll(st, p);
  assert(p.count() == 1 && !strcmp(p.rec[0].name, "Mia") && p.rec[0].age == 0 && p.rec[0].pin == 0);
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
  assert(!strcmp(petIn(st, "s0"), "Biscuit") && !st.has("crago", "save"));
  MemStore empty; shell::Profiles none; shell::loadAll(empty, none);
  assert(none.count() == 0);                                  // a fresh device has nothing to migrate
}

// ---- Biscuit's save from before Porthole ----
// Built the way the old firmware wrote them: {magic, pet, FNV-1a over the pet} with UTC stamps, in namespace
// "zoegotchi" (keys pet3/pet2/pet1) or "biscuit" (key pet1).
namespace old = biscuit::legacy;
static const char* const OLD_NS = "zoegotchi";   // the first firmware's namespace
static uint32_t oldChecksum(const void* d, size_t n) {   // FNV-1a, the old checksum, written out again so the test checks the port
  uint32_t h = 2166136261u; const uint8_t* p = (const uint8_t*)d;
  while (n--) h = (h ^ *p++) * 16777619u;
  return h;
}
template <class P> static void putOld(MemStore& st, const char* ns, const char* key, uint32_t magic, const P& pet) {
  old::Blob<P> b; memset(&b, 0, sizeof b);
  b.magic = magic; b.pet = pet; b.hash = oldChecksum(&b.pet, sizeof b.pet);
  st.save(ns, key, &b, sizeof b);
}
static old::PetV1 oldPet(uint32_t friendship, uint32_t ageSec) {   // last saved ageSec before NOW
  old::PetV1 o; memset(&o, 0, sizeof o);
  o.updatedAt = NOW - ageSec + old::UTC_OFFSET_SEC; o.createdAt = o.updatedAt - 86400;
  o.version = 1; o.lastVisitDay = (int32_t)(NOW / 86400);
  o.fullness = 60; o.happiness = 70; o.energy = 80; o.friendship = friendship; o.daysTogether = 4; o.stories = 5;
  return o;
}
static old::PetV3 oldNamed(const char* kid, const char* dog, uint32_t friendship, uint32_t version) {
  old::PetV3 o; memset(&o, 0, sizeof o);
  o.base = oldPet(friendship, 0); o.base.version = version; o.setupComplete = 1;
  snprintf(o.playerName, sizeof o.playerName, "%s", kid); snprintf(o.petName, sizeof o.petName, "%s", dog);
  return o;
}
static bool biscuitIn(MemStore& st, int id, const char* dog, uint32_t friendship) {   // biscuit/s<id> holds this pup
  uint8_t blob[shell::BLOB_MAX]; biscuit::Save s;
  size_t n = st.load(biscuit::STORE, shell::key('s', id), blob, sizeof blob);
  return n && biscuit::loadBlob(blob, n, s) && !strcmp(s.petName, dog) && s.friendship == friendship;
}
static bool oldGone(MemStore& st) {
  return !st.has(OLD_NS, "pet3") && !st.has(OLD_NS, "pet2") && !st.has(OLD_NS, "pet1") && !st.has("biscuit", "pet1");
}
// Boot: profiles, the Pets Club migration, then Biscuit's (loadAll runs both).
static void boot(MemStore& st, shell::Profiles& p) { shell::loadAll(st, p); }

// The pup goes to the profile named like the kid in the save, whatever the case; the old keys go after the marker.
static void biscuitToNamedProfile() {
  MemStore st;
  Save a = house("Sam", "Rex", 8, 0), b = house("Kai", "Dot", 6, 0);
  st.save("crago", "s0", &a, sizeof a); st.save("crago", "s1", &b, sizeof b);
  putOld(st, OLD_NS, "pet3", old::MAGIC_V3, oldNamed("KAI", "Pip", 30, 3));
  putOld(st, OLD_NS, "pet1", old::MAGIC_V1, oldPet(99, 3600));   // an older generation: never read
  shell::Profiles p; boot(st, p);
  assert(p.count() == 2 && biscuitIn(st, 1, "Pip", 30) && !st.has(biscuit::STORE, "s0"));
  assert(st.has(shell::NS, "mb") && oldGone(st) && !strcmp(petIn(st, "s1"), "Dot"));
  putOld(st, OLD_NS, "pet3", old::MAGIC_V3, oldNamed("Sam", "Ghost", 1, 3));
  shell::Profiles again; boot(st, again);                   // once only: the marker stops a second run
  assert(again.count() == 2 && biscuitIn(st, 1, "Pip", 30) && !st.has(biscuit::STORE, "s0"));
}

// No profile yet: one is made from the kid's name in the save (age unknown, asked once); a save without a name
// (v1) gets a neutral one.
static void biscuitNewProfile() {
  MemStore st;
  putOld(st, OLD_NS, "pet2", old::MAGIC_V2, oldNamed("Sam", "Pip", 30, 2));
  shell::Profiles p; boot(st, p);
  assert(p.count() == 1 && same(p.rec[0], "Sam", 0, 0, 0) && biscuitIn(st, 0, "Pip", 30) && oldGone(st));
  MemStore v1; putOld(v1, "biscuit", "pet1", old::MAGIC_V1, oldPet(7, 0));
  shell::Profiles q; boot(v1, q);
  assert(q.count() == 1 && !strcmp(q.rec[0].name, "Friend") && q.rec[0].age == 0 && biscuitIn(v1, 0, "Biscuit", 7));
  assert(oldGone(v1));
}

// A name longer than the shell lets a kid type: the profile gets its first ui::NAME_LEN letters, and that is also
// what matches.
static void biscuitLongName() {
  MemStore st;
  putOld(st, OLD_NS, "pet3", old::MAGIC_V3, oldNamed("Christopher", "Pip", 30, 3));
  shell::Profiles p; boot(st, p);
  assert(p.count() == 1 && !strcmp(p.rec[0].name, "Christop") && biscuitIn(st, 0, "Pip", 30));
  MemStore two;
  Save a = house("Sam", "Rex", 8, 0), b = house("CHRISTOP", "Dot", 6, 0);
  two.save("crago", "s0", &a, sizeof a); two.save("crago", "s1", &b, sizeof b);
  putOld(two, OLD_NS, "pet3", old::MAGIC_V3, oldNamed("Christopher", "Pip", 30, 3));
  shell::Profiles q; boot(two, q);
  assert(q.count() == 2 && biscuitIn(two, 1, "Pip", 30) && !two.has(biscuit::STORE, "s0"));
}

// Both old places hold a save: the newer one wins. No name to match: the first profile gets it.
static void biscuitNewestToFirstProfile() {
  MemStore st;
  Save a = house("Ava", "Rex", 7, 0);
  st.save("crago", "s2", &a, sizeof a);                               // one profile, id 2
  putOld(st, OLD_NS, "pet1", old::MAGIC_V1, oldPet(10, 3600));
  putOld(st, "biscuit", "pet1", old::MAGIC_V1, oldPet(20, 60));       // saved later
  shell::Profiles p; boot(st, p);
  assert(p.count() == 1 && biscuitIn(st, 2, "Biscuit", 20));
  assert(!st.has("biscuit", "pet1") && st.has(OLD_NS, "pet1"));        // the other place is left alone
  MemStore other;
  putOld(other, OLD_NS, "pet3", old::MAGIC_V3, oldNamed("Sam", "Pip", 30, 3));
  putOld(other, "biscuit", "pet1", old::MAGIC_V1, oldPet(20, 3600));  // older than pet3
  shell::Profiles q; boot(other, q);
  assert(q.count() == 1 && !strcmp(q.rec[0].name, "Sam") && biscuitIn(other, 0, "Pip", 30));
  assert(!other.has(OLD_NS, "pet3") && other.has("biscuit", "pet1"));
}

// A save that does not convert is left where it is and nothing is made from it. In the first firmware the newest key
// decides: a damaged pet3 hides an older pet2, as it did on that firmware.
static void biscuitCorruptIgnored() {
  MemStore st;
  old::PetV3 bad = oldNamed("Sam", "Pip", 30, 3); bad.base.energy = 500;
  putOld(st, OLD_NS, "pet3", old::MAGIC_V3, bad);
  putOld(st, OLD_NS, "pet2", old::MAGIC_V2, oldNamed("Sam", "Old", 5, 2));
  uint8_t junk[40] = {1, 2, 3}; st.save("biscuit", "pet1", junk, sizeof junk);
  shell::Profiles p; boot(st, p);
  assert(p.count() == 0 && !st.has(biscuit::STORE, "s0") && st.has(shell::NS, "mb"));
  assert(st.has(OLD_NS, "pet3") && st.has(OLD_NS, "pet2") && st.has("biscuit", "pet1"));
}

// Power lost after every write of the Biscuit migration: the next boot still ends with the one profile and its pup.
static void biscuitCrash() {
  auto setup = [](MemStore& st) { putOld(st, OLD_NS, "pet3", old::MAGIC_V3, oldNamed("Sam", "Pip", 30, 3)); };
  MemStore clean; setup(clean); int base = clean.writes;
  shell::Profiles p; boot(clean, p);
  int total = clean.writes - base;
  assert(total == 10);   // marker, erase "save" (Pets Club); erase s0 twice, p0, biscuit/s0, marker, three erases
  for (int n = 0; n <= total; n++) {
    MemStore st; setup(st); st.budget = st.writes + n;
    shell::Profiles cut; boot(st, cut);
    st.budget = -1;
    shell::Profiles again; boot(st, again);
    assert(again.count() == 1 && !strcmp(again.rec[0].name, "Sam") && biscuitIn(st, 0, "Pip", 30));
    assert(st.has(shell::NS, "mb") && (n == 7 || oldGone(st)));   // cut right after the marker: pet3 lingers, unread
  }
}

static void restBudget() {
  shell::Record r = draft("Sam", 8, 0);
  uint32_t t = NOW;
  for (int i = 0; i < 1000; i++) assert(!shell::play(r, t += 1, 1, 1, false));   // one profile: nobody to hand over to
  assert(r.playSec == 0 && r.lastPlayed == t);
  assert(!shell::play(r, t += 60, 60, 2, false) && r.playSec == 5);               // a clock jump counts 5 s at most
  r.playSec = shell::SESSION_SEC - 1;
  assert(shell::play(r, t += 1, 1, 2, false));                                    // the budget runs out: rest begins
  assert(r.playSec == 0 && shell::resting(r, t) && shell::resting(r, t + shell::REST_SEC - 1));
  assert(!shell::play(r, t += 1, 1, 2, false) && r.playSec == 0);                 // resting time does not count
  assert(!shell::resting(r, r.restUntil));
  shell::Record b = draft("Kai", 6, 0);
  b.playSec = 100; b.lastPlayed = t;
  shell::recharge(b, t + shell::REST_SEC - 1); assert(b.playSec == 100);   // a short break keeps the budget
  shell::recharge(b, t + shell::REST_SEC); assert(b.playSec == 0);         // a real break refills it
  // A clock set backwards must not strand anyone: a rest never lasts longer than REST_SEC from now...
  shell::Record c = draft("Ava", 7, 0);
  c.restUntil = t + 5 * 3600; assert(!shell::resting(c, t));
  c.restUntil = t + shell::REST_SEC; assert(shell::resting(c, t));
  // ...and "last played in the future" is not a real break.
  c.restUntil = 0; c.playSec = 100; c.lastPlayed = t + 3600;
  shell::recharge(c, t); assert(c.playSec == 100);
}

// Daily cap, v2 record fields: a v1 record (44 bytes) still loads, with nothing played today.
static void recordV1() {
  shell::Record r = draft("Sam", 8, 1234); r.playSec = 42; shell::seal(r);
  uint8_t v1[shell::REC_V1_SIZE]; memcpy(v1, &r, sizeof v1 - 4);
  uint16_t ver = 1, size = (uint16_t)shell::REC_V1_SIZE; memcpy(v1 + 4, &ver, 2); memcpy(v1 + 6, &size, 2);
  uint32_t crc = os::crc32(v1, sizeof v1 - 4); memcpy(v1 + sizeof v1 - 4, &crc, 4);
  shell::Record out;
  assert(shell::loadRecord(v1, sizeof v1, out) && !strcmp(out.name, "Sam") && out.pin == 1234 && out.playSec == 42);
  assert(out.dayPlaySec == 0 && out.playDay == 0 && out.version == shell::REC_VERSION && out.size == sizeof(shell::Record));
  v1[10] ^= 1; assert(!shell::loadRecord(v1, sizeof v1, out));   // still CRC-checked
  r.dayPlaySec = 600; r.playDay = NOW / shell::DAY_SEC; shell::seal(r);   // the new fields round-trip
  assert(shell::loadRecord(&r, sizeof r, out) && out.dayPlaySec == 600 && out.playDay == NOW / shell::DAY_SEC);
  uint8_t wrong[sizeof r]; memcpy(wrong, &r, sizeof r); memcpy(wrong + 4, &ver, 2);
  crc = os::crc32(wrong, sizeof wrong - 4); memcpy(wrong + sizeof wrong - 4, &crc, 4);
  assert(!shell::loadRecord(wrong, sizeof wrong, out));   // a v1 header on a v2-sized blob is not a record
}

// 25 min a day per profile, alone or not; used up means resting until local midnight, then a fresh budget.
static void dailyCap() {
  shell::Record r = draft("Sam", 8, 0);
  uint32_t t = NOW;   // 10:00
  for (uint32_t i = 1; i < shell::DAILY_SEC; i++) assert(!shell::play(r, t += 1, 1, 1, false));
  assert(r.dayPlaySec == shell::DAILY_SEC - 1 && shell::restLeft(r, t, 1) == 0);
  assert(shell::play(r, t += 1, 1, 1, false));                      // the last second: done for today
  assert(shell::playedToday(r, t) && shell::restLeft(r, t, 1) == 14 * 3600 - shell::DAILY_SEC);
  assert(shell::play(r, t += 60, 60, 1, false));                    // still done (a sim hook kept a game open)
  uint32_t midnight = NOW - NOW % shell::DAY_SEC + shell::DAY_SEC;
  assert(shell::restLeft(r, midnight - 1, 1) == 1 && shell::restLeft(r, midnight, 1) == 0);
  assert(!shell::play(r, midnight, 1, 1, false) && r.dayPlaySec == 1 && r.playDay == midnight / shell::DAY_SEC);
  // with two profiles the cap wins over a turn: it ends the session too
  shell::Record k = draft("Kai", 6, 0);
  k.playDay = NOW / shell::DAY_SEC; k.dayPlaySec = shell::DAILY_SEC - 1; k.playSec = 100;
  assert(shell::play(k, NOW, 1, 2, false) && k.playSec == 0 && shell::playedToday(k, NOW) && !shell::resting(k, NOW));
  assert(shell::restLeft(k, NOW, 2) == 14 * 3600);
}

// No touch for a minute is not play: neither budget moves, but a used-up day still ends the game.
static void idleNotCounted() {
  shell::Record r = draft("Sam", 8, 0);
  uint32_t t = NOW;
  for (int i = 0; i < 3600; i++) assert(!shell::play(r, t += 1, 1, 2, true));
  assert(r.dayPlaySec == 0 && r.playSec == 0 && r.lastPlayed == t);
  assert(!shell::play(r, t += 1, 1, 2, false) && r.dayPlaySec == 1 && r.playSec == 1);
  r.dayPlaySec = shell::DAILY_SEC;
  assert(shell::play(r, t += 1, 1, 2, true));
}

// The clock set back never strands anyone: to an earlier day it lifts the cap; within the day the rest still ends
// at that day's midnight, so it never lasts more than a day.
static void capClockBack() {
  shell::Record r = draft("Sam", 8, 0);
  r.playDay = NOW / shell::DAY_SEC; r.dayPlaySec = shell::DAILY_SEC;
  assert(shell::restLeft(r, NOW, 1) > 0 && shell::restLeft(r, NOW - 3600, 1) == 15 * 3600);
  assert(shell::restLeft(r, NOW - shell::DAY_SEC, 1) == 0 && !shell::playedToday(r, NOW - 2 * shell::DAY_SEC));
  assert(!shell::play(r, NOW - shell::DAY_SEC, 1, 1, false) && r.dayPlaySec == 1);   // yesterday is a new budget
  // a turn rest stays clamped to REST_SEC and only counts with someone to hand over to
  shell::Record k = draft("Kai", 6, 0); k.restUntil = NOW + shell::REST_SEC;
  assert(shell::restLeft(k, NOW, 2) == shell::REST_SEC && shell::restLeft(k, NOW, 1) == 0);
  k.restUntil = NOW + 5 * 3600; assert(shell::restLeft(k, NOW, 2) == 0);
}

// RTC lost: the firmware guesses the clock as the newest lastPlayed + 60 s, the same day a capped kid was capped
// (a capped kid never moves lastPlayed). liftCaps frees them, on disk too, and leaves untouched records unwritten.
static void capAfterClockGuess() {
  MemStore st; shell::Profiles p; shell::loadAll(st, p);
  shell::create(st, p, draft("Sam", 8, 0), STORES, 2); shell::create(st, p, draft("Kai", 6, 0), STORES, 2);
  shell::Record& sam = p.rec[0];
  sam.playDay = NOW / shell::DAY_SEC; sam.dayPlaySec = shell::DAILY_SEC; sam.lastPlayed = NOW; shell::saveRecord(st, p, 0);
  uint32_t guess = sam.lastPlayed + 60;
  assert(shell::restLeft(sam, guess, 2) > 0);                 // without the fix: "Back tomorrow" on every cold boot
  int before = st.writes;
  shell::liftCaps(st, p);
  assert(shell::restLeft(p.rec[0], guess, 2) == 0 && st.writes == before + 1 && !strcmp(st.last, "porthole/p0"));
  shell::Profiles boot; shell::loadAll(st, boot);
  assert(boot.rec[0].dayPlaySec == 0 && shell::restLeft(boot.rec[0], guess, 2) == 0);
}

int main() {
  records();
  pinCodes();
  createAndLimit();
  removal();
  createClearsStale();
  migrateGap();
  migrateCrash();
  deleteAllStaysDeleted();
  corruptNoRemigrate();
  migrateV1();
  migrateLegacyKey();
  biscuitToNamedProfile();
  biscuitNewProfile();
  biscuitNewestToFirstProfile();
  biscuitLongName();
  biscuitCorruptIgnored();
  biscuitCrash();
  restBudget();
  recordV1();
  dailyCap();
  idleNotCounted();
  capClockBack();
  capAfterClockGuess();
  printf("test_shell: all checks passed (sizeof Record = %zu)\n", sizeof(shell::Record));
  return 0;
}
