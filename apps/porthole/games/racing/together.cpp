#include "together.h"
#include <string.h>

namespace racing::together {
namespace {
bool fresh(const Wire& a, const Wire& b) {   // a change the other device should see now, not in SEND_MS
  return a.stage != b.stage || a.paused != b.paused || a.raceId != b.raceId || a.track != b.track || a.to != b.to ||
         a.finished != b.finished;
}
}  // namespace

void Link::take(const Wire& w, uint32_t from, uint32_t ms, Stage mine) {
  const uint32_t self = radio_->self();
  if (peer_.addr) {   // paired: only the peer, and only what it addressed to this device
    if (from != peer_.addr || w.to != self) return;
    peer_.w = w; peer_.heardMs = ms; heard_ = true; gen_++;
    return;
  }
  if (mine == Stage::Looking && (Stage)w.stage == Stage::Lobby && w.to == self) {   // the first kid to join
    peer_ = {from, ms, w}; heard_ = true; gen_++;
    return;
  }
  if ((Stage)w.stage == Stage::Looking && !w.to) invite_ = {from, ms, w};
}

void Link::tick(uint32_t ms, Wire& mine) {
  if (!radio_ || radio_->state() != lan::S_READY) return;
  Wire w;
  uint32_t from = 0;
  for (int n = 0; n < 16; n++) {   // a bounded drain: the radio keeps at most a few
    const size_t len = radio_->recv(&w, sizeof w, &from);
    if (!len) break;
    if (len != sizeof w || w.magic != MAGIC || w.version != VERSION || from == radio_->self()) continue;
    w.name[sizeof w.name - 1] = 0;   // a name from the network is shown: always terminated
    take(w, from, ms, (Stage)mine.stage);
  }
  mine.magic = MAGIC; mine.version = VERSION; mine.to = peer_.addr;
  if ((Stage)mine.stage == Stage::Idle && (Stage)last_.stage == Stage::Idle) return;   // quiet on the track cards
  if (!fresh(mine, last_) && ms - sentMs_ < SEND_MS) return;
  radio_->send(&mine, sizeof mine, mine.to);
  last_ = mine; sentMs_ = ms;
}
void Link::leave(Wire& mine) {
  mine.stage = (uint8_t)Stage::Idle; mine.magic = MAGIC; mine.version = VERSION; mine.to = peer_.addr;
  if (radio_ && peer_.addr) for (int i = 0; i < 3; i++) radio_->send(&mine, sizeof mine, peer_.addr);
  last_ = mine;
  unpair();
}
}  // namespace racing::together
