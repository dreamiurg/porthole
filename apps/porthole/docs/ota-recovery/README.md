# Beta OTA recovery, 2026-10-25

On the Waveshare ESP32-S3-Touch-LCD-2.1, both the connected development image
and published beta 0.29.1 returned `progress display unavailable: update deferred`
when sent `U`. The development image also intentionally refuses downloads, and
the device was configured for Stable rather than Beta.

The repaired 0.29.1 recovery build uses the latest main baseline bc4c63a plus
small-chunk update display allocation and heap-backed Starfighter/packed-image
scratch. Static internal RAM is 125304 / 131072 bytes. The physical boot reports
`update ram: 200088 free, 122112 needed`; all four profiles load.

The USB recovery used separate firmware parts, preserving NVS. A backup was
kept locally; requested settings changes were independently reviewed and verified
by byte-for-byte flash readback. Private saves and credentials are not committed.

This is an allocation-only repair: no player-visible pixels, controls, or game
rules changed. The code reviewer confirmed that a new gameplay playthrough is
not required. Hardware OTA checks exercise the actual DMA and network path;
host tests cover sparse-picture pixels, bank isolation, allocation handshake,
packed-image output, and the existing gameplay regressions.

Validation:

- Independent code review: rollback failure and allocation failure findings fixed;
  final pass reported no findings.
- `make check`, `make ci` with Node 22 and CI Python dependencies: passed.
  All 74 playtest scenarios passed; line coverage 98.8% (8217/8315).
- `make complexity` and firmware build: passed, warnings treated as errors.
- Physical Beta check: `update=3 available=0.29.1` (current), after Wi-Fi and TLS.
- Both requested replacement PINs unlock their existing profiles; all four
  profiles remain present. No private profile screenshots are committed.
- Physical Starfighter title, calibration, brief and flight render with the
  heap-backed scene. A short flight accepted a shot and exited by holding Exit.
  Metrics before screenshot capture: 29.3 fps, 34.18 ms/frame, 31.67 ms render.
  Upright gravity was supplied through the serial tilt harness and then cleared.
  [Device framebuffer](starfighter-flight.png). This is a rendering smoke test,
  not a complete gameplay/balance test or optical inspection of the LCD.

Full OTA download/reboot into the newly published repair is the remaining
post-release device check. The deliberate insufficient-RAM rollback path has not
been exercised on hardware. Free-RAM headroom is a conservative guard, not a
proof against every later heap fragmentation or allocation failure.
