// Self-check for the battery gauge (os/battery.h): the percent curve, charging seen only on clear evidence (a jump, or
// a trend over minutes, never a backlight-size step or a sleep gap), and a time left only after a steady fall.
// Run: make test
#include "host/check.h"
#include <stdio.h>
#include "battery.h"

static int checks = 0;
using battery::Gauge;

// Samples every 10 s from t for secs, the voltage moving mvPerMin.
static uint32_t run(Gauge& g, uint32_t t, int secs, int mv, double mvPerMin) {
  for (int s = 0; s <= secs; s += 10) g.sample(t + s, (uint16_t)(mv + mvPerMin * s / 60));
  return t + secs + 10;
}

static void curve() {
  CHECK(battery::percent(3000) == 0 && battery::percent(3270) == 0);
  CHECK(battery::percent(4200) == 100 && battery::percent(4300) == 100);
  CHECK(battery::percent(3730) == 20 && battery::percent(3840) == 50);
  for (int mv = 3000; mv < 4300; mv++) CHECK(battery::percent(mv + 1) >= battery::percent(mv));
  CHECK(!battery::Reading{}.present() && battery::Reading{4000}.present());
}

static void plugAndUnplug() {
  Gauge g;
  CHECK(!g.now().present());
  uint32_t t = run(g, 1000000, 300, 3900, 0);
  CHECK(!g.now().charging && g.now().pct == 63);
  g.sample(t, 3920); g.sample(t + 10, 3900);   // the backlight's load: not a plug
  CHECK(!g.now().charging);
  g.sample(t + 20, 4040);   // plugged in
  CHECK(g.now().charging && g.now().minutesLeft == -1);
  t = run(g, t + 30, 600, 4040, 3);   // charging up
  CHECK(g.now().charging);
  g.sample(t, 3960);   // unplugged
  CHECK(!g.now().charging);
}

static void slowTrends() {
  Gauge g;   // plugged in while asleep: it wakes higher, then keeps rising
  uint32_t t = run(g, 1000000, 120, 3800, 0);
  g.sample(t + 3600, 3815);   // an hour asleep, resting a little higher: neither a jump nor a trend
  CHECK(!g.now().charging);
  t = run(g, t + 3610, 900, 3815, 2.5);
  CHECK(g.now().charging);
  t = run(g, t, 900, 4100, -1.5);   // unplugged gently: the fall turns it off
  CHECK(!g.now().charging);
  Gauge full;   // on the charger and full: flat at the float voltage
  run(full, 1000000, 120, 4190, 0);
  CHECK(full.now().charging);
}

static void timeLeft() {
  Gauge g;
  uint32_t t = run(g, 1000000, 19 * 60, 4000, -1);
  CHECK(g.now().minutesLeft == -1);   // under 20 minutes of fall
  run(g, t, 15 * 60, 3980 - 1 * 19, -1);
  int16_t left = g.now().minutesLeft;
  CHECK(left > 60 && left < 1000);   // about 1 mV, roughly 0.2 %, a minute from ~70 %
  Gauge flat;
  run(flat, 1000000, 40 * 60, 3900, 0);
  CHECK(flat.now().minutesLeft == -1);   // no fall, no guess
}

static void clockMoves() {
  Gauge g;   // a fall measured, then the clock set back an hour: the window starts over, no garbage
  uint32_t t = run(g, 1000000, 30 * 60, 4000, -1);
  CHECK(g.now().minutesLeft > 0);
  run(g, t - 3600, 60, 3970, 0);
  CHECK(g.now().minutesLeft == -1 && !g.now().charging);
  Gauge h;   // synced weeks ahead
  t = run(h, 1000000, 30 * 60, 4000, -1);
  run(h, t + 30 * 86400, 60, 3970, 0);
  CHECK(h.now().minutesLeft == -1);
}

int main() {
  curve();
  plugAndUnplug();
  slowTrends();
  timeLeft();
  clockMoves();
  printf("test_battery: all %d checks passed\n", checks);
  return 0;
}
