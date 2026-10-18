// Live aircraft around the device's spot, for Plane Radar (games/plane-radar/). The firmware feeds it from adsb.fi over
// the saved wifi (firmware/radar.cpp), the sim and the tests from canned planes. The spot (where the device is) is one
// per device, kept by the feed (firmware: NVS "sys"); planes come as offsets from it. Header-only.
#pragma once
#include <stdint.h>

namespace air {

constexpr int MAX_PLANES = 48;
constexpr int32_t NO_ALT = INT32_MIN;   // altitude unknown

enum Kind : uint8_t { K_AIRLINER, K_SMALL, K_MILITARY };   // small: ADS-B category A1/A2 (light and small planes)

struct Plane {
  float eastKm, northKm;   // from the spot, true north up
  float trackDeg;          // where it is going, true degrees clockwise from north
  uint16_t knots;          // ground speed
  int32_t altFt;           // NO_ALT when unknown; 0 = on the ground
  char call[9];            // callsign (flight number), "" when it sends none
  char route[8];           // "SEA-SFO" (IATA airports, firmware: adsbdb.com), "" if unknown
  Kind kind;
};

enum State : uint8_t {
  S_OFF,       // nothing asked for it yet (or no feed on this build)
  S_NO_SPOT,   // no spot set yet (a grown-up sets it)
  S_NO_WIFI,   // no network saved, or it would not connect
  S_JOINING,   // waiting for the radio, joining wifi, or before the first answer
  S_LIVE,      // planes from a fetch in the last STALE_MS
  S_LOST,      // joined, but no good answer for STALE_MS: the scope stays empty rather than frozen
  S_COUNT,
};

constexpr uint32_t STALE_MS = 15000;

struct Snapshot {
  State state = S_OFF;
  uint32_t gen = 0;      // +1 on every good fetch: the planes changed
  int n = 0;
  Plane planes[MAX_PLANES];
};

class Feed {
 public:
  // Starts (or keeps) fetching planes within radiusKm of the spot; a second call only changes the radius.
  virtual void start(float radiusKm) = 0;
  virtual void stop() = 0;                // the radio goes back to the updater
  virtual void read(Snapshot& out) = 0;   // a consistent copy, any time
  virtual bool spot(double* lat, double* lon) = 0;   // false when none is set

 protected:
  ~Feed() = default;
};

// The one feed, set by the firmware or the sim before the shell starts; nullptr reads as S_OFF.
inline Feed*& feedSlot() { static Feed* f = nullptr; return f; }
inline void setFeed(Feed* f) { feedSlot() = f; }
inline Feed* feed() { return feedSlot(); }

}  // namespace air
