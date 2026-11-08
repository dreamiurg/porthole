# Pets Club Shelter Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** An old dog can move to a sunny farm when the kid chooses. Its picture and name hang on the room
wall, and the next dog comes from a shelter of seven named breeds, keeping its name or getting a new one.

**Architecture:** Breeds are art only: `tools/art.py` bakes every breed into `DOG_FRAMES`/`DOG_PARTS`, and
the game indexes them by `save_.breed`. The rules live in `pet.cpp` (`retire`, `adoptFromShelter`, Save v3).
New screens are appended to `Game::Screen` and reuse the existing widgets, keyboard and confirm pattern.

**Tech Stack:** C++17 (host sim + ESP32), Python art generator, `tests/playtests/*.txt` scripts.

**Spec:** `docs/superpowers/specs/2026-11-08-pets-club-shelter-design.md`

## Global Constraints

- Indexed surface: palette indices 0-31 only, 8x8 font, ASCII only (app brief constraint 2).
- Every tap target is at least 24x22 logical px and inside the circle. Every new screen has a visible way back.
- Save is append-only: new fields go before `crc`, `SAVE_VERSION` 2 -> 3, and v1 (140 B) and v2 (156 B) blobs still load.
- Pet names are at most `ui::NAME_LEN` (8) letters.
- No new sounds. The existing celebrate on a new pup's arrival may keep its usual sound.
- The farm button appears only when `pet::ageDays >= RETIRE_DAYS` (30). Nothing happens without a kid's confirm.
- The wall keeps the last `WALL_SLOTS` (4) dogs. `dogsRaised` counts all of them.
- Kept on retire: stickers, hats owned, books read/unlocked/done, streak, every counter, theme, kid fields, the
  wall. Reset: petName, adoptedAt, stage, bond, tricks, the stats (back to `adopt()` defaults), the hat worn,
  poop/dirty/asleep/want.
- Start over stays the full erase it is now.
- `make check` and `make ci` green. `COV_MIN` (98.4) is not lowered.

## Review Focus

1. A device with a v2 save updates: the dog must look and act exactly as before (breed 0 = Golden pup, wall empty).
2. A fifth retired dog: the oldest frame drops off, the newest four show in order, and `dogsRaised` is 5.
3. The kid backs out of the shelter or the name keyboard: they return to the empty room with Shelter showing,
   never to a half-adopted dog. Leaving the game there and coming back shows the empty room again.
4. The kid clears the prefilled name to nothing and presses OK: same as today's keyboard behavior (OK needs a letter).
5. A dog retires at night while asleep: the new pup starts awake, and the room isn't left in night mode with no dog.

Each line has a test in the task that owns it (Tasks 1, 1, 4, 4, 1).

---

### Task 1: Save v3 and the retire/adopt rules

**Files:**
- Modify: `games/pets-club/pet.h` (Save fields, constants, declarations)
- Modify: `games/pets-club/pet.cpp` (`loadBlob`, `retire`, `adoptFromShelter`, `canRetire`)
- Test: `host/test_pet.cpp`

**Interfaces:**
- Produces:
  - Save fields before `crc`: `uint8_t breed, dogsRaised, wallCount, hasDog; char wallNames[WALL_SLOTS][12]; uint8_t wallBreeds[WALL_SLOTS];` (sizeof(Save) == 212).
    `hasDog`: 0 on a v1/v2 load is wrong, so `loadBlob` sets it to 1 for migrated blobs. After `retire`, 0 = empty room.
  - `constexpr uint16_t SAVE_VERSION = 3; constexpr size_t SAVE_V2_SIZE = 156; constexpr int RETIRE_DAYS = 30, WALL_SLOTS = 4, NUM_BREEDS = 7;`
  - `bool pet::canRetire(const Save& s, uint32_t now);` true when `hasDog && ageDays(s, now) >= RETIRE_DAYS`.
  - `void pet::retire(Save& s, uint32_t now);` pushes `{petName, breed}` onto the wall (drops the oldest at 4), `dogsRaised++`, `hasDog = 0`, clears the dog fields listed in Global Constraints.
  - `void pet::adoptFromShelter(Save& s, uint32_t now, uint8_t breed, const char* name);` sets the dog fields as `adopt()` does, `breed`, `hasDog = 1`, keeps everything kept.

