# Porthole: games for a round screen

Porthole is the firmware for the **Waveshare ESP32-S3-Touch-LCD-2.1**: a small shared runtime
(`os/`: 160×160 indexed framebuffer, 32-color palette, 8×8 font, touch input), the board layer
(`firmware/`), a shell (`shell/`: who's playing, a profile per kid, the game launcher) and the games
built on it (`games/`). Its first and, for now, only game is Pets Club.

## Profiles and the launcher

Power on, pick who's playing, pick a game. Up to four kids share one board, each with a profile: a
name, one of eight animal faces, an age (it picks the reading level) and an optional 4-digit code.
Every game keeps its progress per profile. From the launcher a kid can mute the buzzer for their
profile. Each kid plays up to 25 minutes a day across every game, then the board says "Back
tomorrow" until midnight. With two or more profiles, a kid who has played for about 6 minutes also
rests for 10, so the board passes to the next kid. A minute without a touch does not count as play.
A long press on a profile (after its code) deletes
it and everything it made.

## Pets Club: a pixel puppy

A touch-only, Tamagotchi-style puppy for the **Waveshare ESP32-S3-Touch-LCD-2.1** (round 480×480
IPS, capacitive touch, no buttons). Made for kids aged 5-10 who like dogs and books: the puppy
grows over real days, learns tricks, and above all loves being **read to**. Every profile gets its
own pup and a house on Paw Street.

Everything is drawn at 160×160 in a 32-color retro palette and scaled up 3× to the panel.

![Pets Club screens: home, a story, a trick lesson, spelling fetch, a daily gift, and the street](docs/screenshots.png)

## What it does

* **Adopt.** A parcel arrives and a puppy pops out. The kid names the pup and picks the house
  colors.
* **Home.** One cosy room: a window with day, night, rain and a visiting squirrel, a bookshelf that
  fills up with unlocked books, a lamp to tuck the dog in, a food bowl, a toy ball. Four big
  buttons: **feed · play · read · tricks**. Tap the dog to pet it; hold for a belly rub.
* **Needs, gently.** Food, fun, sleep and clean decay slowly and **stop at a floor: the dog never
  dies, runs away or resets.** Poop shows up a few hours after meals (one tap cleans it); muddy
  paws after playing mean a bath you give by rubbing the mud off.
* **Read.** A library of 30 original short stories in three reading levels, 5 to 11 pages each.
  The dog sits and listens, and each story ends with one comprehension question. Reading is the
  biggest source of bond hearts.
* **Play.** *Fetch*: slide to catch falling bones and dodge the bee. *Word Fetch*: read a clue,
  tap letter tiles to spell the word.
* **Tricks.** Eight tricks (sit, shake, speak, spin, roll over, play dead, beg, dance) unlock
  with bond hearts, each learned in three Simon-says lessons on three different days.
* **Day after day.** The real-time clock drives it all: a gift every new calendar day (a book, a
  hat, a party on day 7), a day streak, growing from puppy to dog to grown dog, sixteen stickers,
  bedtime at 8 pm. Time away counts for at most 12 hours and nights are slept through, so a
  weekend off never punishes anyone.
* **Paw Street.** The front door opens onto the neighbourhood: a house for every profile on the
  board, with that kid's pup at the door. Only your own house opens.
* **Reading level by age.** Stories and spelling words scale with the profile's age: level 1 at
  5-6 up to level 3 from 9.
* **Quiet by design.** The buzzer is harsh, so it only sounds for rare moments (growing up, a
  new trick, a gift, a sticker). Each profile can mute it on the launcher.

## Play it without the board

You need a C++17 compiler and Python 3. From this directory (`apps/porthole/`):

```bash
make snap && python3 tools/webemu.py   # or: make webemu
```

Open http://127.0.0.1:8765 (`python3 tools/webemu.py 8766` for another port). The mouse is your
finger; the buttons under the screen skip time and force states (hungry, muddy, grown...) so you
can see a week of play in a minute.

With SDL2 installed, `make sim` opens a native window instead: `h` / `n` / `d` skip 1 hour, 8
hours, 1 day; `r` resets; `s` saves a screenshot to `build/host/shot.bmp`; `q` quits.

The easiest way to put it on a board: download `porthole-<version>-factory.bin` from the [releases](https://github.com/dreamiurg/porthole/releases) and write it at address `0x0` with [esptool-js](https://espressif.github.io/esptool-js/) in Chrome or Edge ([step by step](https://dreamiurg.net/porthole/)). This erases saved progress.

## Build and flash

Needs [PlatformIO](https://platformio.org/). The board config is shared across the monorepo
(`../../platform/waveshare-round.ini`); the first build downloads the toolchain and takes a few
minutes.

```bash
make firmware                                # pio run -e firmware
make flash                                   # PlatformIO finds the port on its own
make flash PORT=/dev/cu.usbmodemXXXX        # ...or name it (Linux: usually /dev/ttyACM0)
make monitor                                 # serial log at 115200 baud
```

On first boot the clock is set from the build time. To set it exactly, send `T<epoch>` over serial
using local wall-clock seconds, e.g. `printf 'T%s\n' "$(date +%s)" > /dev/cu.usbmodemXXXX`.

### Serial commands (115200 baud)

| Command | Effect |
| --- | --- |
| `S` | print the active profile, the open game's stats, free heap and the current epoch |
| `T<epoch>` | set the clock (local wall-clock seconds) |
| `R` | erase every profile and every game's saves, then reboot |
| `P<n>` | clear the secret code of profile `n` (the parent escape hatch) |
| `D` | toggle touch-position logging |

## Checks

```bash
make check      # content fits its pixel boxes, sprites are current, -Werror build, pet and shell self-tests (seconds)
make playtest   # 17 scripted playthroughs, a UI audit (target size, bezel, overlap, contrast), chaos monkeys under ASan
make ci         # check + playtest + coverage: what CI runs for this app, minus the firmware build
make coverage   # line coverage of os/, shell/ and games/ -> build/coverage/coverage.xml (needs gcovr, or uvx)
```

The playtest report lands in `build/playtest/report.md`, with contact sheets next to it if
Pillow is installed.

## Hardware notes

| Function | Where |
| --- | --- |
| LCD | ST7701, 16-bit RGB, pins per the Waveshare wiki; init over 9-bit SPI on GPIO1/2 with CS on the expander |
| Touch | CST820, I²C 0x15 on SDA 15 / SCL 7 |
| IO expander | TCA9554 @ 0x20: P0 LCD reset, P1 touch reset, P2 LCD CS, P7 buzzer |
| RTC | PCF85063 @ 0x51, set from build time on first boot, restored from the save if it stops |
| Backlight | GPIO6 PWM; dims after 1 min idle, off after 5 min (tap wakes), off 20 s after bedtime |
| Storage | NVS (`Preferences`): a 44-byte record per profile (`porthole/p0`..`p3`) and one blob per profile per game (Pets Club: 156 bytes, `crago/s0`..`s3`), all with CRC. Pets Club's houses become profiles on the first Porthole boot |
| Serial | UART0 through the on-board CH343 USB bridge, 115200 baud |

Rendering: two 480×480 RGB565 framebuffers in PSRAM with bounce buffers; each frame the 160×160
indexed buffer is expanded through the palette into the back buffer and swapped on vsync.
Day / evening / night are palette variants applied at flip time.

## Layout

```
os/                shared, platform-independent runtime: renderer (gfx.cpp), input, palette, font
games/pets-club/   Pets Club: pet simulation (pet.cpp), screens (game.cpp), stories & words
                   (content*.h), generated sprites (sprites.h), and its tools: pixel-art rig
                   (art.py), content checker (check_content.py)
firmware/          Waveshare board layer (board.cpp: display, touch, expander, RTC, buzzer, NVS)
                   and the entry point (main.cpp: input, saves, idle dimming, serial commands)
host/              SDL2 / headless simulator and the pet self-test
tests/             playtest scenarios
tools/             browser emulator, playtest runner
```

Contributing, or pointing an AI agent at it? Read [CLAUDE.md](CLAUDE.md) first: it has the hard
constraints (round screen, 32 colors, ASCII only, append-only saves, the pet never dies) and the
workflows.

## Credits

* 8×8 font: `font8x8` by Daniel Hepper, public domain.
* Panel init sequence and pin map: Waveshare ESP32-S3-Touch-LCD-2.1 demo code.
* Palette: PICO-8's 16 colors plus 16 extras.
* Stories, art rig and game design: written for this project.
