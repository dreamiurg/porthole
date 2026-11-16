// Regression: one failed touch read in the middle of a press must not look like a finger lift, and a dead bus must.
#include "host/check.h"
#include <stdio.h>
#include "firmware/touch_hold.h"
int main() {
  board::TouchHold hold;
  const board::Touch none = {false, 0, 0}, finger = {true, 200, 300}, moved = {true, 210, 300};
  auto same = [](board::Touch a, board::Touch b) { return a.down == b.down && a.x == b.x && a.y == b.y; };
  assert(same(hold.step(false, none), none));     // a failure before any touch is no touch
  assert(same(hold.step(true, finger), finger));
  assert(same(hold.step(false, none), finger));   // one failed read: the press holds where it was
  assert(same(hold.step(true, moved), moved));    // and the drag goes on, uncut
  assert(same(hold.step(false, none), moved));
  assert(same(hold.step(false, none), none));     // a second failure in a row is a release
  assert(same(hold.step(false, none), none));     // and a dead bus stays released
  assert(same(hold.step(true, finger), finger));
  assert(same(hold.step(true, none), none));      // a good read of "no finger" is a lift at once
  assert(same(hold.step(false, none), none));     // nothing to hold after a real lift
  assert(hold.misses() == 6);
  puts("touch hold: one failed read keeps the press, two release it, a real lift is immediate");
}
