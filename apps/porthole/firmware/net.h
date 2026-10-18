// Wireless updates on the board: shell::System over wifi, HTTPS to GitHub releases and the spare OTA slot.
// Design: docs/superpowers/specs/2026-10-17-wireless-updates-design.md. Settings live in NVS namespace "sys".
#pragma once
#include <stdint.h>
#include "air.h"
#include "system.h"

#ifndef PORTHOLE_VERSION
#define PORTHOLE_VERSION "dev"   // a local build; release CI passes -DPORTHOLE_VERSION="X.Y.Z"
#endif

namespace net {

class Updater final : public shell::System {
 public:
  // localNow: the firmware's local wall-clock seconds (main.cpp nowSec()), first called a minute later. Loads the "sys"
  // settings, mounts the optional SD card and takes /porthole/wifi.txt from it. Call once, after board::init() and
  // before Shell::begin: info() reads what this loads.
  void begin(uint32_t (*localNow)());
  // Call every loop: once a minute (the first a minute after boot) it starts the daily check if one is due.
  void tick(uint32_t ms);
  // The daily run writes flash only while the screen is dark: a flash write stalls the RGB panel's refill from PSRAM,
  // and a lit screen shows a broken frame or two. Call setDark every loop with whether the backlight is off. The daily
  // download starts only in the dark and stops (to resume the next dark spell) if the screen wakes. A Check now run
  // keeps going with the screen lit: the grown-up asked for it and watches its progress.
  void setDark(bool dark);

  void info(shell::SysInfo& out) override;
  shell::UpdStatus status() override;
  void checkNow() override;
  bool tryCheckNow();   // checkNow, false when it was dropped (a run or a wifi job holds the radio, or an update is ready)
  void restart() override;
  shell::Channel channel() override;
  void setChannel(shell::Channel c) override;
  bool startScan() override;
  int scanResult(shell::Network* out, int max) override;
  bool startTest(const char* ssid, const char* pass) override;
  int testResult() override;
  bool saveWifi(const char* ssid, const char* pass) override;   // also serial W and the SD card's wifi.txt
  // Home ("sys"/home), for Radar: the lookup is a radio job like a scan, sending the address once to Nominatim.
  bool hasHome() override { return true; }
  bool home(shell::Home& out) override;
  bool saveHome(const shell::Home& h) override;   // also serial L
  bool startLookup(const char* address) override;
  int lookupResult(shell::Home& out) override;
};

// Radar's planes from adsb.fi over the saved wifi, as one more radio job: while started (and the screen is not dark)
// a core-0 task holds the radio, polls every 5 s and drops it on stop(), on the dark (so the daily update runs) or when
// it can't join. Lives here because it shares the radio's one-job machinery with the updater.
class Radar final : public air::Feed {
 public:
  void start(float radiusKm) override;
  void stop() override;
  void read(air::Snapshot& out) override;
  bool spot(double* lat, double* lon) override;
};

}  // namespace net