- [ ] **Step 1: Write the failing tests** in `host/test_pet.cpp`, following the shape of `saveV1`:
  - `saveV2Migrates`: build a 156-byte v2 blob (version 2, crc over 152). It loads: `breed == 0`, `hasDog == 1`, `wallCount == 0`, `streak` kept. A corrupted byte is rejected.
  - `retireKeepsKidProgress`: a save with `stickersMask = 0x5`, `hatsMask = 3`, `booksRead = 7`, `trickProgress[0] = 3`, `bond = 900`, `asleep = 1`, age 31 days. `canRetire` is true at day 30 and false at day 29. After `retire`: stickers/hats/books equal, `hasDog == 0`, `wallCount == 1`, `wallNames[0]` == old name, `dogsRaised == 1`. After `adoptFromShelter(s, now, 3, "Cocoa")`: `trickProgress[0] == 0`, `bond == 0`, `asleep == 0`, `stage == STAGE_PUPPY`, `breed == 3`, petName "Cocoa", stickers still 0x5.
  - `wallRollsAtFour`: retire five dogs named A..E. `wallCount == 4`, names B,C,D,E in slots 0..3, `dogsRaised == 5`.
  - `breedSurvivesRoundTrip`: seal, then loadBlob of a v3 save keeps `breed`, the wall and `hasDog`.

- [ ] **Step 2: Run** `make -C apps/porthole test`. Expected: test_pet fails to compile (unknown fields/functions).

- [ ] **Step 3: Implement** the fields and functions above. `loadBlob` gets a `n == SAVE_V2_SIZE` branch like the v1 one (version 2, crc over `SAVE_V2_SIZE - 4`, copy, zero the rest, `hasDog = 1`, seal), and the v1 branch also sets `hasDog = 1`. `adopt()` sets `hasDog = 1`. `retire` reuses `adopt()`'s defaults for the stats, so the two can't drift.

- [ ] **Step 4: Run** `make -C apps/porthole test`. Expected: all pass.

- [ ] **Step 5: Commit** `feat(porthole): Pets Club save v3 with breed, memory wall and retire rules`

### Task 2: Seven breeds in the art generator

Owner: `pixel-artist` (opus).

**Files:**
- Modify: `games/pets-club/tools/art.py` (`BREEDS`, `draw_dog(size, pose, breed)`, `emit`)
- Regenerated: `games/pets-club/sprites.h`
- Modify: `games/pets-club/game.cpp` (the 9 `DOG_FRAMES`/`DOG_PARTS` uses at lines 184, 191, 205, 319, 343, 846, 975, 1066, 1273)
- Modify: `games/pets-club/content.h` (shelter names)

**Interfaces:**
- Produces: `DOG_FRAMES[NUM_BREEDS][3][NUM_POSES]`, `DOG_PARTS[NUM_BREEDS][3][NUM_POSES]`, `enum Breed { BR_GOLDEN, BR_DALMATIAN, BR_HUSKY, BR_BEAGLE, BR_POODLE, BR_CORGI, BR_LAB, NUM_BREEDS_ART }` in sprites.h. content.h: `static const char* const SHELTER_NAMES[7] = {"Connor","Boxy","Romeo","Cocoa","Marsha","Waffles","Charcoal"}; static const char* const BREED_NAMES[7] = {"Golden pup","Dalmatian","Husky","Beagle","Poodle","Corgi","Black Lab"};` plus `static_assert(NUM_BREEDS_ART == pet::NUM_BREEDS)` in game.cpp.
- Consumes: `save_.breed` (Task 1).

