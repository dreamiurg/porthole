---
name: flash
description: Pets Club (apps/porthole). Use when building, flashing, or bringing up Pets Club firmware on the physical Waveshare board. Covers the exact build/upload/serial sequence, the PlatformIO-venv Python-dependency caveat, and how to set the clock.
---

1. Build: `make -C apps/porthole firmware` (runs `pio run -e firmware` in the app dir; the board config extends the shared `platform/waveshare-round.ini`).
2. If the build fails inside PlatformIO's own build tooling (not your code) with a missing Python module, install it into PlatformIO's own venv, not your shell's Python:
   ```bash
   uv pip install --python ~/.local/share/uv/tools/platformio/bin/python pyyaml rich-click intelhex rich esp-idf-size "click<8.2"
   ```
3. Find the device: `pio device list` (the on-board CH343 USB-UART bridge; macOS `/dev/cu.usbmodem*`, Linux usually `/dev/ttyACM*`). PlatformIO auto-detects it when only one board is attached.
4. Flash: `make -C apps/porthole flash` (or `make -C apps/porthole flash PORT=<port>`).
5. Monitor: `make -C apps/porthole monitor` (or with `PORT=<port>`). Expect `[porthole] boot`, then within a couple seconds `[porthole] profiles=N now=<epoch> heap=<bytes>`.
6. Set the clock if the boot log says the RTC was not running: send `T<epoch>` over serial, where `<epoch>` is local wall-clock seconds (the board does no timezone math): `printf 'T%s\n' "$(python3 -c 'import time; print(int(time.time()) + time.localtime().tm_gmtoff)')" > <port>`. Plain `date +%s` is UTC on every machine and would put the board hours off.
   After flashing a board that ran the old standalone Biscuit firmware, always set the clock with `T`, even when the RTC runs: that firmware kept the RTC on UTC, and Porthole reads it as local time.
7. Other serial commands: `S` prints the active profile and game stats plus free heap and current epoch; `R` erases every profile and every game's saves and reboots; `P<n>` clears profile `n`'s 4-digit secret code; `D` toggles touch-position logging.
8. A clean build and a clean boot log are not a working display or touch panel. Only report those as confirmed if you or the user actually saw them work -- say explicitly which you verified and which you didn't.
