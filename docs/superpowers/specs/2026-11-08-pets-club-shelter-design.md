# Pets Club: the Farm, the Memory Wall and the Shelter

Status: **approved** by her dad on 2026-11-08, with all four proposals in "Grown-up decisions" accepted.
Designed with an 8-year-old player in a chat session on 2026-11-08. Her words and choices are kept below.

## What she asked for

> "Shelter and choose your next dog. There would be like different breeds."

When she gets her next dog, she picks it from a shelter. There are several breeds, and she can pick the
same breed as her last dog or a different one. Her first idea was a funeral and a grave, but Pets Club's
hard rule is that nothing dies, so she chose a gentle goodbye instead: option C, the farm plus a picture on
the wall.

## The flow (her final plan, confirmed "correct and perfect")

1. When the dog is very old, a **Move to the farm** button appears. The dog leaves only when the kid
   presses it. Nothing happens by itself, and time away never causes it.
2. **Farm screen**: the dog on a sunny farm next to a wooden sign with its name. One button: **Go home**.
3. **Memory Wall**: the dog's picture and name go into a frame on the room wall and stay there.
4. Back home, the room has no dog yet, and a **Shelter** button is shown.
5. **Shelter**: little kennels, one dog in each, each with a name tag showing the dog's name and breed.
6. She picks a dog. It already has a name, which she can **keep** or **change** (any name, including her
   last dog's).
7. The new puppy comes home and the game goes on as usual.

## The seven shelter dogs (named by her)

| # | Name | Breed | Art notes |
|---|---|---|---|
| 0 | Connor | Golden pup | today's dog, unchanged (tan, brown ears) |
| 1 | Boxy | Dalmatian | white coat, black spots, black ears |
| 2 | Romeo | Husky | gray back, white belly and muzzle, pointy ears, blue eyes |
| 3 | Choco | Beagle | tan, black saddle, white belly and tail tip, long floppy ears |
| 4 | Marsha | Poodle | cream, curly puffs on head, tail tip and paws |
| 5 | Waffles | Corgi | orange and white, short legs, big upright ears |
| 6 | Charcoal | Black Lab | black coat, dark gray outline accents |

`ui::NAME_LEN` is 8 letters, and every name fits. She picked "Chocolate" (9 letters) for the beagle; it
becomes **Choco**, her pick, the same idea in 5 letters. As her dad asked, shelter names are suggestions the kid
keeps or changes on the existing name keyboard.

## Design

**Breeds are art, not rules.** A breed changes only how the dog is drawn. Stats, tricks and growth are
the same for every breed.

- `tools/art.py` grows a `BREEDS` table (coat, belly, ear colors, ear shape, leg length, spots, curls)
  on top of the existing parametric rig, using palette colors only. It emits `DOG_FRAMES[breed][size][pose]`.
  Baking every breed at build time keeps the drawing code as it is and adds no runtime recoloring.
  Cost: about 7x today's dog frames in flash, well within the 7.9 MB app slot.
  Owner: `pixel-artist`.
- The first dog of a new profile stays as today: the parcel intro, a Golden pup, the kid names it.

**Save (append-only, `SAVE_VERSION` 2 -> 3).** New fields go before `crc`:

- `uint8_t breed`: 0 for every existing save, which is the Golden pup they already have.
- `uint8_t wallCount` plus the Memory Wall entries: name `char[12]` + breed `uint8_t`, a fixed small
  number of slots (see decision 3).
- `uint8_t dogsRaised`: lets the game say "your 2nd dog" and drives a possible sticker later.

`loadBlob` accepts v1, v2 and v3. A v2 blob zero-fills the new fields. Round-trip tests in
`host/test_pet.cpp`.

**What carries over to the new dog.** The kid's progress stays: stickers, hats, the library and books read,
streak, totals. The dog's own progress starts fresh: stage, age, hearts/bond, tricks, stats and the hat it
wears. Nothing the kid earned is lost.

**Screens** (`game.cpp`, new `Screen` values appended):

- `SC_FARM`: a farm scene (field, fence, sun) with the dog in its grown size, a wooden sign with its name,
  a line saying "Connor lives here now!", and **Go home**. One rare sound here is fine (a goodbye moment),
  through the existing `Sound` enum; the build plays none. Tapping the dog pats it: it jumps and a few
  hearts float up, with no sound and nothing saved, both here and on a visit. (Added after the second
  playtest, 2026-11-09: the visit was a still picture.)
- `SC_SHELTER`: a carousel with one big kennel per page and left/right arrows, because seven name tags do
  not fit on the round screen at once in the 8x8 font. The tag reads "Choco" / "Beagle". The puppy
  inside idles. **Pick me!** goes to the name step. Back returns to the empty room.
- Name step: reuse the existing pet-name keyboard (`SC_NAME_PET`), prefilled with the shelter name. OK
  keeps it, editing changes it.
- Room: the Memory Wall frames hang on the wall. Tapping them opens **Farm friends** (`SC_WALL`): each dog's
  picture with its name over it. Every picture sits below the room's frames, so the second tap of a slow
  double tap on the frames stops on Farm friends instead of visiting a dog. Tapping a picture visits that dog on the farm (the same `SC_FARM` scene, its name
  and breed from that frame) with the back button to Farm friends instead of Go home. (Revised after the
  first playtest, 2026-11-09: this replaces the toast, which answered "can I see him?" with words only.)
- The room without a dog shows only Shelter (and the way back). Every new screen has a visible way back.
  Decided: the empty room also hides stickers, hats and Start over (they live on the dog's Stats page) until
  the next pup is adopted; nothing is lost, it all comes back with the new pup.

**Where the farm button lives.** On the Stats screen next to the existing long-press "Start over", shown
only once the dog is old enough (decision 1), in a calm color so it is not the page's call to action. It
asks once, "Move Connor to the farm?" and says what happens: "Connor will live there. Then pick a new pup."
No (green, the easy one) / Yes. Yes takes a 2 s press-and-hold: the button fills across the whole 2 s
while the little dog gets up and walks off to the right, and "Hold Yes" sits under the buttons. Letting go
or sliding off starts over; a plain tap on Yes only wiggles the hint. (Added after the second playtest,
2026-11-09: a random masher reached the farm in three taps, 4 runs in 40. Lengthened from 600 ms after the
third: a firm 6-year-old's ordinary press sent the dog away before she could notice the fill.) The move,
and later the shelter's adoption, are saved within a second, so a restart right after keeps them.

## Grown-up decisions (approved: every proposal below)

1. **This bends a hard rule.** The app brief says the pet "never runs away, and never resets". In this
   design the dog leaves only when the kid chooses, like the existing Start over, but gentler: the dog is
   kept on the wall and the kid's progress stays. Is that acceptable?
2. **When is a dog "very old"?** Today a dog is Grown at day 10. Proposal: the farm button appears at day
   30, so it can't be the first thing a kid does with a new dog. Alternatives: at Grown (day 10), or never
   before hearts are full.
3. **How many frames on the wall?** "Forever" vs. room space. Proposal: keep the last 4 dogs (the wall
   fits about 4 small frames), with `dogsRaised` remembering the total.
4. **Start over.** Keep it as is (erases everything, back to the parcel), or send it to the Shelter too?
   Proposal: keep it as is. It's the grown-up's full reset.

## Verification

- `make check` and `make ci` (coverage threshold unchanged or raised).
- Unit tests: v2 -> v3 load, carry-over on farm (stickers kept, tricks reset), wall slots fill and roll,
  breed survives save/load.
- `make art --check`, sheet review of all 7 breeds x 3 sizes x every pose.
- Playtest scenario for farm -> wall -> shelter -> rename -> new puppy.
- Independent `playtester` pass as a 7-8-year-old, source-free for the new screens, per
  `apps/porthole/docs/playtesting.md`.
