// Ownership of the two application buffers. Call under the board's frame critical section.
// A VSYNC does not release a buffer: the RGB driver must have copied its last pixel into SRAM first.
#pragma once
namespace board {
class DisplayFrames {
 public:
  int front() const { return front_; }
  int back() const { return 1 - front_; }
  bool submit() {
    if (pending_ || copied_) return false;
    pending_ = true;
    return true;
  }
  bool frameCopied() {
    if (!pending_) return false;
    pending_ = false; copied_ = true;
    return true;
  }
  bool finish() {
    if (!copied_) return false;
    copied_ = false; front_ = 1 - front_;
    return true;
  }
 private:
  int front_ = 0;
  bool pending_ = false, copied_ = false;
};
}
