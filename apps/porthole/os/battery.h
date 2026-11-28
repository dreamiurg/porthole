// The battery, from its voltage alone: GPIO4 is the only reading the board has (no charge-status line, no USB-power
// line, no current sensor; docs/superpowers/specs/2026-11-28-battery-design.md). Charging and time left are inferred
// from how the voltage moves, and stay off or unknown without clear evidence. Header-only; host/test_battery.cpp.
#pragma once
#include <stdint.h>

namespace battery {

struct Reading {
  uint16_t mv = 0;            // 0 = no reading
  uint8_t pct = 0;            // from the open-circuit curve: an estimate, a little low under load
  bool charging = false;
  int16_t minutesLeft = -1;   // -1 = unknown
  bool present() const { return mv >= 3000; }
};

// A typical Li-ion open-circuit curve, linear between its points.
inline uint8_t percent(uint16_t mv) {
  static const uint16_t MV[] = {3270, 3610, 3690, 3710, 3730, 3750, 3770, 3790, 3800, 3820, 3840,
                                3850, 3870, 3910, 3950, 3980, 4020, 4080, 4110, 4150, 4200};
  const int n = sizeof MV / sizeof MV[0];   // 0 %, then every 5 %
  if (mv <= MV[0]) return 0;
  if (mv >= MV[n - 1]) return 100;
  int i = 1;
  while (MV[i] < mv) i++;
  return (uint8_t)((i - 1) * 5 + 5 * (mv - MV[i - 1]) / (MV[i] - MV[i - 1]));
}

// Fed a smoothed reading every few seconds with UTC seconds (millis() drifts through light sleep; the RTC does not).
// shortcut: thresholds guessed from the datasheets and one plug-in seen on the board; tune them from PostHog's battery events.
class Gauge {
 public:
  static constexpr int JUMP_MV = 60;          // a plug-in or unplug between two samples under GAP_S apart
  static constexpr uint32_t GAP_S = 60;       // longer (a sleep) is never a jump
  static constexpr int RISE_MV = 20, FALL_MV = 10;   // over TREND_MIN minutes: a slow charge, or a discharge
  static constexpr int TREND_MIN = 10;
  static constexpr uint16_t FLOAT_MV = 4170;  // the charger holds the cell here: charging unless a fall says otherwise
  static constexpr int LEFT_MIN = 20;         // minutes of steady fall before a time left
  static constexpr int POINTS = 60;           // one a minute: the last hour

  static constexpr uint32_t JUMP_S = 86400;   // the clock moved further than any sleep (set, or synced): start over
  void sample(uint32_t utc, uint16_t mv) {
    if (last_ && (utc < lastUtc_ || utc - lastUtc_ > JUMP_S)) { n_ = head_ = 0; since_ = 0; }   // points would mix two clocks
    if (last_ && utc >= lastUtc_ && utc - lastUtc_ < GAP_S) {
      if (mv >= last_ + JUMP_MV) setCharging(true, utc);
      else if (mv + JUMP_MV <= last_) setCharging(false, utc);
    }
    last_ = mv; lastUtc_ = utc;
    if (n_ && utc - at_[newest()] < 60) return;   // one point a minute
    at_[(head_ + n_) % POINTS] = utc; mv_[(head_ + n_) % POINTS] = mv;
    if (n_ < POINTS) n_++; else head_ = (head_ + 1) % POINTS;
    trend(utc);
  }

  Reading now() const {
    Reading r;
    if (!last_) return r;
    r.mv = last_; r.pct = percent(last_); r.charging = charging_;
    if (!charging_) r.minutesLeft = left();
    return r;
  }

 private:
  int newest() const { return (head_ + n_ - 1) % POINTS; }
  void setCharging(bool on, uint32_t utc) { if (on != charging_) { charging_ = on; since_ = utc; } decided_ = true; }
  // Against the newest point at least TREND_MIN minutes old, if any.
  void trend(uint32_t utc) {
    int then = -1;
    for (int k = 0; k < n_; k++) {
      int i = (head_ + k) % POINTS;
      if (utc - at_[i] < (uint32_t)TREND_MIN * 60) break;
      then = i;
    }
    const int d = then < 0 ? 0 : (int)last_ - (int)mv_[then];
    if (d >= RISE_MV) setCharging(true, utc);
    else if (d <= -FALL_MV) setCharging(false, utc);
    else if (!decided_ && last_ >= FLOAT_MV) setCharging(true, utc);
  }
  // Minutes to 0 % from a least-squares fit of percent over time, on the points since charging last ended.
  int16_t left() const {
    float sx = 0, sy = 0, sxx = 0, sxy = 0; int m = 0; uint32_t first = 0;   // float: the S3 has no double FPU
    for (int k = 0; k < n_; k++) {
      int i = (head_ + k) % POINTS;
      if (since_ && at_[i] < since_) continue;
      if (!m) first = at_[i];
      float x = (at_[i] - first) / 60.0f, y = percent(mv_[i]);
      sx += x; sy += y; sxx += x * x; sxy += x * y; m++;
    }
    if (m < 2 || (lastUtc_ - first) < (uint32_t)LEFT_MIN * 60) return -1;
    float rate = -(m * sxy - sx * sy) / (m * sxx - sx * sx);   // percent a minute, falling
    if (rate <= 0) return -1;
    float mins = percent(last_) / rate;
    return (int16_t)(mins > 30000 ? 30000 : mins < 1 ? 1 : mins);
  }

  uint32_t at_[POINTS] = {0};
  uint16_t mv_[POINTS] = {0};
  int head_ = 0, n_ = 0;
  uint16_t last_ = 0;
  uint32_t lastUtc_ = 0, since_ = 0;
  bool charging_ = false, decided_ = false;
};

}  // namespace battery
