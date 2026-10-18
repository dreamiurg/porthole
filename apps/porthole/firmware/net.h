// Wireless updates on the board: shell::System over wifi, HTTPS to GitHub releases and the spare OTA slot.
// Design: docs/superpowers/specs/2026-10-17-wireless-updates-design.md. Settings live in NVS namespace "sys".
#pragma once
#include <stdint.h>
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
  bool saveWifi(const char* ssid, const char* pass);   // false if ssid is empty or either is too long
  // Flash is only written while the screen is dark: a flash write stalls the RGB panel's refill from PSRAM, and a lit
  // screen shows broken frames. Call setDark every loop with whether the backlight is off. The daily download starts
  // only in the dark and stops (to resume the next dark spell) if the screen wakes. A Check now run keeps going, start
  // to finish (joining wifi writes calibration data to flash too): while forcesDark() is true the backlight stays off.
  void setDark(bool dark);
  bool forcesDark();

  void info(shell::SysInfo& out) override;
  shell::UpdStatus status() override;
  void checkNow() override;
  void restart() override;
  shell::Channel channel() override;
  void setChannel(shell::Channel c) override;
};

}  // namespace net
