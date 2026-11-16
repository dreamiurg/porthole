// Datagrams between Portholes on the same home wifi, for games played together (Rally's Together race). The firmware
// sends them as UDP on the saved network (net::Lan in firmware/net.cpp), the sim as UDP between instances on this
// machine (host/sim.cpp), the tests through a pair in memory. Nothing here leaves the LAN: a packet goes to one peer's
// address or, with to 0, to every device on the network (broadcast), never past the router. Header-only.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace lan {

constexpr size_t MAX_PACKET = 160;   // a game's datagram at most (Rally's is 144 bytes)

enum State : uint8_t {
  S_OFF,       // not started (or no link on this build: the browser emulator)
  S_NO_WIFI,   // no network saved, or it would not connect
  S_JOINING,   // waiting for the radio or joining the wifi
  S_READY,     // on the network: send() and recv() work
};

class Radio {
 public:
  virtual void start() = 0;   // join the saved network and listen; a second call does nothing
  virtual void stop() = 0;    // the radio goes back to the updater
  virtual State state() = 0;
  virtual uint32_t self() = 0;   // this device's address on the network (0 until S_READY)
  // One datagram to `to` (an address recv() reported), or to every device when to is 0. Dropped unless S_READY.
  virtual void send(const void* data, size_t len, uint32_t to) = 0;
  // The next datagram waiting, its sender in *from; 0 when none. Datagrams can be lost, late or out of order.
  virtual size_t recv(void* buf, size_t cap, uint32_t* from) = 0;

 protected:
  ~Radio() = default;
};

// The one radio, set by the firmware or the sim before the shell starts; nullptr reads as S_OFF.
inline Radio*& radioSlot() { static Radio* r = nullptr; return r; }
inline void setRadio(Radio* r) { radioSlot() = r; }
inline Radio* radio() { return radioSlot(); }

}  // namespace lan