- [ ] **Step 1:** Add `BREEDS` with the spec's art notes (colors from the existing palette; ear shape pointy/floppy/upright; Corgi's `legH` shorter with the body lowered to keep the ground line; Dalmatian spots and Poodle curls as post-outline overlays). Breed 0 must reproduce today's pixels exactly.
- [ ] **Step 2:** Run `python3 games/pets-club/tools/art.py` and confirm every `BR_GOLDEN` frame is byte-identical to the old `DOG_FRAMES` (diff the old sprites.h arrays). Review `build/art/sheet.png` at zoom: all 7 breeds x 3 sizes x 17 poses, no stray pixels, outline style matching.
- [ ] **Step 3:** Index every game.cpp use by `save_.breed` (the street house at 1273 uses that house's save breed; intro 343 and splash 319 stay `BR_GOLDEN`).
- [ ] **Step 4: Run** `make -C apps/porthole check` and `make -C apps/porthole playtest`. Expected: green, snapshots of existing dogs unchanged.
- [ ] **Step 5: Commit** `feat(porthole): Pets Club draws seven dog breeds`

### Task 3: Farm button, confirm and the farm scene

**Files:**
- Modify: `games/pets-club/game.h` (append `SC_CONFIRM_FARM, SC_FARM` after `SC_THEME`), `games/pets-club/game.cpp`
- Test: `tests/playtests/26_farm_shelter.txt` (new; started here, extended in Tasks 4-5)

**Interfaces:**
- Consumes: `pet::canRetire`, `pet::retire`, `save_.hasDog`.
- Produces: screens `confirm_farm`, `farm` (screenName values). After Go home: `SC_HOME` with `hasDog == 0`.

- [ ] **Step 1: Write the playtest** using `newgame Sam Biscuit`, a debug command `dbg old` (new: `adoptedAt -= 30 * 86400`), open stats, `expect-screen stats`, tap the farm button, `expect-screen confirm_farm`, No returns to stats with the dog kept, again Yes, `expect-screen farm`, `snap 01_farm`, tap Go home, `expect-screen home`. Plus a check that before `dbg old` the farm button isn't there (`ui` lists no farm hit region).
- [ ] **Step 2: Run** `make -C apps/porthole playtest`. Expected: FAIL at `dbg old` / the missing screen.
- [ ] **Step 3: Implement.** Stats shows a 24x22+ "Farm" button only when `canRetire`, kept clear of the long-press Start over strip (60,140,40,16) and the other stats buttons (UI audit spacing). The confirm reuses `drawConfirmReset`'s layout: "Move %s to the farm?" / "It will be happy there!", No (green) / Yes. The farm scene: sky, field, a fence and the grown dog of its breed, a wooden sign with its name in the 8x8 font (inside the chord), and the button "Go home". On entering, `pet::retire` and `markDirty()`. No sound (the buzzer is harsh, and a goodbye shouldn't fanfare).
- [ ] **Step 4: Run** `make -C apps/porthole check playtest`. Expected: green, UI audit clean.
- [ ] **Step 5: Commit** `feat(porthole): Pets Club dogs can move to the farm`

### Task 4: Empty room, shelter carousel and naming

**Files:**
- Modify: `games/pets-club/game.h` (append `SC_SHELTER`), `games/pets-club/game.cpp` (`updateHome`/`drawHome` when `!hasDog`, shelter, name step)
- Test: `tests/playtests/26_farm_shelter.txt` (extend)

**Interfaces:**
- Consumes: `SHELTER_NAMES`, `BREED_NAMES`, `DOG_FRAMES[breed][SZ_PUP]`, `pet::adoptFromShelter`, `ui::keyboard`.
- Produces: screen `shelter`. Member `uint8_t shelterSel_`.

