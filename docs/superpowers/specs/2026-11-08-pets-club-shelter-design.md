# Pets Club: the Farm, the Memory Wall and the Shelter

Status: **draft. A grown-up must approve it before any code is written** (see "Needs a grown-up's decision").
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
| 3 | Chocolate | Beagle | tan, black saddle, white belly and tail tip, long floppy ears |
| 4 | Marsha | Poodle | cream, curly puffs on head, tail tip and paws |
| 5 | Waffles | Corgi | orange and white, short legs, big upright ears |
| 6 | Charcoal | Black Lab | black coat, dark gray outline accents |

Every name fits `ui::NAME_LEN` and the 8x8 font.

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
  and **Go home**. One rare sound here is fine (a goodbye moment), through the existing `Sound` enum.
- `SC_SHELTER`: a carousel with one big kennel per page and left/right arrows, because seven name tags do
  not fit on the round screen at once in the 8x8 font. The tag reads "Chocolate" / "Beagle". The puppy
  inside idles. **Pick me!** goes to the name step. Back returns to the empty room.
- Name step: reuse the existing pet-name keyboard (`SC_NAME_PET`), prefilled with the shelter name. OK
  keeps it, editing changes it.
- Room: the Memory Wall frames hang on the wall. Tapping a frame shows a toast: "Connor lives on the farm".
- The room without a dog shows only Shelter (and the way back). Every new screen has a visible way back.

**Where the farm button lives.** On the Stats screen next to the existing long-press "Start over", shown
only once the dog is old enough (decision 1). It asks once, "Move Connor to the farm?", Yes / No, like the
reset confirm.

## Needs a grown-up's decision

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
