# Biscuit on the round touchscreen

Native, offline firmware for the Waveshare ESP32-S3-Touch-LCD-2.1: 480×480 ST7701 RGB LCD, CST820 touch, TCA9554 expander, PCF85063 clock, 16MB flash and 8MB OPI PSRAM. Every game control is on the screen.

The browser game's artwork and reading content are generated into firmware assets. Seven branching stories, 96 illustrated discoveries, care, fetch, six tricks, daily invitations, growth and stickers run locally. Progress is stored in ESP32 NVS (namespace `biscuit`) with version, checksum and validation; ordinary resets and firmware updates preserve it. The device has its own save, separate from browser localStorage.

## Build

Install PlatformIO Core and Node.js 22. Node 22 is required to build the firmware; assets are generated from the browser game's sources at build time. From `apps/biscuit/`:

```sh
make firmware        # = pio run -d firmware -e firmware
```

A pre-build hook (`tools/gen_assets.py`) runs `tools/export-assets.mjs`, which writes `generated/assets.{h,cpp}` and `generated/manifest.json`. Those files are not committed. The generator only rewrites files whose content changed, so incremental builds stay incremental.

`platformio.ini` extends the shared `[waveshare_round]` base in `../../../platform/waveshare-round.ini` (pioarduino 54.03.21 = Arduino-ESP32 3.x, board, flash and PSRAM settings) and adds only Biscuit's partition table, source filter, C++17 flags and LVGL 8.3.10. That platform's optional size-report command requires `esp-idf-size==1.6.1` in PlatformIO's Python environment; newer 2.x removed its `--ng` switch. The stock ESP32 Arduino 2.x platform is incompatible with this driver.

The LVGL fonts in `generated/font*.c` and `generated/fonts.h` are committed. They come from the bundled Montserrat (OFL) via `node firmware/tools/export-fonts.mjs`, which fetches `lv_font_conv` with npm and so needs network access; `make fonts-check` verifies them. No Wi-Fi credentials are required.

## Install and recover

Connect the board's USB-C port (a CH343 USB-to-UART bridge). Before the first installation, save the original 16MB flash with esptool's `read-flash 0 0x1000000 original-flash.bin`. Keep this backup private and outside Git: it can contain old settings.

```sh
make flash                          # PlatformIO auto-detects the port
make flash PORT=/dev/cu.usbmodemXXXX
```

The conservative 115200 upload speed works with the board's automatic reset circuit. If a transfer fails, retry it; no flash erase is needed. To restore the original firmware, use esptool `write-flash 0 original-flash.bin` with the matching connected board. Restoration replaces game progress too.

## Clock and brightness

More → Settings adjusts brightness and opens a touch-only date/time picker. The firmware defaults to US Pacific time, including daylight-saving transitions; change `timezoneRule` in `src/main.cpp` for your region.

Set the RTC to UTC over USB after installation with `python firmware/tools/device.py --port PORT time`. A valid RTC advances daily discoveries and visits without internet. Keeping time through complete power loss requires the board's RTC backup battery. If the RTC loses power, the game resumes from its most recent saved time and asks for the clock to be set; it never guesses how long the puppy was alone. Need decay is capped at eight hours and never falls below 20.

## Verification

From `apps/biscuit/`:

```sh
make check                               # JS lint + tests, partition check, native pet-model test
make firmware && make firmware-test      # build, then the real-font layout test
python firmware/tools/device.py --self-test
```

`make firmware-test` runs `tools/check-layout.sh`: it compiles the actual firmware paginator against LVGL from `.pio/libdeps/firmware/lvgl` and measures every label and reading page with the exact bundled fonts, rejecting content outside the circular panel.

`tools/device.py` communicates over USB for status, UI tree, clock setting, simulated touch and RGB framebuffer capture. It requires pyserial (included in PlatformIO's Python environment). Without `--port` it picks the single `/dev/cu.usbmodem*` device on macOS; pass `--port` elsewhere. Simulated taps pass through the same LVGL input path as the physical touch driver. They verify game flows, not finger calibration. Framebuffer captures show the pixels sent to the LCD; they are not photographs of brightness, contrast or viewing angles.

Run `python firmware/tests/device_flow.py --port PORT` for the complete USB-driven care, reading, discovery, fetch, training, sleep and settings flow. It captures screen pixels under `output/device-qa/` and restores progress when finished. The `test-begin` / `test-end` commands suspend progress writes and restore the original game state, brightness and clock basis afterward. Always finish a test session. Test clock changes remain virtual and never write the RTC.

### Verified on hardware

On an ESP32-S3 (revision 0.2, 16MB flash, 8MB PSRAM) the display, touch, expander and RTC initialize; home animation runs at four frames per second with touch polling around 27-46Hz and about 272KB internal heap free. The USB-driven flow above passed end to end, and saves and the clock survived a hardware reset. Physical finger accuracy, LCD appearance, battery endurance and timekeeping through complete power loss are unverified.

## Hardware sources and licenses

- [Waveshare board documentation](https://docs.waveshare.com/ESP32-S3-Touch-LCD-2.1)
- [Official driver examples](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1/ESP32-S3-Touch-LCD-2.1-Demo.zip): pin mapping, panel command values and timing. The supplied board example has no separate license notice; its provenance is retained in `src/board.cpp`.
- [Board schematic](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1/ESP32-S3-Touch-LCD-2.1_schematic_diagram.pdf)
- [NXP PCF85063A datasheet](https://www.nxp.com/docs/en/data-sheet/PCF85063A.pdf)
- [LVGL](https://github.com/lvgl/lvgl/tree/v8.3.10), MIT; [Arduino-ESP32](https://github.com/espressif/arduino-esp32/tree/3.2.1), LGPL2.1; [ESP-IDF](https://github.com/espressif/esp-idf), Apache2.0.
- [Montserrat license](../assets/fonts/OFL.txt). Illustrations, puppy artwork and stories are original project content; discovery source citations remain available in both versions.
