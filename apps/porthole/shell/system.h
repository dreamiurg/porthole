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
  UP_COUNT,        // not a state: the number of them (tables indexed by UpdState)
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

struct Network {   // one a scan found
  char ssid[33] = {0};
  int8_t rssi = 0;              // dBm, strongest near 0
  bool secured = false;         // needs a password
};
constexpr int MAX_NETWORKS = 12;   // the most a scan reports

// Adds one scan result to list (n entries), which stays strongest first with one entry per name, its strongest: a mesh
// or a dual-band router shows one name several times. A hidden network (no name) or one with a name over 32 bytes is
// skipped; past MAX_NETWORKS the weakest falls off. Returns the new count.
inline int addNetwork(Network (&list)[MAX_NETWORKS], int n, const char* ssid, int rssi, bool secured) {
  const int max = MAX_NETWORKS;
  size_t len = ssid ? strlen(ssid) : 0;
  if (!len || len > 32) return n;
  rssi = rssi < -127 ? -127 : rssi > 0 ? 0 : rssi;
  for (int i = 0; i < n; i++) {
    if (strcmp(list[i].ssid, ssid)) continue;
    if (list[i].rssi >= rssi) return n;   // a weaker copy of one listed
    memmove(list + i, list + i + 1, (size_t)(n - i - 1) * sizeof *list);   // a stronger one: listed again below
    n--;
    break;
  }
  int at = 0;
  while (at < n && list[at].rssi >= rssi) at++;
  if (at >= max) return n;
  if (n == max) n--;
  memmove(list + at + 1, list + at, (size_t)(n - at) * sizeof *list);
  memcpy(list[at].ssid, ssid, len + 1);
  list[at].rssi = (int8_t)rssi; list[at].secured = secured;
  return n + 1;
}

class System {
 public:
  virtual void info(SysInfo& out) = 0;
  virtual UpdStatus status() = 0;
  virtual void checkNow() = 0;              // forced check + download in the background, ignoring once-a-day; returns at once
  virtual void restart() = 0;               // reboot; starts a READY update
  virtual Channel channel() = 0;
  // Persisted; takes effect at the next check. Implementations clear `available` and return to UP_IDLE (unless busy
  // or READY): the old channel's result says nothing about the new one.
  virtual void setChannel(Channel c) = 0;

  // Wifi setup (the menu's Wifi screens). The radio does one job at a time, in the background (the firmware's core 0):
  // the update check and download, a scan or a test join. startScan and startTest refuse (false, nothing started)
  // while another job runs; checkNow and the daily check do nothing while a scan or a test runs. Wifi is off after each.
  virtual bool startScan() = 0;
  // -1 while a scan runs; else the last scan's networks into out (up to max, as addNetwork keeps them): 0 = none
  // found, the scan failed, or none yet.
  virtual int scanResult(Network* out, int max) = 0;
  // Joins ssid with pass ("" = an open network) without saving it, then turns wifi off again.
  virtual bool startTest(const char* ssid, const char* pass) = 0;
  virtual int testResult() = 0;   // -1 while joining; 1 it joined; 0 it could not (or no test yet)
  // The network the daily check joins, persisted; false (nothing saved) if ssid is empty or over 32 bytes, or pass
  // over 64. pass "" = an open network. Never logged.
  virtual bool saveWifi(const char* ssid, const char* pass) = 0;

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
