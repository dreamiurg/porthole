// Shared-shell integration: Kingdom's pages at the save cadence, its decisions at once, and an ordinary game's cadence.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include "games/living-kingdom/game.h"
#include "games/living-kingdom/layout.h"
#include "host/fixtures/kingdom_save_v1.h"
#include "shell.h"

namespace {
using kingdom::Save;
constexpr uint32_t NOW = 1790000000;
struct Memory : shell::Store {
  std::map<std::string, std::vector<uint8_t>> entries;
  int gameWrites = 0; bool fail = false;   // fail: every write is refused, like a worn-out flash
  size_t load(const char* ns, const char* key, void* out, size_t max) override {
    auto i = entries.find(std::string(ns) + "/" + key);
    if (i == entries.end() || i->second.size() > max) return 0;
    memcpy(out, i->second.data(), i->second.size()); return i->second.size();
  }
  bool save(const char* ns, const char* key, const void* bytes, size_t length) override {
    assert(length <= shell::BLOB_MAX);
    if (fail) return false;
    const auto* data = static_cast<const uint8_t*>(bytes);
    entries[std::string(ns) + "/" + key] = std::vector<uint8_t>(data, data + length);
    if (!strcmp(ns, "kingdom") || !strcmp(ns, "ordinary")) gameWrites++;
    return true;
  }
  void erase(const char* ns, const char* key) override { entries.erase(std::string(ns) + "/" + key); }
  Save campaign(uint8_t profile = 0) {
    Save saved{}; uint8_t bytes[shell::BLOB_MAX];
    const size_t length = load("kingdom", shell::key('s', profile), bytes, sizeof bytes);
    assert(kingdom::loadSave(saved, bytes, length, profile)); return saved;
  }
};
struct Ordinary : App {
  uint8_t byte = 1; bool dirty = true, urgent = false, churn = false;   // churn: something to save every frame
  const char* name() const override { return "Ordinary"; }
  const char* store() const override { return "ordinary"; }
  const gfx::Sprite& icon() const override { return kingdom::art::icon(); }
  void enter(const AppEnter&) override { dirty = true; }
  void update(uint32_t, uint32_t, const Input&) override { if (churn) { byte++; dirty = true; } }
  void render() override {}
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void** data, size_t* length, bool allowed) override {
    if (!allowed || !dirty) return false;
    *data = &byte; *length = 1; dirty = false; return true;
  }
  bool saveUrgent() const override { return urgent && dirty; }
  bool wantsHome() override { return false; }
  void leave() override { dirty = true; }
  const char* screenName() const override { return "ordinary"; }
  void debugPrint() override {}
  void debugCmd(const char*) override {}
};
struct Session {
  Memory store;
  Shell shell;
  kingdom::Game game;
  App* apps[1] = {&game};
  InputTracker input;
  uint32_t ms = 0;
  void step(bool down = false, int x = 0, int y = 0, uint32_t delta = 20) {
    ms += delta; shell.update(NOW + ms / 1000, ms, input.step(down, x, y, ms));
  }
  void tap(const ui::Box& box) {
    const int x = box.x + box.w / 2, y = box.y + box.h / 2;
    step(false, 0, 0, 500); step(true, x, y); step(false, x, y, 40);
  }
  void begin() {
    shell.begin(store, apps, 1); step(); assert(shell.createProfile("Reader", 9, "") == 0);
    assert(shell.openApp("Kingdom")); step();
  }
  void settle() { step(false, 0, 0, 1000); }   // past the urgent checkpoint's one-second gap
  void cadence() { step(false, 0, 0, 5100); }   // past the five-second cadence: what changed is written
  void start() { begin(); tap(kingdom::layout::START); cadence(); }   // the new campaign lands first
};
void sameCursor(const kingdom::UiCursor& a, const kingdom::UiCursor& b) { assert(!memcmp(&a, &b, sizeof a)); }
// Pages turned are one write at the cadence, not one a page; what was written resumes after an abrupt restart.
void pageCheckpoint() {
  Session s; s.start(); const int writes = s.store.gameWrites;
  s.tap(kingdom::layout::NEXT); s.settle(); s.tap(kingdom::layout::NEXT); s.settle();
  assert(s.store.gameWrites == writes && s.store.campaign().cursor.page == 0);   // not a flash write a page
  s.cadence(); assert(s.store.gameWrites == writes + 1);
  const Save saved = s.store.campaign(); sameCursor(saved.cursor, s.game.save().cursor); assert(saved.cursor.page == 2);
  // A new shell represents an abrupt restart: leave() is deliberately never called on the old shell.
  Shell rebooted; kingdom::Game resumed; App* apps[] = {&resumed};
  rebooted.begin(s.store, apps, 1); rebooted.update(NOW, 10, {});
  Input pick{}; pick.x = pick.downX = 80; pick.y = pick.downY = 67; pick.tap = true;
  rebooted.update(NOW, 600, pick); assert(rebooted.openApp("Kingdom"));
  InputTracker fingers; resumed.update(NOW, 1200, fingers.step(true, 80, 126, 1200));
  resumed.update(NOW, 1250, fingers.step(false, 80, 126, 1250));
  assert(!strcmp(resumed.screenName(), "lk_read")); sameCursor(resumed.save().cursor, saved.cursor);
}
void urgentAtomicDecision() {
  Session s; s.start();
  for (int n = 0; s.game.save().cursor.mode == kingdom::READ && n < 64; n++) s.tap(kingdom::layout::NEXT);
  assert(s.game.save().cursor.mode == kingdom::CHOICE);
  s.tap(kingdom::layout::CARD0);
  for (int n = 0; s.game.save().cursor.mode == kingdom::CONFIRM && n < 64; n++) s.tap(kingdom::layout::NEXT);
  assert(s.game.save().cursor.mode == kingdom::RESULT);
  s.settle(); const Save saved = s.store.campaign();   // within a second of the decision, well inside the cadence
  sameCursor(saved.cursor, s.game.save().cursor);
  assert(saved.journalCount == 1);
  const kingdom::Scene* scene = kingdom::currentScene(saved); assert(scene);
  assert(kingdom::hasDecision(saved, scene->choices[saved.cursor.selectedChoice].id));
  assert(s.ms < 30000 && !s.game.saveUrgent());
}
void sessionBreakCursor() {
  Session s; const shell::Limits limits{1, 25}; s.store.save("sys", "limits", &limits, sizeof limits); s.start();
  s.tap(kingdom::layout::NEXT); s.cadence(); const Save before = s.store.campaign(); assert(before.cursor.page == 1);
  for (int n = 0; n < 65 && strcmp(s.shell.screenName(), "rest"); n++) s.step(true, 0, 0, 1000);
  assert(!strcmp(s.shell.screenName(), "rest")); const Save paused = s.store.campaign();
  sameCursor(paused.cursor, before.cursor); assert(paused.day == before.day && paused.journalCount == before.journalCount);
  kingdom::Game resumed; Profile profile{0, "Reader", 0, 9, 1}; SaveSlot slot{&paused, sizeof paused};
  resumed.enter({&profile, &profile, &slot, 1, NOW + 86400, 0, true});
  sameCursor(resumed.save().cursor, before.cursor); assert(resumed.save().seed == before.seed);
}
void quitFromDetails() {
  Session s; s.start(); s.tap(kingdom::layout::NEXT); s.cadence(); const Save original = s.store.campaign();
  s.tap(kingdom::layout::MENU); s.tap(kingdom::layout::TOOL0);
  assert(s.game.save().cursor.mode == kingdom::PARTY_OVERVIEW); s.tap(kingdom::layout::PARTY1);
  assert(s.game.save().cursor.mode == kingdom::PARTY); s.tap(kingdom::layout::MENU);
  assert(s.game.save().cursor.mode == kingdom::MENU); s.tap(kingdom::layout::TOOL3);
  assert(!strcmp(s.shell.screenName(), "launcher")); const Save saved = s.store.campaign();
  sameCursor(saved.cursor, original.cursor); assert(saved.day == original.day && saved.journalCount == original.journalCount);
  assert(s.shell.openApp("Kingdom")); s.tap(kingdom::layout::START);
  sameCursor(s.game.save().cursor, original.cursor);
}
void ordinaryCadence() {
  Memory store; Shell shell; Ordinary ordinary; App* apps[] = {&ordinary};
  assert(!ordinary.saveUrgent()); shell.begin(store, apps, 1); shell.update(NOW, 10, {});
  assert(shell.createProfile("Other", 8, "") == 0 && shell.openApp("Ordinary"));
  shell.update(NOW + 1, 1000, {}); assert(store.gameWrites == 0);
  shell.update(NOW + 5, 5010, {}); assert(store.gameWrites == 0);
  shell.update(NOW + 5, 5011, {}); assert(store.gameWrites == 1);
  ordinary.dirty = true; shell.update(NOW + 6, 6000, {}); assert(store.gameWrites == 1);
}
// A game that is always urgent is still written at most once a second; one that only changes its view is not written.
void urgentThrottled() {
  Memory store; Shell shell; Ordinary ordinary; App* apps[] = {&ordinary};
  ordinary.urgent = ordinary.churn = true; shell.begin(store, apps, 1); shell.update(NOW, 10, {});
  assert(shell.createProfile("Other", 8, "") == 0 && shell.openApp("Ordinary"));
  for (uint32_t ms = 20; ms <= 3000; ms += 20) shell.update(NOW + ms / 1000, ms, {});
  assert(store.gameWrites >= 2 && store.gameWrites <= 3);
  // Kingdom marks its save dirty on every view change; reopening and Continue change no byte, so nothing is written.
  Session s; s.start(); s.tap(kingdom::layout::NEXT); s.settle();
  s.tap(kingdom::layout::MENU); s.tap(kingdom::layout::TOOL3); assert(!strcmp(s.shell.screenName(), "launcher"));
  const int writes = s.store.gameWrites;
  assert(s.shell.openApp("Kingdom")); s.tap(kingdom::layout::START); s.settle(); s.step(false, 0, 0, 6000);
  assert(!strcmp(s.game.screenName(), "lk_read") && s.store.gameWrites == writes);
}
// A refused write stays pending in the shell: it lands at the next cadence once the store works again, though the
// game has nothing new to hand out.
void failedSaveRetried() {
  Memory store; Shell shell; Ordinary ordinary; App* apps[] = {&ordinary};
  shell.begin(store, apps, 1); shell.update(NOW, 10, {});
  assert(shell.createProfile("Other", 8, "") == 0 && shell.openApp("Ordinary"));
  store.fail = true; shell.update(NOW + 5, 5011, {}); assert(store.gameWrites == 0 && !ordinary.dirty);
  store.fail = false; shell.update(NOW + 6, 6000, {}); assert(store.gameWrites == 0);   // the cadence, not every frame
  shell.update(NOW + 10, 10012, {}); assert(store.gameWrites == 1 && store.load("ordinary", "s0", &ordinary.byte, 1) == 1);
}
// A board updated from version 1 (host/fixtures: a real stored save): the campaign opens exactly as it was left and the
// next save writes it as version 2. A write refused on the way leaves the version 1 blob whole, and it lands later.
void versionOneMigrates() {
  Session s; s.shell.begin(s.store, s.apps, 1); s.step();
  assert(s.shell.createProfile("Ann", 9, "") == 0 && s.shell.createProfile("Ben", 9, "") == 1 && s.shell.createProfile("Cy", 9, "") == 2);
  s.store.save("kingdom", "s2", KINGDOM_SAVE_V1, sizeof KINGDOM_SAVE_V1);
  const std::vector<uint8_t>& stored = s.store.entries["kingdom/s2"];
  s.store.fail = true; assert(s.shell.openApp("Kingdom")); s.step();
  assert(!memcmp(&s.game.save(), KINGDOM_SAVE_V1, sizeof(Save)));
  s.step(false, 0, 0, 6000); s.step();
  assert(stored.size() == sizeof(Save) && !memcmp(stored.data(), KINGDOM_SAVE_V1, sizeof(Save)));   // the only copy, untouched
  s.store.fail = false;
  for (int n = 0; n < 400 && stored.size() == sizeof(Save); n++) s.step(false, 0, 0, 1000);
  assert(stored.size() < 300 && stored[4] == kingdom::SAVE_VERSION);
  const Save migrated = s.store.campaign(2); assert(!memcmp(&migrated, KINGDOM_SAVE_V1, sizeof migrated));   // every field
  s.tap(kingdom::layout::START); assert(!strcmp(s.game.screenName(), "lk_journal"));   // where the kid left it
  // After a restart it loads from version 2, the same campaign.
  Shell rebooted; kingdom::Game resumed; App* apps[] = {&resumed};
  rebooted.begin(s.store, apps, 1); rebooted.update(NOW, 10, {});
  Input pick{}; pick.x = pick.downX = 80; pick.y = pick.downY = 93; pick.tap = true;   // the third name on the picker
  rebooted.update(NOW, 600, pick); assert(rebooted.openApp("Kingdom"));
  assert(!memcmp(&resumed.save(), KINGDOM_SAVE_V1, sizeof(Save)));
}
} // namespace
int main() {
  static_assert(sizeof(Save) <= shell::BLOB_MAX, "the shell must hold a complete campaign checkpoint");
  pageCheckpoint(); urgentAtomicDecision(); sessionBreakCursor(); quitFromDetails(); ordinaryCadence();
  urgentThrottled(); failedSaveRetried(); versionOneMigrates();
  puts("kingdom shell: pages at the cadence, immediate decisions, abrupt restart, exact break resume and ordinary save cadence passed");
}
