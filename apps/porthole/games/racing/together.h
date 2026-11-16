// Rally together: two kids race the same race on two Portholes on the same home wifi (AGENTS.md, Together). Like a
// second player at an arcade cabinet: while Rally's track cards are open, a device says it is there (a broadcast a few
// times a second); the other device shows "Race!" with that kid's face, and once both kids tap it they are paired.
// Paired, each device sends its whole state as one small datagram (Wire) about 20 times a second, so a lost one
// changes nothing. The host (the lower address) runs the three rivals and sends them along; the other device drives its
// own car and shows the host's rivals. What goes over the network: the
// profile's name and avatar, the levels, the car's place on the track, and nothing else about the kid (AGENTS.md rule 4).
#pragma once
#include <stdint.h>
#include "lan.h"
#include "track.h"
#include "tune.h"

namespace racing::together {
constexpr uint32_t MAGIC = 0x32594C52;   // "RLY2"
constexpr uint8_t VERSION = 2;
constexpr uint32_t SEND_MS = 50, HERE_MS = 250, LOST_MS = 3000, INVITE_MS = 1500;   // HERE_MS: the broadcast, unpaired
constexpr uint32_t NOTICE_MS = 3000, GUESS_MS = 300;   // GUESS_MS: how far the other car is carried on between datagrams
constexpr int PAUSES = 2;                  // pause holds a kid gets each race (losing the grip always pauses)
constexpr int STARS = 2, STAR_RACES = 5;   // stars for each finish together, for the first STAR_RACES a day
// The tailwind: the kid behind gets up to TAILWIND more top speed, from WIND_FROM_S s behind, full at WIND_FULL_S s.
constexpr float TAILWIND = 0.08f, WIND_FROM_S = 1, WIND_FULL_S = 5;

// What a device is doing, as the other one sees it.
enum class Stage : uint8_t { Idle, Looking, Lobby, Grip, Ready, Race, Done };   // Looking: on the track cards

struct Bot { float dist, x, vx, speed; uint32_t finishMs; uint16_t boostMs, airMs; uint8_t finished, pad[3]; };
struct Wire {
  uint32_t magic;
  uint8_t version, stage, track, paused;
  uint32_t to;             // the paired device's address; 0 = anyone (on the track cards)
  uint32_t seed;           // the host's race: seed, level and track, the same field on both devices
  char name[12];           // the profile's name and avatar: who is joining (AGENTS.md rule 4, LAN only)
  uint8_t avatar, raceId, in, finished;   // in: this kid tapped Race! on the track cards
  int8_t level[TRACKS];    // the level each track would race at for this kid: the host races at the weaker one
  int8_t raceLevel;
  uint8_t place;           // 1..5 among both kids and the rivals, once finished
  uint8_t hits[CARS - 1], pad[3];   // the guest's rockets that found each rival this race, counted up (the host applies them)
  float dist, x, speed;
  uint32_t finishMs;
  Bot bot[CARS - 1];       // the host's rivals
};
static_assert(sizeof(Bot) == 28 && sizeof(Wire) == 144, "the wire layout: both ends are little-endian 32-bit");
static_assert(sizeof(Wire) <= lan::MAX_PACKET, "one datagram");

// One device's end: listens for invites while idle, pairs with one peer, and sends this device's Wire.
class Link {
 public:
  void begin(lan::Radio* r) { radio_ = r; peer_ = invite_ = {}; }
  // Drains the radio, then sends `mine` when it changed or SEND_MS (HERE_MS unpaired) passed. mine.to is filled in here.
  void tick(uint32_t ms, Wire& mine);
  lan::State state() const { return radio_ ? radio_->state() : lan::S_OFF; }
  bool on() const { return radio_ != nullptr; }   // this build has a network at all

  // Another Porthole on its track cards, heard within INVITE_MS: the kid there, and whether they tapped Race!.
  bool invited(uint32_t ms) const { return invite_.addr && ms - invite_.heardMs < INVITE_MS; }
  const Wire& invite() const { return invite_.w; }
  uint32_t inviter() const { return invite_.addr; }
  uint32_t self() const { return radio_ ? radio_->self() : 0; }

  // Pairing: the guest (the higher address) pairs when it joins; the host when a Lobby addressed to it comes in while
  // on its cards.
  void join(uint32_t ms) { peer_ = invite_; peer_.heardMs = ms; heard_ = false; }
  void unpair() { peer_ = {}; heard_ = false; }
  void leave(Wire& mine);   // tells the peer this device left (Stage::Idle, a few times: datagrams get lost), unpairs
  bool paired() const { return peer_.addr != 0; }
  bool heard() const { return heard_; }   // the peer has answered since pairing (addressed to this device)
  bool lost(uint32_t ms) const { return paired() && ms - peer_.heardMs > LOST_MS; }
  const Wire& peer() const { return peer_.w; }
  uint32_t peerAge(uint32_t ms) const { return ms - peer_.heardMs; }
  uint32_t gen() const { return gen_; }   // +1 for every datagram from the peer

 private:
  struct End { uint32_t addr = 0, heardMs = 0; Wire w{}; };
  lan::Radio* radio_ = nullptr;
  End peer_, invite_;
  uint32_t gen_ = 0, sentMs_ = 0;
  bool heard_ = false;
  Wire last_{};
  void take(const Wire& w, uint32_t from, uint32_t ms, Stage mine);
};
}  // namespace racing::together
