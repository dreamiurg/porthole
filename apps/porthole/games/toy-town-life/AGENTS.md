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

The current save is **V2, 92 bytes**, magic `0x544f5954`, ending in an IEEE CRC32
of all preceding bytes. The shipped **V1, 88 bytes** remains readable. `State`,
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
84-byte prefix into fresh V2 defaults, requires category 0–7, then validates and
reseals V2. V2 requires category 0–8, hair color 0–3 and zero reserved bytes.

Preserve `V1_SIZE`, `V2_SIZE` and their strict size/version decoders for future
schema changes. Initialize appended fields, bump version and retain the frozen
V1 fixture migration test in `host/test_toytown.cpp`; never invent a V0 record.
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
