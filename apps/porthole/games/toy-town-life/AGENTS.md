# Toy Town Life

A quiet, offline miniature neighborhood for children 6–10: choose a doll, try free
styles, pack three items and visit School, Cafe or Park. Launcher label `Toy Town`;
per-profile namespace `toytown`, keys `s<profile id>`. No sound, tilt, network,
real-world waiting, relationship score, currency grind or absence penalty.
V1 has three complete outings. Room decoration, Mall and a typed character-name
keyboard are outside this implementation; names use the four local suggestions.

Read the app's [brief](../../AGENTS.md), approved [gameplay brief](../../../../docs/superpowers/specs/toy-town-life/gameplay-brief.md)
and [design review](../../../../docs/superpowers/specs/toy-town-life/design-review.md).
The reference storyboards are inspiration, never runtime screens. Toy Town owns
its art and typography; do not borrow another game's visual assets.

## Player contract

- Use recognizable places with separate NPCs, props and a moving customized
  doll, short named speech and specific picture buttons. The approved target is
  `docs/toy-town-life/scene-interface/approved-school-reference.png` under the app.
  Keep the scene visible; do not restore the large prose panel. All five main
  School actions must retain actual illustrations above their complete labels.
  Tap-to-walk is a continuous floor interaction; named NPCs and Park landmarks
  have priority. Editors such as picture hanging use their own visible controls.
- All four doll materials have the same opportunities. Hair, skin, outfit, shoes,
  accessories, color and suggested in-world names are free choices; appearance
  never determines social approval or scenario success.
- The doll picker uses four pictured material cards. The wardrobe shows a live
  production-art doll, direct picture choices or swatches, and a selected check.
  Pictured previous/next category tabs expose nine categories in persisted order:
  Hair, Skin, Fabric, Shoes, Accessory, Color, Name, Material, Hair color. Color
  changes fabric; Hair color independently changes hair (the Plush rabbit's
  outer ears), leaving the face's skin choice intact. All options are free.
  Done returns to the doll picker; Back opens the ordinary pause/exit choices.
  Preserve pictured input, short measured labels, raised controls, immediate
  preview and the 24x22 logical target floor when changing these screens.
- A bag holds at most three of five items. Missing school paper has a borrowing
  route, so packing cannot strand a child.
- School: listening reveals Mina's concern about her first words. Sitting beside
  her, making cards and coordinating with Theo produce different visible costs;
  after-class conversation allows repair. There is no perfect moral score.
  Helping Theo includes a real editable display: choose one of three original
  pictures and place/replace it in three frames, then explicitly finish. Borrowed
  paper does not end the outing or take a bag slot. Repairs accumulate as bits
  rather than replacing each other; reflect both in the actors and display.
- Cafe: ten pretend coins per outing; sandwich 5, fruit bowl 3, water 1, cookie 2.
  Quantities are 0–3. An order needs food (sandwich or fruit), water and total at
  most ten. Compare trays freely; invalid orders keep the cart editable. Pretend
  coins never become a persistent balance or cosmetics gate.
- Park: seed selects bench, fountain or path together with its matching last-seen
  report and ribbon clue. Wrong guesses can be revised and discussed with Nori.
- An outing offers Keep memory, Add details or Skip. Details cycles the caption
  and sticker on a separate page and returns to the same decision. Skip completes
  the outing without adding or replacing any scrapbook entry. Repeated completion
  is idempotent. Keep the newest eight memories in order.
  Stop and Continue are both ordinary, easy choices. Exit preserves the exact
  page and selections, even during dialogue or after shell-enforced rest.

## Modules and saves

`state.h` contains deterministic rules, state validation and the serialized
record; `content.h` owns authored dialogue; `game.*` owns input/navigation and
`rendering.cpp` draws the native RGB565 surface. Assets and their reproducible
build sources live under `art/`, `tools/` and `generated/`.

