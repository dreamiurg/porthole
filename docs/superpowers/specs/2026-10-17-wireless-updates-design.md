# Wireless updates: design

Status: approved for build, 2026-10-17. Not started. Design page with diagrams and the reasoning behind each cut:
https://claude.ai/artifact/DzEsXDuUVM7atoBN9ywFBb

## What and why

Today every firmware update needs the board on a USB cable. After this work, the board downloads new releases over
wifi in the background, once a day. The new version starts the next time the board is switched on, and the kid
sees a short "Porthole has something new!" card. Releases come in two channels:

- **beta**: every release, automatically.
- **stable**: only releases a person promotes.

The kids' boards follow stable. A test board can follow beta.

The board is not powered overnight, so nothing depends on a night-time window.

## Decisions

- **Stock bootloader, two app slots.** ESP-IDF's OTA slots give automatic rollback. No custom bootloader and no
  recovery app.
- **Download and install are separate steps.**
  - The download streams straight into the spare slot, then sets that slot to start on the next power-on.
  - Starting the new version takes about 2 seconds.
  - The kid is never asked a question. There is only one sensible answer, and "Later" would come back on every
    power-on.
- **The channels use GitHub's pre-release flag. Nothing extra is hosted.**
  - Every release is created as a pre-release. That is beta.
  - Promoting a release clears the flag. That is stable.
  - GitHub's `releases/latest` ignores pre-releases, so it always means "newest stable".
- **Keep `tools/release.py`; don't adopt release-please.**
  - `release.py` already works out semver from conventional commits per app and writes release notes, and it has
    tests (`tools/test_release.py`).
  - release-please would put a release PR in front of every beta and commit a changelog to `main`.
- **No manifest.**
  - Stable: a plain-text `porthole-version.txt` release asset.
  - Beta: `porthole-version.txt` on the rolling `porthole-beta` pre-release, which the release workflow re-points at
    every new release. (Not the releases API: on the board it turned out not to list releases newest first.)
  - Integrity: HTTPS with the built-in certificate bundle, plus the image's appended SHA256, which `Update.end()`
    checks.
- **The SD card is optional, with one job:** `/porthole/wifi.txt`. Serial `W` is the other way to set wifi.
- **No downgrades over the air, and dev builds never update.** A local build reports version `"dev"`.

## Hard rules this must keep

- **Saves survive.** NVS stays at `0x9000`. The new data lives in a new namespace, `"sys"`.
- **Mark the firmware valid before anything can write a save.**
  - Every game's `loadBlob` rejects a newer save version, so a rollback after a save-format change would wipe
    progress.
  - Order in `setup()`:
    1. `enableLoopWDT()`
    2. `board::init()` and the first frame
    3. `esp_ota_mark_app_valid_cancel_rollback()` and `disableLoopWDT()`
    4. `g_shell.begin()`
  - `verifyRollbackLater()` returns true.
- **Record the attempt date in NVS before connecting**, so a crash in the download path can't trigger a retry on
  every boot.
- **Rule 4 changes wording, not intent.** Suggested: "No network traffic carries anything about the kid. Only a
  daily update check and download go out."
- **Flash is only written while the screen is dark** (decided on the board, 2026-10-17). Writing flash while a game
  was on screen didn't just stutter (34 ms frames became about 62 ms): the glass showed broken frames, because a
  flash write stalls the RGB panel's refill from PSRAM. The daily download starts only when the backlight is off and
  stops if the screen wakes, resuming in the next dark spell. Check now keeps the screen lit.
- **Root rule 7.** The "something new" card and the hidden menu use the shell's own widgets and palette, with
  nothing from any game.

## Flash layout

```
nvs,       data, nvs,      0x9000,   0x5000     # unchanged
otadata,   data, ota,      0xe000,   0x2000     # unchanged
ota_0,     app,  ota_0,    0x10000,  0x7F0000   # same start as today's factory, so make flash still works
ota_1,     app,  ota_1,    0x800000, 0x7F0000
coredump,  data, coredump, 0xFF0000, 0x10000
```

- `board_upload.maximum_size = 8323072`. The build fails once firmware outgrows a slot.
- v0.14.0 was 5.95 MB; with wifi and TLS linked in it is about 6.8 MB, which leaves about 1.5 MB of room.

## Phases

### Phase 0: spike on the board (half a day, throwaway branch)

Check each of these on real hardware:

