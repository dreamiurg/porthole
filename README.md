# Porthole

[![Beta](https://img.shields.io/github/v/release/dreamiurg/porthole?include_prereleases&filter=porthole-v*&label=beta)](https://github.com/dreamiurg/porthole/releases)
[![Stable](https://img.shields.io/github/v/release/dreamiurg/porthole?filter=porthole-v*&label=stable)](https://github.com/dreamiurg/porthole/releases/latest)

Open source firmware for a round Waveshare ESP32 touchscreen, with 12 small games and printable 3D models for a
case and a steering wheel. I built it for my kids, and hope yours will enjoy it, too.

**[Play it in your browser](https://porthole.dreamiurg.net/play/)**, no board needed: it is the real firmware,
built for the web. The [project page](https://porthole.dreamiurg.net/) has the same demo, a 3D view of the case
and an install guide. I write about this and other projects on my [blog](https://dreamiurg.net/).

<p align="center"><img src="site/images/puck.webp" width="420" alt="The board in its 3D-printed case, showing the Porthole menu"></p>

## Games

All of them run in one firmware. Each kid gets a profile, up to four per board. No game punishes a kid who is
away: nothing dies, and no progress is lost.

| | | | |
| :---: | :---: | :---: | :---: |
| <img src="docs/games/pets-club.png" width="150" alt="Pets Club"><br>**Pets Club**<br>a pixel puppy that grows over real days | <img src="docs/games/biscuit.png" width="150" alt="Biscuit"><br>**Biscuit**<br>read stories with a pup, find 96 discoveries | <img src="docs/games/marble-kick.png" width="150" alt="Marble Kick"><br>**Marble Kick**<br>tilt a marble through 12 mazes | <img src="docs/games/tilt-fc.png" width="150" alt="Tilt FC"><br>**Tilt FC**<br>street football: tilt to run, tap to pass |
| <img src="docs/games/pumpkin-hide-seek.png" width="150" alt="Pumpkin Hide & Seek"><br>**Pumpkin Hide & Seek**<br>grab three candies before Grandma finds you | <img src="docs/games/radar.png" width="150" alt="Radar"><br>**Radar**<br>live planes over your home | <img src="docs/games/starfighter.png" width="150" alt="Starfighter"><br>**Starfighter**<br>bank to fly, tap to fire | <img src="docs/games/kingdom.png" width="150" alt="Kingdom"><br>**Kingdom**<br>a 40-scene story where choices matter |
| <img src="docs/games/toy-town.png" width="150" alt="Toy Town"><br>**Toy Town**<br>a doll town: school, café, park | <img src="docs/games/rally.png" width="150" alt="Rally"><br>**Rally**<br>hold it like a wheel and race | <img src="docs/games/sand-jar.png" width="150" alt="Sand Jar"><br>**Sand Jar**<br>tilt sand, water and stars | <img src="docs/games/times-cards.png" width="150" alt="Times Cards"><br>**Times Cards**<br>times tables, a few minutes a day |

More about each game: [apps/porthole/README.md](apps/porthole/README.md).

## Software

The firmware is C++ on Arduino and PlatformIO. A host simulator and a browser emulator run the same code on a
computer, so you can build and test a game without the board.

- **Put it on a board:** plug the board into a computer over USB-C, open
  [porthole.dreamiurg.net](https://porthole.dreamiurg.net/#install) in Chrome or Edge and press **Install**.
  You do this once: after that the board updates itself over Wi-Fi. Details and the manual way:
  [docs/install.md](docs/install.md).
- **Build from source:** `make check`, then `make flash APP=porthole`. Setup:
  [docs/development.md](docs/development.md).
- **Make your own game:** point Claude Code or Codex at the repo and describe what you want.
  [AGENTS.md](AGENTS.md) holds the rules it follows, and [docs/new-app.md](docs/new-app.md) the app contract.

Every merge ships a beta, and a beta becomes stable after three quiet days ([docs/releases.md](docs/releases.md)).

## Hardware

**The board** is the [Waveshare ESP32-S3-Touch-LCD-2.1](https://www.waveshare.com/esp32-s3-touch-lcd-2.1.htm):
a 2.1-inch round 480x480 touchscreen with an ESP32-S3, a motion sensor, a clock, a buzzer and a battery plug.
About $35 from Waveshare or $45 on [Amazon](https://www.amazon.com/dp/B0DDPQSKJD). Get the flat 2.1-inch
model; the other sizes and the curved 2.1B are not supported. Specs and pins: [docs/hardware.md](docs/hardware.md).

**The case** is a round puck that holds the board and a battery. Print the bezel and plate from
[v1.1](hardware/case/stl/v1.1) and the lid from [v1.2](hardware/case/stl/v1.2) in PETG, add a protected 3.7 V
LiPo (2000 mAh or more, 1.25 mm plug; check its polarity first, many ship reversed) and close it with M2
screws. No soldering.

**The steering wheel** is a two-handed grip the puck sits in, for Rally and the tilt games: two printed halves
that go together around a steel ring the puck sticks to. It is still a work in progress:
[hardware/grip](hardware/grip). The [project page](https://porthole.dreamiurg.net/#case) shows it in 3D.

<p align="center"><img src="site/images/grip.webp" width="600" alt="Renders of the steering-wheel grip holding the puck"></p>

## Contributing

Fixes, new stories and new games are welcome. [CONTRIBUTING.md](CONTRIBUTING.md) has the workflow.
