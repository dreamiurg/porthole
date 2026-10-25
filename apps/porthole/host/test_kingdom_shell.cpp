// Shared-shell integration: immediate story checkpoints and the unchanged ordinary-game cadence.
#include "host/check.h"
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include "games/living-kingdom/game.h"
#include "games/living-kingdom/layout.h"
#include "shell.h"

namespace {
using kingdom::Save;
constexpr uint32_t NOW = 1790000000;
struct Memory : shell::Store {
  std::map<std::string, std::vector<uint8_t>> entries;
  int gameWrites = 0;
  size_t load(const char* ns, const char* key, void* out, size_t max) override {
    auto i = entries.find(std::string(ns) + "/" + key);
    if (i == entries.end() || i->second.size() > max) return 0;
    memcpy(out, i->second.data(), i->second.size()); return i->second.size();
  }
  void save(const char* ns, const char* key, const void* bytes, size_t length) override {
    assert(length <= shell::BLOB_MAX);
    const auto* data = static_cast<const uint8_t*>(bytes);
    entries[std::string(ns) + "/" + key] = std::vector<uint8_t>(data, data + length);
    if (!strcmp(ns, "kingdom") || !strcmp(ns, "ordinary")) gameWrites++;
  }
  void erase(const char* ns, const char* key) override { entries.erase(std::string(ns) + "/" + key); }
  Save campaign(uint8_t profile = 0) {
    Save saved{}; uint8_t bytes[shell::BLOB_MAX];
    const size_t length = load("kingdom", shell::key('s', profile), bytes, sizeof bytes);
    assert(kingdom::loadSave(saved, bytes, length, profile)); return saved;
  }
};
struct Ordinary : App {
  uint8_t byte = 1; bool dirty = true;
  const char* name() const override { return "Ordinary"; }
  const char* store() const override { return "ordinary"; }
  const gfx::Sprite& icon() const override { return kingdom::art::icon(); }
  void enter(const AppEnter&) override { dirty = true; }
  void update(uint32_t, uint32_t, const Input&) override {}
  void render() override {}
  Tint tint() const override { return TINT_DAY; }
  bool asleep() const override { return false; }
  bool soundOn(uint32_t) override { return false; }
  bool takeSave(const void** data, size_t* length, bool allowed) override {
    if (!allowed || !dirty) return false;
    *data = &byte; *length = 1; dirty = false; return true;
  }
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
  void start() { begin(); tap(kingdom::layout::START); }
};
void sameCursor(const kingdom::UiCursor& a, const kingdom::UiCursor& b) { assert(!memcmp(&a, &b, sizeof a)); }
void urgentPageCheckpoint() {
  Session s; s.start(); const int writes = s.store.gameWrites;
  const uint32_t before = s.ms; s.tap(kingdom::layout::NEXT);
  assert(s.ms - before < 5000 && s.store.gameWrites == writes + 1);
  const Save saved = s.store.campaign(); sameCursor(saved.cursor, s.game.save().cursor); assert(saved.cursor.page == 1);
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
  const Save saved = s.store.campaign(); sameCursor(saved.cursor, s.game.save().cursor);
  assert(saved.journalCount == 1);
  const kingdom::Scene* scene = kingdom::currentScene(saved); assert(scene);
  assert(kingdom::hasDecision(saved, scene->choices[saved.cursor.selectedChoice].id));
  assert(s.ms < 30000 && !s.game.saveUrgent());
}
void sessionBreakCursor() {
  Session s; const shell::Limits limits{1, 25}; s.store.save("sys", "limits", &limits, sizeof limits); s.start();
  s.tap(kingdom::layout::NEXT); const Save before = s.store.campaign(); assert(before.cursor.page == 1);
  for (int n = 0; n < 65 && strcmp(s.shell.screenName(), "rest"); n++) s.step(true, 0, 0, 1000);
  assert(!strcmp(s.shell.screenName(), "rest")); const Save paused = s.store.campaign();
  sameCursor(paused.cursor, before.cursor); assert(paused.day == before.day && paused.journalCount == before.journalCount);
  kingdom::Game resumed; Profile profile{0, "Reader", 0, 9, 1}; SaveSlot slot{&paused, sizeof paused};
  resumed.enter({&profile, &profile, &slot, 1, NOW + 86400, 0, true});
  sameCursor(resumed.save().cursor, before.cursor); assert(resumed.save().seed == before.seed);
}
void quitFromDetails() {
  Session s; s.start(); s.tap(kingdom::layout::NEXT); const Save original = s.store.campaign();
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
} // namespace
int main() {
  static_assert(sizeof(Save) <= shell::BLOB_MAX, "the shell must hold a complete campaign checkpoint");
  urgentPageCheckpoint(); urgentAtomicDecision(); sessionBreakCursor(); quitFromDetails(); ordinaryCadence();
  puts("kingdom shell: immediate pages/decisions, abrupt restart, exact break resume and ordinary save cadence passed");
}
