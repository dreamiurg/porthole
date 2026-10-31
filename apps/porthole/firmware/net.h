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
  // utcNow: the firmware's canonical UTC seconds, called on the main core only. Loads the "sys"
  // settings, mounts the optional SD card and takes /porthole/wifi.txt from it. Call once, after board::init() and
  // before Shell::begin: info() reads what this loads.
  void begin(uint32_t (*utcNow)());
  // Call every loop: clock sync soon after boot and every six hours, independent of screen/build version;
  // once a minute (the first a minute after boot) it also starts the daily update check if one is due.
  void tick(uint32_t ms);
  // Daily checks start in the dark and stop on activity. Explicit checks wait for the loop to install an
  // SRAM-backed display before any flash writes; it remains installed through all cleanup. A daily check
  // respects the idle backlight; only a manual check keeps its progress lit.
  void setDark(bool dark);
  bool progressRequested();
  bool manualCheck();
  void progressReady(bool ok);
  void cancelCheck();
  bool cancelling();
  bool committing();
  // Consume a worker sample on the main core only; includes time elapsed since it was received.
  bool takeClock(uint32_t& utcEpoch);
  shell::ClockStatus clockStatus() override;
  bool syncClock() override;
  bool setUtcOffset(int16_t minutes) override;
  bool setTimeZone(const char* iana, bool automatic) override;
  bool detectTimeZone() override;

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
 private:
  void applyZoneResult();
  bool storeTimeZone(const char* iana, bool automatic, bool detected);
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
