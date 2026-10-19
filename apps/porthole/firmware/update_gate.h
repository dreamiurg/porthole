// A worker cannot touch flash until the loop has installed the SRAM-backed progress display.
#pragma once
#include <atomic>
namespace net {
class UpdateGate {
 public:
  void request() { state.store(WAITING); }
  bool requested() const { return state.load() != OPEN; }
  bool ready() const { return state.load() == READY; }
  bool failed() const { return state.load() == FAILED; }
  bool cancelled() const { return state.load() == CANCELLED; }
  bool committing() const { return state.load() == COMMITTING; }
  bool cancel() {
    State expected = READY;
    return state.compare_exchange_strong(expected, CANCELLED);
  }
  bool beginCommit() {
    State expected = READY;
    return state.compare_exchange_strong(expected, COMMITTING);
  }
  void acknowledge(bool ok) {
    State expected = WAITING;
    state.compare_exchange_strong(expected, ok ? READY : FAILED);
  }
  void release() { state.store(OPEN); }
 private:
  enum State { OPEN, WAITING, READY, FAILED, CANCELLED, COMMITTING };
  std::atomic<State> state{OPEN};
};
}
