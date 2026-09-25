// The contract between the Porthole shell and a game. Games include os/ headers only; the shell knows a game
// only through this interface and the static app array in the firmware/sim entry point.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "gfx.h"
#include "input.h"
#include "palette.h"

constexpr int MAX_PROFILES = 4;   // one persisted record per profile: raising this needs no migration

struct Profile { uint8_t id; char name[12]; uint8_t avatar, age, muted; };   // what games see
struct SaveSlot { const void* data; size_t len; };                             // len 0 = none

// Everything enter() gets, bundled because the complexity gate caps functions at 5 parameters.
// saves[i] belongs to all[i]; a game may read other profiles' saves (Paw Street) but only writes who's.
struct AppEnter { const Profile* who; const Profile* all; const SaveSlot* saves; int n; uint32_t nowSec, ms; };

// What render() draws on: the 160x160 indexed framebuffer (gfx::fb, upscaled 3x through the tint's palette) or the
// full 480x480 RGB565 panel (gfx565::fb, os/gfx565.h). An RGB565 render() paints every pixel: the target is one of the
// panel's two buffers and still holds an older frame.
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
  // true when there is something to write for `who`; *len 0 means erase `who`'s save.
  virtual bool takeSave(const void** data, size_t* len, bool allowed) = 0;
  virtual bool wantsHome() = 0;                  // back pressed on the game's top screen (reported once)
  virtual void leave() = 0;                      // flush state; next takeSave returns the final save
  // Test and serial hooks: the sim's `debug`/`screen`/`dbg` and the firmware's `S` reach the active game.
  virtual const char* screenName() const = 0;
  virtual void debugPrint() = 0;
  virtual void debugCmd(const char* cmd) = 0;

 protected:
  ~App() = default;   // apps are static objects, never deleted through the interface
};
