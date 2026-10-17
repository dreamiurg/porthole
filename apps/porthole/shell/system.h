// The device side of wireless updates, as the shell sees it: the hidden system menu reads status and starts a check,
// the "something new" card compares versions. Firmware implements it over wifi/OTA/NVS (firmware/net.cpp), the
// host sim and tests with canned states. Design: docs/superpowers/specs/2026-10-17-wireless-updates-design.md.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace shell {

enum Channel : uint8_t { CH_STABLE, CH_BETA };

// Where the background check/download stands. Every state but CHECKING and DOWNLOADING is a resting one.
enum UpdState : uint8_t {
  UP_IDLE,         // nothing tried since boot
  UP_NO_WIFI,      // no network saved, or it would not connect
  UP_CHECKING,     // asking GitHub for the newest version on the channel
  UP_CURRENT,      // the running version is the newest (or newer: never downgrades)
  UP_DOWNLOADING,  // writing the new image into the spare slot; pct is meaningful
  UP_READY,        // downloaded and verified; it starts on the next power-on (or restart())
  UP_FAILED,       // the check or download failed; the running firmware is untouched
};

struct UpdStatus {
  UpdState state = UP_IDLE;
  uint8_t pct = 0;              // UP_DOWNLOADING only, 0..100
  char available[16] = {0};     // newest version seen on the channel, "" if never checked
  uint32_t lastCheck = 0;       // local epoch seconds of the last attempt, 0 = never
};

struct SysInfo {
  const char* version = "dev";  // PORTHOLE_VERSION, "dev" for local builds
  char slot[8] = {0};           // running app partition, "ota_0" / "ota_1"
  char reset[16] = {0};         // last reset reason, e.g. "power-on", "panic"
  uint32_t heapFree = 0;        // internal heap, bytes
  bool sd = false;              // a card is mounted
  bool wifiSaved = false;
  char ssid[33] = {0};          // saved network, "" if none
};

class System {
 public:
  virtual void info(SysInfo& out) = 0;
  virtual UpdStatus status() = 0;
  virtual void checkNow() = 0;              // forced check + download in the background, ignoring once-a-day; returns at once
  virtual void restart() = 0;               // reboot; starts a READY update
  virtual Channel channel() = 0;
  virtual void setChannel(Channel c) = 0;   // persisted; takes effect at the next check

 protected:
  ~System() = default;
};

// Parses "X.Y.Z" (an optional leading 'v' is skipped). False for "dev" or anything else.
inline bool parseVersion(const char* s, int v[3]) {
  if (!s) return false;
  if (*s == 'v') s++;
  for (int i = 0; i < 3; i++) {
    if (*s < '0' || *s > '9') return false;
    char* end; long n = strtol(s, &end, 10);
    v[i] = (int)n; s = end;
    if (i < 2) { if (*s != '.') return false; s++; }
  }
  return *s == 0;
}

// True when a is a strictly newer release than b. A dev build (b unparsable) is never offered anything, and an
// unparsable a is never newer: devices only move forward between real releases.
inline bool newer(const char* a, const char* b) {
  int x[3], y[3];
  if (!parseVersion(a, x) || !parseVersion(b, y)) return false;
  for (int i = 0; i < 3; i++) if (x[i] != y[i]) return x[i] > y[i];
  return false;
}

}  // namespace shell
