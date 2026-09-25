# The board

Every app targets the **Waveshare ESP32-S3-Touch-LCD-2.1**
([product page](https://www.waveshare.com/esp32-s3-touch-lcd-2.1.htm),
[wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1)).

| Part | Detail |
| --- | --- |
| SoC | ESP32-S3R8: dual-core 240 MHz, 8 MB octal PSRAM, 16 MB flash |
| Display | 2.1" round IPS, 480x480, ST7701S driven as 16-bit RGB565 parallel |
| Touch | CST820 capacitive, I2C `0x15` |
| I/O expander | TCA9554, I2C `0x20` |
| Clock | PCF85063 RTC, I2C `0x51`, keeps time on the backup cell |
| Buzzer | Single-tone piezo on the expander. It is harsh: use it rarely. |
| USB | USB-C through a CH343 USB-UART bridge, so `Serial` is UART0, not USB CDC |
| Radio | 2.4 GHz Wi-Fi (802.11 b/g/n) and Bluetooth 5 LE, onboard antenna |
| Motion | QMI8658 6-axis accelerometer and gyroscope |
| Storage | microSD (TF) card slot |
| Battery | MX1.25 header for a 3.7 V lithium cell, with a charging chip onboard |

Buy the flat version, `ESP32-S3-Touch-LCD-2.1`
([Amazon](https://www.amazon.com/dp/B0DDPQSKJD),
[Waveshare](https://www.waveshare.com/esp32-s3-touch-lcd-2.1.htm)). The
`ESP32-S3-Touch-LCD-2.1B` has curved 2.5D glass over the same size screen; no
app here has been tested on it. Round boards in other sizes won't run these apps.

## Not used yet

These parts are on the board but no app uses them yet. They're good starting
points for a new app or feature.

- Motion sensor: shake to wake a pet, tilt to steer in a minigame, or count steps
  when someone carries the device around.
- Bluetooth LE: two devices near each other could let pets meet, play together
  or trade stickers.
- Wi-Fi: set the clock automatically, update firmware without a cable, or
  download new stories.
- microSD slot: room for more stories, art or sound than the 16 MB flash holds.
- Battery header: a small lithium cell would make the device fully cordless.

## Pins that matter

| Signal | Where |
| --- | --- |
| I2C SDA / SCL (touch, expander, RTC) | GPIO15 / GPIO7 |
| ST7701 init SPI: CLK / MOSI | GPIO2 / GPIO1 (9-bit SPI) |
| ST7701 CS | expander P2 |
| LCD reset | expander P0 |
| Touch reset | expander P1 |
| Buzzer | expander P7 |
| Backlight | GPIO6 (LEDC PWM) |

The display comes up in two steps: an init sequence over 9-bit SPI, then the
RGB panel through ESP-IDF's `esp_lcd` RGB driver with frame buffers in PSRAM.
`apps/pets-club/src/board.cpp` does this without extra libraries.
`apps/biscuit/firmware/src/board.cpp` does the same under LVGL.

## Toolchain

`platform/waveshare-round.ini` pins the PlatformIO platform (pioarduino 54.03.21,
Arduino-ESP32 3.x on ESP-IDF 5.4) and the board settings every app shares.
`platform/requirements.txt` pins PlatformIO Core 6.1.19 and the Python modules
that builder imports. Install them into a virtualenv or with pipx:

```sh
pip install -r platform/requirements.txt
```

PlatformIO auto-detects the serial port when one board is plugged in. Pass
`PORT=/dev/cu.usbserial-XXXX` (macOS) or `PORT=/dev/ttyUSB0` (Linux) to pick one.

## Designing for a round screen

- The visible area is a circle. A rectangle that fits the 480x480 square can
  still lose its corners behind the bezel. Check every corner of every text box
  and button against the circle.
- There are no physical buttons. Every screen needs a visible way back.
- Fingers are big: aim for touch targets of at least 8 mm (72 physical px).
