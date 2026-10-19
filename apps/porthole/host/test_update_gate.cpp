// Regression: no flash writes on worker startup, allocation failure, or a stale display acknowledgment.
#include "host/check.h"
#include <stdio.h>
#include "firmware/update_gate.h"
int main() {
  net::UpdateGate gate;
  gate.acknowledge(true);
  gate.request();
  assert(gate.requested() && !gate.ready() && !gate.failed());
  gate.acknowledge(true);
  assert(gate.ready());
  gate.release();
  gate.acknowledge(true);
  gate.request();
  assert(!gate.ready());
  gate.acknowledge(false);
  assert(gate.failed() && !gate.ready());
  gate.acknowledge(true);   // allocation failure must not become permission to write flash
  assert(gate.failed());
  gate.release();
  assert(!gate.requested());
  gate.request(); gate.release();   // task could not be allocated
  gate.acknowledge(true);
  assert(!gate.ready() && !gate.requested());
  gate.request(); gate.acknowledge(true);
  assert(gate.cancel() && gate.cancelled());
  assert(!gate.beginCommit());   // a cancellation accepted at the final byte must prevent installing the image
  gate.release(); gate.request(); gate.acknowledge(true);
  assert(gate.beginCommit() && gate.committing());
  assert(!gate.cancel() && !gate.cancelled());   // never report Stopping once verification/boot selection begins
  gate.release();
  puts("update gate: startup, allocation failure and stale acknowledgments passed");
}
