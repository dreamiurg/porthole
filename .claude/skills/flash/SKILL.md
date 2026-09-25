---
name: flash
description: Pets Club (apps/pets-club). Use when building, flashing, or bringing up Pets Club firmware on the physical Waveshare board. Covers the exact build/upload/serial sequence, the PlatformIO-venv Python-dependency caveat, and how to set the clock.
---

1. Build: `make -C apps/pets-club firmware` (runs `pio run -e firmware` in the app dir; the board config extends the shared `platform/waveshare-round.ini`).
2. If the build fails inside PlatformIO's own build tooling (not your code) with a missing Python module, install it into PlatformIO's own venv, not your shell's Python:
   ```bash
   uv pip install --python ~/.local/share/uv/tools/platformio/bin/python pyyaml rich-click intelhex rich esp-idf-size "click<8.2"
   ```
3. Find the device: `pio device list` (the on-board CH343 USB-UART bridge; macOS `/dev/cu.usbmodem*`, Linux usually `/dev/ttyACM*`). PlatformIO auto-detects it when only one board is attached.
4. Flash: `make -C apps/pets-club flash` (or `make -C apps/pets-club flash PORT=<port>`).
5. Monitor: `make -C apps/pets-club monitor` (or with `PORT=<port>`). Expect `[pets-club] boot`, then within a couple seconds `[pets-club] houses=N now=<epoch> heap=<bytes>`.
6. Set the clock if the boot log says the RTC was not running: send `T<epoch>` over serial, e.g. `printf 'T%s\n' "$(date +%s)" > <port>`. The board treats the value as local wall-clock seconds with no timezone math, so use `date +%s` on a machine already set to the timezone you want the board to show.
7. Other serial commands: `S` prints stats plus free heap and current epoch; `R` erases every house and reboots; `P<n>` clears house `n`'s 4-digit secret code; `D` toggles touch-position logging.
8. A clean build and a clean boot log are not a working display or touch panel. Only report those as confirmed if you or the user actually saw them work -- say explicitly which you verified and which you didn't.