- [ ] **Step 1: Extend the playtest:** from the empty room, `snap 02_empty_room`, the four care buttons are absent and a "Shelter" button shows. Tap it, `expect-screen shelter`, `snap 03_kennel_connor`, tap the right arrow 3 times, `snap 04_kennel_cocoa`. Back returns to home with no dog (Review Focus 3). Leave to the launcher and reopen: still the empty room. Shelter again, Pick me on Cocoa, `expect-screen name_pet` with "Cocoa" prefilled, OK, `expect-screen home`, `expect` the debug line shows `pet=Cocoa` and the breed. A second run backspaces to empty and presses OK: stays on the keyboard (Review Focus 4). Types "Choco" instead: the pet is named Choco.
- [ ] **Step 2: Run** the playtest. Expected: FAIL at the Shelter button.
- [ ] **Step 3: Implement.**
  - `drawHome` with `!hasDog`: the room, wall included, with no dog, poop, toasts or want bubbles. In place of the care buttons, one centered "Shelter" button. The back button still goes to the launcher. No daily gift in the empty room (it waits for the new pup). `pet::simulate` with `!hasDog` only sets `lastSeen = now` and returns `EV_NONE` (test `simulateEmptyRoom` in `host/test_pet.cpp`: a week later, no stage-up, no poop, stats unchanged).
  - `SC_SHELTER`: one big kennel per page (wood frame, dark doorway, the breed's pup idling inside at 2x), and a name tag under it: the name in plum, the breed in dark gray, each centered inside the chord. Left/right arrows 24x22+ wrap around 0..6, with dots for 7 pages. A "Pick me!" button. Back goes to home.
  - Pick me: `nameBuf_` prefilled with `SHELTER_NAMES[sel]`, then `SC_NAME_PET`. On OK in shelter mode, `pet::adoptFromShelter(save_, now_, shelterSel_, nameBuf_)`, `markDirty()`, then celebrate "Welcome home, %s!" -> `SC_HOME` (skip `SC_THEME`: the house keeps its colors). Back on the keyboard in shelter mode goes to `SC_SHELTER`, not the launcher. The random-name button stays.
  - The debug print adds `breed=` and `hasDog=` (and `wall=`).
- [ ] **Step 4: Run** `make -C apps/porthole check playtest`. Expected: green, UI audit clean on `shelter`.
- [ ] **Step 5: Commit** `feat(porthole): Pets Club shelter with seven named dogs`

### Task 5: The Memory Wall

**Files:**
- Modify: `games/pets-club/game.h` (append `SC_WALL`), `games/pets-club/game.cpp` (`drawRoom` frames, home hotspot, wall screen)
- Test: `tests/playtests/26_farm_shelter.txt` (extend), `host/test_pet.cpp` if any helper is added

**Interfaces:**
- Consumes: `wallCount`, `wallNames`, `wallBreeds`, `DOG_FRAMES[breed][SZ_PUP][P_SIT]`.
- Produces: screen `wall`.

- [ ] **Step 1: Extend the playtest:** after the new pup comes home, `snap 05_room_with_frame` (one small frame on the wall). Tap the frames, `expect-screen wall`, `snap 06_wall` shows "Biscuit" with its picture. Back goes to home. Add `dbg wall5` (fills 5 retired dogs through `pet::retire`) and snap the wall with four frames, oldest first.
- [ ] **Step 2: Run** the playtest. Expected: FAIL at the wall screen.
- [ ] **Step 3: Implement.** `drawRoom` draws up to 4 small gold-edged frames on the wall in the free space between the door, window and shelf, each with that breed's tiny sit pose (a 1x pup head crop is fine). The whole frame group is one hotspot of at least 24x22 px, at least 2 px from every other home hotspot and the dog's walking box (see the layout comment above `PIPS_L`). If no such spot exists, put a "Wall" button on Stats instead and keep the frames as decoration. `SC_WALL`: title "Farm friends", up to 4 framed pups at 2x in a 2x2 grid, each with its name under it inside the chord, and a back button. A wall with 0 frames never shows the hotspot.
- [ ] **Step 4: Run** `make -C apps/porthole check playtest`. Expected: green.
- [ ] **Step 5: Commit** `feat(porthole): Pets Club memory wall for dogs on the farm`

### Task 6: Docs, full gates, review and the independent playtest

**Files:**
- Modify: `games/pets-club/AGENTS.md` (Save v3 fields, the v2 migration as the new reference pattern, breeds),
  `apps/porthole/AGENTS.md` (the "never runs away, never resets" sentence gains: "except when the kid moves an
  old dog to the farm (day 30+, confirmed), which keeps the kid's progress and the dog on the wall")
- Create: `apps/porthole/docs/pets-club/shelter-playtest.md` (the persona/brief, build, findings, resolutions, evidence)

- [ ] **Step 1:** Update the docs as above.
- [ ] **Step 2: Run** `make -C apps/porthole ci`. Expected: green, coverage >= 98.4. Raise `COV_MIN` if it grew.
- [ ] **Step 3:** Run `/simplify`, then `/code-review` on the branch diff. Fix every real finding.
- [ ] **Step 4:** Dispatch an independent `playtester` (opus): an 8-year-old, a confident reader, who knows Pets Club but has never seen the farm or the shelter, plays source-free with imprecise taps: from an old dog to the farm, the wall, browsing all seven kennels, renaming, backing out midway. Fix findings and replay until `docs/playtesting.md` acceptance is met. Record it in `shelter-playtest.md`.
- [ ] **Step 5: Commit** `docs(porthole): Pets Club shelter docs and playtest record` and push.
