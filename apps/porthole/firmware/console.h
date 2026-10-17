// The serial console on both of the board's USB-C ports. The "USB to UART" port goes through the CH343 to UART0;
// the other is the ESP32-S3's own USB (USB-Serial-JTAG, CDC). Whichever one is plugged in, logs show up there and
// serial commands are read from it: every write goes to both, reads take whichever has bytes waiting.
//
// `Serial` is redefined to this console, so every Serial.print / readStringUntil / parseInt in the firmware (and
// tools/devctl.py on either port) keeps working unchanged. Arduino.h comes first: its own `#define Serial Serial0`
// runs once (pragma once), so the redefinition below sticks wherever this header is included.
#pragma once
#include <Arduino.h>
#include <HWCDC.h>

class Console : public Stream {
 public:
  void begin(unsigned long baud) {
    Serial0.begin(baud);
    // Waits only while a write makes no progress, then drops output until a host reads again: a board on a USB port
    // with no terminal open stalls one write by this much, not every frame. Long enough for a frame dump to drain.
    usb_.setTxTimeoutMs(20);
    usb_.setTxBufferSize(4096);   // the default 256 B overflows mid-reply (an S answer is ~300 B) and drops bytes
    usb_.begin();
  }
  size_t write(uint8_t c) override { usb_.write(c); return Serial0.write(c); }
  size_t write(const uint8_t* b, size_t n) override { usb_.write(b, n); return Serial0.write(b, n); }
  int available() override { return Serial0.available() + usb_.available(); }
  int read() override { return Serial0.available() ? Serial0.read() : usb_.read(); }
  int peek() override { return Serial0.available() ? Serial0.peek() : usb_.peek(); }
  void flush() override { Serial0.flush(); usb_.flush(); }
  using Print::write;

 private:
  HWCDC usb_;
};

inline Console console;

#undef Serial
#define Serial console