`scene_input.cpp` handles walking, named NPCs and spatial clue inspection;
`scene_hang.cpp` owns the display editor; `scene_copy.cpp` keeps dialogue short;
`scene_render.cpp` and `scene_pictures.cpp` compose scenes and action art using
shared `scene_ui.h` geometry. Preserve card illustrations, clear literal actions,
full native text, visible outcomes and exact editor resume when changing them.

The current save is **V3, 100 bytes**, magic `0x544f5954`, ending in an IEEE CRC32
of all preceding bytes. The shipped **V1, 88 bytes** and **V2, 92 bytes** remain readable. `State`,
`Entry` and the persisted `Place`/`Page` ids are append-only from release. Never
reorder, resize or repurpose existing fields, ids or arrays. Reject incorrect
size, magic, version, CRC and out-of-range data without partially replacing the
destination state. The seed is any uint32; wrap is intentional. No elapsed-time
field drives rewards or deterioration.

V2 preserves every V1 field through `seed` (bytes 0–83), then appends `hairColor`
at byte 84 and three zero reserved bytes before the CRC at byte 88. `hairColor`
is independent of the hair shape and outfit `color`: 0 retains the original
hair tint, with choices 1–3 added. Style category 8 is appended after the V1
categories 0–7. V1 validates its own exact header and CRC before copying its
84-byte prefix into fresh defaults, requires category 0–7, then validates and
reseals the current version. V2 requires category 0–8, hair color 0–3 and zero reserved bytes.

V3 keeps the complete V2 prefix through its three reserved bytes and appends
`worldX`, `worldY`, `schoolPaper`, `worldPlace` at bytes 88–91, then `schoolHangMask`, `schoolPicture`, `schoolPictures`, `schoolTask` at bytes
92–95 and CRC at byte 96. Logical walking bounds use shared `WORLD_MIN_X`/`WORLD_MAX_X` and
`WORLD_MIN_Y`/`WORLD_MAX_Y` constants; fresh and migrated records stand at
(80, 80), Home. `moveTo` validates before changing position; commit movement
on arrival. Starting an outing places the doll at (80, 80) in its destination
and resets borrowed paper. `borrowPaper` supplies cue cards without using a bag
slot or completing School; `hasPaper` recognizes packed or borrowed paper.
`CAFE_QUANTITY` is appended as page 27 and `SCHOOL_HANG` as page 28.
The display tracks occupied slots with a three-bit mask, selection 0–2, three
two-bit picture choices (each 0–2), and task 0 (none), 1 (initial Theo help),
or 2 (Theo repair). `beginDisplay` reopens the display without clearing its
pictures or selection; `hangPicture`
places or replaces a picture without completing the activity. Only explicit Done
uses `finishDisplay` once all three slots are occupied. Starting a new outing
clears every display field. School repairs use additive bits 1 (Theo) and 2
(Mina), retaining the shipped single-repair meanings; `repairSchool` never
undoes an earlier repair. Historical records reject the new combined value 3. Both historical loaders reject page or
resume ids above 26, including a correctly checksummed new quantity page.

Preserve `V1_SIZE`, `V2_SIZE`, `V3_SIZE` and their strict size/version decoders for future
schema changes. Initialize appended fields, bump version and retain the frozen
V1 and V2 fixture migration tests in `host/test_toytown.cpp`; never invent a V0 record.
Unknown future versions must not be misread as V1 or V2.

## Verification

`host/test_toytown.cpp` proves cart arithmetic/eligibility, consequences,
deterministic clue coherence, exact-once completion, bounded memories and save
rejection/roundtrips. `host/test_toytown_ui.cpp` exercises the App contract,
ordinary touch lifecycle, navigation, interruption, save isolation and render/UI
geometry, direct pictured selection and exact production-art thumbnail fidelity. These are supplementary to the required source-free black-box emulator
playthrough in [playtesting.md](../../docs/playtesting.md). Parent app `make check`,
`make ci`, firmware compile and final-build screenshot/input evidence are the
acceptance gates; report emulator-only scope honestly.