| Check | Done when |
| --- | --- |
| Wifi plus a TLS fetch of `releases/latest/download/<asset>` follows the redirect | Body read on the board |
| Internal RAM during the TLS handshake | `M` `largest_internal` logged, enough left over |
| Flash writes from a core-0 task while a game runs | Frame times from `M` within normal range, or the fallback is chosen |
| Image carries an appended SHA256 | `esptool.py image_info firmware.bin` shows it |
| Rollback works | A panic before mark-valid boots the old slot |
| SD pins (1-bit SDMMC: CLK 2, CMD 1, D0 42, D3 on expander P3) | A card mounts after `board::init()` |

### Phase 1: release channels (CI only, 1 day)

No firmware behavior changes. It can ship before Phase 0 is finished.

- `release.yml`:
  - `gh release create --prerelease`
  - build with `-DPORTHOLE_VERSION="<ver>"`
  - upload `porthole-<ver>-ota.bin`, which is `firmware.bin`
  - upload `porthole-version.txt`
- `platformio.ini`: `PORTHOLE_VERSION` defaults to `"dev"`. The `S` serial output prints it.
- New `promote.yml` (`workflow_dispatch`, input `tag`, defaulting to the newest pre-release):
  - `git push --force origin "$TAG^{commit}:refs/heads/stable"` (force, so promoting an older tag rolls back;
    `stable` therefore gets no fast-forward-only rule)
  - `gh release edit "$TAG" --prerelease=false --latest`, after the push, so a failed push changes nothing
- Promote v0.14.0 once, by hand, so `stable` exists. It has no `porthole-version.txt`, so stable boards get a
  404 until a release built by this workflow is promoted; the firmware treats a 404 as "no update".
- README: what beta and stable mean, how to promote, and that `factory.bin` at `0x0` erases saves.

Done when a merge creates a pre-release with both new files, and running Promote flips it to Latest and moves
`stable`.

### Phase 2: wireless updates on the board (2-3 days, the last USB flash)

- `partitions.csv` with the two app slots, and `maximum_size`.
- `firmware/main.cpp`:
  - the mark-valid ordering
  - serial `W<ssid>\t<pass>` and `U` (download now), each in its own function
- `firmware/net.cpp` (new): `net::Updater`, implementing `shell::System` (`shell/system.h`); the shell owns the "something new" card and `"sys"/ran`.
  - Daily, background, low priority on core 0, starting a minute after boot.
  - Wifi off afterwards.
  - Clock sync by NTP is deferred: the device keeps local time with no timezone setting (TODO in `net.cpp`).
- `shell/`: `newer(a, b)` version compare, with a host test.
- Update CLAUDE.md rule 4 and `apps/porthole/CLAUDE.md`.

Done when:
- A board flashed once over USB with this build picks up a promoted stable release without a cable.
- It shows the card on the next power-on.
- It keeps every profile and save.
- A deliberately broken build rolls back.

### Phase 3: hidden menu and SD wifi (1-2 days)

- Hold the "Who's playing?" title for 5 s to open the menu. No code for now.
- Screens:
  - **Info**: version, slot, last reset reason, heap, wifi, SD.
  - **Wifi**: saved network and the last result.
  - **Updates**: Stable/Beta switch stored in `"sys"/chan`, running vs available version, last check, **Check now**
    with progress, then **Restart now**.
- Beta lookup: `releases/download/porthole-beta/porthole-version.txt`, the same plain-text read as stable.
- SD: mount after `board::init()`. If `/porthole/wifi.txt` differs from what's saved, save it.
- Network calls behind `board::` with host stubs. One playtest scenario across the three screens. 72 px targets
  inside the circle, and Back on every screen.

Done when `make ci` passes, the playtest is clean, and on the board a `wifi.txt` card sets up wifi and Check now
installs a beta.

### Not planned (add when a real need shows up)

- On-screen keyboard or network scan list
- Installing from SD
- Update log
- Parent code on the menu
- Recovery app plus one large slot (only if firmware nears 7.9 MB)
- Release-please (only if a `CHANGELOG.md` in the repo is wanted)
- Filtering releases by tag prefix (only when a second app publishes releases)

## Estimate

This is about a week of agent-assisted work in total:

| Phase | Estimate |
| --- | --- |
| 0 | half a day |
| 1 | 1 day |
| 2 | 2-3 days |
| 3 | 1-2 days |

- Phase 1 runs in parallel with Phase 0.
- Phase 3's screens can be built in the simulator while Phase 2 is on the board, once the `board::` signatures are
  fixed.
- Phase 2 is the critical path. It is the last USB flash, and it needs a few real power cycles to verify.

## Unverified

- Rollback with the pinned pioarduino 54.03.21 bootloader. The sdkconfig says it's enabled.
- Whether `HTTPClient` follows the cross-host redirect over HTTPS.
- Internal RAM headroom for TLS next to the display's bounce buffers.
- The SD pinout. It comes from Waveshare's wiki table, not their demo code.
