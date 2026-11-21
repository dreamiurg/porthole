// The contract between the Porthole shell and a game. Games include os/ headers only; the shell knows a game
// only through this interface and the static app array in the firmware/sim entry point.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "gfx.h"
#include "input.h"
#include "palette.h"
#include "skills.h"

constexpr int MAX_PROFILES = 4;   // one persisted record per profile: raising this needs no migration

struct Profile { uint8_t id; char name[12]; uint8_t avatar, age, muted; };   // what games see
struct SaveSlot { const void* data; size_t len; };                             // len 0 = none

// The red "new" dot on a button inside a game (ui::newDot), for news an update brought (news.toml, "Kid news" in
// AGENTS.md). A game asks with the spot's name from its news.toml entry, isNew("together"), and says seen() when the
// kid taps that button; the shell keeps it per kid. tools/news.py checks the game calls isNew with that literal.
class NewSpots {
 public:
  virtual bool isNew(const char* spot) const = 0;
  virtual void seen(const char* spot) = 0;

 protected:
  ~NewSpots() = default;
};

// Everything enter() gets, bundled because the complexity gate caps functions at 5 parameters.
// saves[i] belongs to all[i]; a game may read other profiles' saves (Paw Street) but only writes who's.
// daylight: the kid picked a brightness on the launcher, so the game draws day whatever the clock says (tint() returns
// TINT_DAY, no night scenes); false is Auto, the light follows the clock. A game's own night (a pet asleep) still holds.
// stars: who's star balance (os/stars.h), kept and saved by the shell; nullptr when there is none (tests).
// avatars: the shell's avatar pictures, Profile::avatar indexing them (modulo avatarCount), for a game that shows who
// is playing (Rally's race together); nullptr in tests.
// news: who's new dots inside this game (NewSpots); nullptr in tests.
struct AppEnter {
  const Profile* who; const Profile* all; const SaveSlot* saves; int n; uint32_t nowSec, ms; bool daylight;
  uint32_t* stars;
  const gfx::Sprite* avatars = nullptr;
  int avatarCount = 0;
  skills::Book* book = nullptr;   // the kid's skill ratings (os/skills.h); null in tests that need none
  NewSpots* news = nullptr;
};

// What render() draws on: the 160x160 indexed framebuffer (gfx::fb, upscaled 3x through the tint's palette) or the
// full 480x480 RGB565 panel (gfx565::fb, os/gfx565.h). An RGB565 render() must leave the whole target correct, knowing
// the target is one of the panel's two buffers, and every present flips them, an indexed frame's too: the target holds
// the frame from two presents ago. An indexed present (the game's own or the shell's) overwrites a buffer with its
// upscaled frame, so after any indexed present, or right after enter(), treat both buffers as unknown. Painting every
// pixel is always right; a game may instead remember what each buffer holds and write only what changed. The sim
// emulates this: two buffers, flipped on every frame, magenta until first drawn.
enum Surface : uint8_t { SURFACE_INDEXED, SURFACE_RGB565 };

class App {
 public:
  virtual const char* name() const = 0;          // launcher label
  virtual const gfx::Sprite& icon() const = 0;   // launcher icon, from the game's own sprites
  virtual const char* store() const = 0;         // NVS namespace; keys are "s<profile id>"
  virtual void enter(const AppEnter& e) = 0;
  virtual void update(uint32_t nowSec, uint32_t ms, const Input& in) = 0;
  virtual void render() = 0;
  virtual Surface surface() const { return SURFACE_INDEXED; }   // polled every frame: a game may switch per screen
  virtual Tint tint() const = 0;
  virtual bool asleep() const = 0;               // backlight dimming policy
  virtual bool soundOn(uint32_t ms) = 0;         // the shell gates this with the profile's mute
  // true when there is something to write for `who`. The shell keeps what is stored instead when that is this game's
  // save from a newer firmware (os/save.h: the first eight bytes are magic, version, size; a crc32 of all before it
  // ends the blob), and never erases a save: *len 0 writes nothing.
  virtual bool takeSave(const void** data, size_t* len, bool allowed) = 0;
  // A moment that must survive a power cut (a story's decision, Pets Club's farm and adoption) asks for a checkpoint
  // within a second. Otherwise the five-second cadence holds: flash space is small, so not for every tap or page.
  virtual bool saveUrgent() const { return false; }
  virtual bool wantsHome() = 0;                  // back pressed on the game's top screen (reported once)
  virtual void leave() = 0;                      // flush state; next takeSave returns the final save
  // Test and serial hooks: the sim's `debug`/`screen`/`dbg` and the firmware's `S` reach the active game.
  virtual const char* screenName() const = 0;
  virtual void debugPrint() = 0;
  virtual void debugCmd(const char* cmd) = 0;

 protected:
  ~App() = default;   // apps are built once by psram::make (games/apps.h) and never deleted
};
