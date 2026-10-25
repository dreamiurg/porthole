// Regression: stale VSYNC / timeout must never allow drawing into a framebuffer still being copied for scan-out.
#include "host/check.h"
#include <stdio.h>
#include "firmware/display_frames.h"
int main() {
  board::DisplayFrames frames;
  assert(frames.front() == 0 && frames.back() == 1);
  assert(!frames.frameCopied() && !frames.finish());   // no submission: discard stale completion
  for (int i = 0; i < 1000; i++) {
    int old = frames.front(), submitted = frames.back();
    assert(frames.submit());
    // A VSYNC or a wait timeout is not a frame-copy completion. Ownership must remain unchanged.
    assert(!frames.finish() && !frames.submit());
    assert(frames.front() == old && frames.back() == submitted);
    assert(frames.frameCopied());
    assert(!frames.frameCopied() && !frames.submit());   // duplicate interrupt cannot complete the next frame
    assert(frames.front() == old);
    assert(frames.finish());
    assert(frames.front() == submitted && frames.back() == old);
    assert(!frames.finish() && !frames.frameCopied());
  }
  puts("display frames: stale notifications, timeout ownership and 1000 buffer swaps passed");
}
