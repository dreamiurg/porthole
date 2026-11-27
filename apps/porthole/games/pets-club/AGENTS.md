# Pets Club: game brief

The shared app brief (`apps/porthole/AGENTS.md`) covers what every game shares: the
round-screen and tap-target rules, the append-only save-layout rule, the buzzer, and the
per-app workflow. This file has what's specific to Pets Club: the module map, the `Save`
layout, and the age-based content tables.

## Module map

- `pet.h` / `pet.cpp`: the simulation. Owns the `Save` struct (below), stat decay and
  floors, growth stages, gifts, stickers, trick progress, and `wordLenRange` (below). No rendering, no platform calls -- `host/test_pet.cpp` exercises it directly.
- `game.h` / `game.cpp`: the screens. One `Game` class (an `App`), a `Screen` enum, one
  `update`/`draw` pair per screen, input handling, toast/particle/sound selection, and save
  hand-off back to the shell via `takeSave()` (never an empty save: with no pup there is
  nothing to write). `wantsHome()` reports whether the game wants
  the shell to show the launcher instead of resuming mid-screen. `pressMs_` is when the
  current press began, and `pressedAfter(ms)` holds a tap to a press begun at least `ms`
  after `screenMs_`: a screen that changes in place (Fetch's and Word Fetch's results and each next word, the
  lesson's "Your turn!") resets `screenMs_`, so a finger already down never answers on its lift.
- `content.h`, plus its extensions `content_level3_books.h`, `content_books_en.h` (English levels 2-4 and 8) and
  `content_level3_words.h`: English story text (`Book`) and spelling words (`WordClue`); `content_books_uk.h`:
  the Ukrainian books (`UK_BOOKS`, gate `tools/check_books_uk.py`). Plain data, no logic. `content.h` pulls the level-3 files in via `#if __has_include(...)`; the
  include-guard string in each extension file has to match its filename exactly -- a
  mismatch silently compiles the extension out, with no error and no warning at build time.
- `books.h`: the two bookshelves, English and Ukrainian (below, "Books and reading levels"). Pure logic;
  `host/test_pets_books.cpp` measures every Ukrainian string in Andika in the box the reader draws it in.
- `learn.cpp`: Puzzles and Little reads, from the Play menu (moved from Biscuit on 2026-11-16; Biscuit keeps
  its stories and discoveries). Puzzles: three strands (plus and minus, times, words) at the kid's own level per strand
  (`puzzles.h`, design `docs/design/adaptive-practice.md`), five questions a round, a miss comes back on a later
  day (`review.h`) with how to solve it. Little reads: three short passages a round, paged like a book, one question
  each, the level moving 3-up/1-down (`docs/design/learning-principles.md`). A right answer adds `LEARN_BOND` and
  is kept in the save; every `PUZZLES_PER_STAR` (5) right puzzles or `READS_PER_STAR` (3) right reads is a star in
  the kid's balance (`os/stars.h`), which the Shop spends (below). Content: `content_puzzles.h`
  (spelling words, clues, times stories' things) and `content_reads.h` (at least 8 reads per level 1-5), both in
  `namespace learn` and gated by `tools/check_learn.py`; rules tested in `host/test_pets_learn.cpp`; playtest
  `tests/playtests/0a_puzzles_reads.txt`. `debugCmd rich` adds 50 stars.
- `ukrainian.cpp` / `ukrainian.h`: Ukrainian words, the Puzzles screen's fourth button (design page approved
  2026-11-21). Rounds of five; three exercises take turns: a word's meaning (three English choices), a lookalike
  (the English, three Ukrainian words a letter apart: кіт / кит / кут) and syllables tapped in order with one decoy
  (со-ба-ка; two from step 4, `DECOY_STEP`). Sentences ("Sentence builder" page approved 2026-11-22): once a kid
  reads words at step 3 (`SENTENCE_FROM`), two of the round's five places are a sentence whose translation is built
  from tiles tapped in order among decoys; a tile that can come next in any right order (`alt`) goes into the line
  under it, one needed later flashes, a decoy goes dark red and off. Steps 1-3 build English, 4-5 Ukrainian among
  wrong forms of the same words. Rated by its own skill `READ_UK` (no miss right, one miss slow, more wrong). The 60
  sentences are hand-written in `sentences_uk.h`; `sentence_layout.h` places them in the circle (Andika 36, tiles
  24x22, up to three rows) and the test checks every one fits. Colors, not sentences, say how it went (redesign approved 2026-11-22): a right answer turns green and the
  next question comes after a second; a miss turns the screen pink a moment, the option stays dark red and off, the
  right one is not shown, and the kid tries again. Only the first answer rates the word. Five dots stand for the round
  (green first try, orange found later); an exercise's line ("Which word means") shows the first time it comes in a
  visit; the round's end is its stars drawn as stars and an arrow for another round. The rating, the calibration and the stars are the shared engine's
  (`os/skills.h`, skill `WORDS_UK`, through `AppEnter::book`): a word's step s (1..7) is level s - 1, plus
  `KIND_OFFSET` per exercise; `READ_MS` tells a guess, `SLOW_MS` a slow read. Words asked lately are kept out of the
  next picks (a RAM ring of 16), and a missed word comes back as a meaning on a later day (on the puzzles' review queue,
  `review.h`, as strand `REVIEW_STRAND`, so the save needs no new bytes). The screen is RGB565
  (`surface()`): the frame and English are the indexed art, expanded, then the Ukrainian is Andika Bold
  (`os/uk_fonts.h`, OFL, 24/36/48/88 px, from `tools/fontconv.py --game uk`) drawn on top. Content `content_uk.h` is
  written by `tools/uk_words.py` (the word list, each word's step, the lookalike sets; `--check` in lint). Events
  `uk_answer`, `uk_sentence`, `uk_calibrated` and `level_finished` go out behind Usage stats; what tunes what:
  `docs/superpowers/specs/2026-11-21-ukrainian-telemetry.md`. Tested in `host/test_pets_uk.cpp`; playtest
  `tests/playtests/b4_pets_ukrainian.txt` and `b5_pets_uk_sentences.txt` (`answer-uk right|wrong`);
  `debugCmd ukmeaning|uklook|uksyl X` shows the word meaning X as that exercise, `uksent N` sentence N.
- The Shop (Stats > Shop, `SC_SHOP`): a hub with a tile a shelf, Hats (`SC_HATS`) and Collars (`SC_COLLARS`), one
  page for both (`updateWear`/`drawWear`): arrows flip the shelf while the sitting dog tries each thing on, a tap
  wears what the kid owns, and buying is a 1 s hold on Buy that fills it plum (the farm's Yes pattern, shorter; a
  tap or an early let-go buys nothing and wiggles "Hold to buy"), then a celebration. A tap on a Buy the kid can't
  afford opens the "N more" panel (any tap closes it). Prices are `HAT_PRICES` and `COLLARS` (`content.h`) through
  `shopPrice`, whose one knob `PRICE_PCT` follows the stars engine's daily rate. Stars are read and spent only via
  `stars::balance`/`stars::spend` on `wallet_`. Collars are recolors: `tools/art.py` paints every dog frame (the
  puppy too) a 4x2 `C_RED` band and a `C_GOLD` tag, colors no dog uses otherwise, and `drawDog` repaints those
  pixels with the worn collar (Stripes alternates columns; the Gold Collar's tag twinkles). Red is everyone's and
  free; Bell needs the Best Friends sticker and Gold the All Grown Up one. Bought things are the kid's: a new pup
  keeps them. Playtest `tests/playtests/0b_shop.txt`; `debugCmd collars` owns every collar.
- `sprites.h`: AUTO-GENERATED by `tools/art.py` (every breed's dog frames, below). Never
  hand-edited; its own header says so.
- `tools/art.py` (parametric dog rig + ASCII icons -> `sprites.h` and `build/art/sheet.png`)
  and `tools/check_content.py` (the content gate; see the `content` skill); `tools/check_learn.py` gates the
  puzzle and reads content (spelling, letter patterns and word counts per level).

## Save layout

`Save` (`pet.h`) is a raw byte blob, one per profile, NVS namespace `"crago"`, key
`s<profile id>`, written by the shell. Magic `0x4F475243` ("CRGO"), `version`, `size`,
fields, then `crc` last -- the append-only rule in the app brief applies exactly here.

`SAVE_VERSION` is currently 5 (320 bytes). v5 appended, before `crc`: `booksEn[8]` and `booksUk[8]` (a bit per book
read, by its index in `BOOKS` / `UK_BOOKS`; a v4 blob's `booksDoneMask` is copied into `booksEn` on load and is no
longer written; `booksUnlocked` is no longer read) and `reserved4[16]`, zero, kept for the Shop's room and toys; its first
half became `booksEnMore[8]` (English books 64..127, 2026-11-27), always zero before, so no new version, and
`reserved4[8]` is left for the Shop. It
took one more NVS entry: the profile budget in `host/test_nvs_budget.cpp` went from 78 to 79, which still fits four
kids with 13 entries to spare; don't raise it again without a plan for where a fifth kid's save goes. The Shop took v4's reserved bytes, which were always zero, so it needed no new version: `collar` and
`collarsOwned` (once `reserved0/1`: the collar worn, 0 = Red, and a bit per bought collar), `lastBuyDay` (once
`reserved2`) and `lastBuyItem` (half of the old `reserved3`, for a later same-day Give back: hat i, or 0x80 | collar i).
One reserved byte is left. v4 appended the kid's learning, before `crc`: `puzzleLevels[3],
puzzleReviewCount, readLevel, readStreak, readsRight, puzzlesRight, reserved3 (2 bytes), puzzleReviews[8]` (a v3 blob loads
with every strand never played and nothing to review; a new pup keeps them, they are the kid's). v3 appended, before `crc`: `breed`,
`dogsRaised`, `wallCount`, `hasDog` (one byte each), `wallNames[WALL_SLOTS][12]` (the same
size as `petName`; a `static_assert` in `host/test_pet.cpp` holds them equal) and
`wallBreeds[WALL_SLOTS]`.

The v1/v2 -> v3 migration is the reference pattern for any future bump. Each older layout
has a size constant that is never changed again: `SAVE_V1_SIZE = 140` (before the house,
reading-level and turn-taking fields) and `SAVE_V2_SIZE = 156` (before breed and the
wall). `pet::loadBlob` takes a blob of exactly the current size as is; a blob of an older
size must carry that size's own version (a 140-byte blob claiming v2 is rejected), and its
CRC over everything up to `size - 4` must hold. It then copies those bytes (every older
field keeps its offset), zero-fills the rest and fixes up any field whose zero would be
wrong: here `hasDog = 1`, since every save before the farm has its dog. Zero then means
"not asked yet" for `kidAge`, "none" for `pin`, a Golden pup (`breed` 0) and an empty
wall. `host/test_pet.cpp` builds each older blob with `oldBlob()` and has a case per
version (`saveV1` ... `saveV3Migrates`): fields kept, new fields zero, a corrupt CRC and a
wrong version both rejected. To add a field: put it immediately before `crc`, bump
`SAVE_VERSION`, add a `SAVE_V5_SIZE = 320` beside the others (`SAVE_V4_SIZE = 288` is the last one so far), extend `loadBlob`, and copy
those test cases. Never touch an existing size constant or reorder/resize a field.

## Breeds

A breed changes only how the dog is drawn; stats, tricks and growth are the same for all.
`BREEDS` in `tools/art.py` (order = `enum Breed` = `Save::breed`: Golden pup, Dalmatian,
Husky, Beagle, Poodle, Corgi, Black Lab) layers coat, belly, ears, legs, spots and curls
on the one parametric rig, in palette colors, and bakes every breed into `sprites.h` as
`DOG_FRAMES[breed][size][pose]` (with `DOG_PARTS` the same shape, for the hat and mud
anchors), plus the wall's pictures cut from each breed's sitting pup: `DOG_PORTRAIT_SMALL`
(the room's 10x10 head) and `DOG_PORTRAIT` (the wall screen's 22x14). `pet.h`'s `NUM_BREEDS` must equal the art's `NUM_BREEDS_ART` (a `static_assert`
in `game.cpp`), and so must the two name tables in `content.h`: `SHELTER_NAMES` (the
suggested names, at most `ui::NAME_LEN` letters) and `BREED_NAMES`. `pet::loadBlob`
makes the v3 bytes safe once, for every save it loads: a `breed` or `wallBreeds` byte past
the table becomes 0 (the Golden pup), `wallCount` is capped at `WALL_SLOTS`, `hasDog`
is 0 or 1, a `hat` past `NUM_HATS` is no hat, a `collar` past `NUM_COLLARS` is the Red one and a `stage` past `STAGE_GROWN` is grown, so every
draw indexes the art with them directly. A new profile's first dog is still the
parcel intro's Golden pup.

## The farm, the shelter and the wall

The one way a dog leaves, and only on the kid's yes (a parent decision of 2026-11-08):

- **The farm.** `pet::canRetire` is true once the dog is `RETIRE_DAYS` (30) days old. An
  old dog's Stats page then swaps its four stat-pip rows (home still shows them) for "To
  the farm" (sky blue: calm, not a call to action), which asks "Move <name> to the farm?"
  and says "<name> will live there. Then pick a new pup." (`SC_CONFIRM_FARM`, a green No! /
  Yes through `askNoYes`/`drawNoYes`). Yes sends the dog away, so it
  takes a 2 s hold (`YES_HOLD_MS` in `game.cpp`; `yesHold_`, a `ui::Hold` read with
  `progress(YES_HOLD_MS)`) that fills the button plum across the whole 2 s while the sitting
  dog gets up and walks 12 px right,
  toward a little red barn at the card's right edge (the picture says "farm" for a
  non-reader), with "Hold Yes" under the buttons. Letting go or sliding off starts over,
  and a press that starts on No! never counts; letting go early (a tap or up to 2 s) only
  wiggles the hint (a random masher got there in three taps, and a firm 6-year-old's press
  in `ui::HOLD_MS`). Every other button on the farm's, the shelter's and Farm friends'
  screens, "To the farm", the empty room's Shelter and the room's wall frames acts on
  release however long it was held (`ui::clicked`, for a box or a circle; the back button
  through `backClicked()`, the kennel arrows through `arrowClick`), and the shelter's
  "Welcome home" card ends on any release (`celebAnyRelease_`), so a kid whose long press
  "didn't work" on No! is never steered to the long press that Yes rewards. The
  hold calls `pet::retire` and shows `SC_FARM` (the dog in the sun under a sign with its
  name, "<name> lives here now!", "Go home"); no sound. It also clears the room's window
  event and rain, which only run with a dog. A finger on the farm's dog, on a visit too (a
  tap, a hold or a rub), makes it jump, with a heart every 300 ms and the jump pose for each
  heart's first 200 ms: no sound, nothing saved (`bounceUntil_`; `go()` ends it and resets
  `yesHold_`).
- **No Start over** (a parent decision of 2026-11-09): the old hidden long press under Stats' last row, whose Yes
  erased the pup, is gone, since a 6-year-old did it with two ordinary holds. A new dog comes from the farm and
  the shelter, and a full reset is deleting the profile, which takes a code. Pets Club never erases its save
  (`takeSave` writes only with a pup). `tests/playtests/12_no_start_over.txt` holds that presses on the old strip
  do nothing.
- **Bring back.** A habit longer than any hold still gets through, so the move can be undone until a new pup
  is adopted. The farm right after a move (not a visit) shows "Bring back" (`BRING_BACK_BTN`, orange, two lines,
  left of the dog where No! was; the dog steps right), and so does the empty room (`ROOM_BRING_BACK_BTN`, left of
  the Shelter, with the newest farm dog's little picture on it), after Go home, the launcher or a restart too:
  both whenever `pet::canBringBack` (no dog, a frame on the wall), never on a visit (`farmBringBack()`). The room's
  Bring back takes only a press that began once the room had shown for a second (`pressedAfter(1000)`): taps
  from Paw Street's own house, the splash pup and a sticker card land where it sits, and a slow double tap's
  second press gets past the fresh gate, however long it is then held. It calls
  `pet::bringBack`, marks the save dirty and urgent, goes home to the dog and says "<name> is home!" with a few
  hearts; no sound. The move, its undo and the shelter's adoption are saved at once: `saveUrgent()` (`urgent_`)
  asks the shell for its one-second checkpoint, so a restart right after either keeps it. Time away never
  retires a dog.
- **What `retire` keeps and resets.** Kept: stickers, hats owned, books read/unlocked/done, streak, every
  counter, theme, the kid's fields, the wall, and the dog's own fields (`petName`, `breed`, `adoptedAt`, stage,
  bond, tricks, the stats, `variant`, the hat worn), which wait for `pet::bringBack`. Cleared, so the empty room is
  calm and in day: asleep, poop, dirty and want. The dog goes onto the wall, `dogsRaised` counts it (all of
  them, ever, up to 255), and `hasDog = 0`. `pet::bringBack` takes the newest frame down, `dogsRaised--`,
  `hasDog = 1` and `lastSeen = now` (the empty room's time is not time away; a poop already due is moved on).
  The full-wall caveat: a move onto a full wall (four frames) took the oldest frame down, and Bring back does
  not hang it again. `pet::adoptFromShelter` replaces the old dog's fields through `freshDog`, the same
  defaults `pet::adopt` uses (no hat, and today's cookies and pats its own), so from then on the move stands.
- **The empty room** (`hasDog == 0`, until `pet::adoptFromShelter`): `pet::simulate` only
  moves `lastSeen`; nothing grows, decays or poops, and with no dog asleep there is no night tint. Nothing
  else reads the kept dog's fields while `hasDog` is 0 (the room, the gift, growth and Paw Street all check it).
  Only the Shelter and Bring back buttons, the front door (Paw Street, whose house shows no pup) and the
  wall work; the care buttons, the pips, the gift and the dog are gone (a sticker earned
  there celebrates with no dog). Stickers and the Shop live on Stats, so they
  can't be reached from an empty room until the next pup is adopted (decided). The Shelter
  and Bring back buttons sit higher than the farm's Go home and the shelter's Pick me!, so a
  finger tapping one spot a little slower than the fresh gate never chains into naming or bringing the dog back.
- **The shelter** (`SC_SHELTER`): one kennel a page, arrows left and right (wrapping; like the library's and the
  hats' they sink and turn brown while a press that began on one is held),
  starting on Connor each time it opens from the room, a tag with the pup's suggested name and breed, "Pick me!".
  Picking opens the pet-name keyboard (`SC_NAME_PET`) prefilled with that name: OK keeps
  it, editing changes it, an empty name can't be OKed, and back returns to the kennels.
  OK calls `pet::adoptFromShelter` (a fresh puppy of that breed) and celebrates "Welcome
  home". Back from the shelter is the empty room; nothing is adopted until OK.
- **The memory wall.** The wall keeps the last `WALL_SLOTS` (4) dogs, oldest first; a
  fifth retire takes the oldest frame down. In the room the frames hang between the door
  and the window (the hotspot `WALL`, live whenever `wallCount > 0`, empty room included)
  and open `SC_WALL`, "Farm friends": each pup at 2x with its name over it, fitted to 64
  px. Every picture sits below `WALL`, so the second tap of a slow double tap on the
  frames stops on Farm friends instead of visiting a dog. A picture visits that dog on the
  farm (`SC_FARM` for that slot, `farmVisit_`): the back button there returns to Farm
  friends.

Debug (see `docs/playtesting.md`): `dbg old` ages the dog to `RETIRE_DAYS`; `dbg wall5`
puts five shelter pups, Romeo to Charcoal (the widest name), through the farm and keeps
the kid's dog; `dbg breed N` sets the breed (past the table: the Golden pup, as a load would). `debug` prints `breed=`,
`hasDog=`, `bringback=` (`pet::canBringBack`) and `wall=`, on the wall screen `oldest=` / `newest=`, and on the farm `farm=` / `visit=`. The scenario is
`tests/playtests/b0_pets_farm_shelter.txt`.

Since Porthole, the shell's profile owns the kid's name, age, secret code, mute and rest
budget. `Save`'s own `kidName`/`kidAge` are copied in from the profile on every entry
(`Game::enterHouse()`, `game.cpp:54`) -- the word-fetch screen
(`game.cpp:731`) read `save_.kidAge` directly, so it has to be kept current, not treated as
migration-only. `muted`, `pin`, `restUntil` and `playSec`, by contrast, are read exactly
once, by the shell's migration (`shell/migrate.cpp`) converting an old Pets Club house into
a profile; after that the profile is the source of truth for them and `game.cpp` doesn't
touch them again. All of it stays in the struct because the layout is append-only.

## Books and reading levels

Two shelves in the library, English and Ukrainian, each with its own reading level from the shared skills engine
(`os/skills.h`): `READ_EN` (Little reads move it too, and a new kid's starts where `readLevel` stands) and `READ_UK`
(starts at level 1, calibrating). `books::shelf` (`books.h`) fills a shelf of up to 8 covers: the first unread book
at the kid's level (or the nearest level with one, above first), one unread a level up, one a level down, then books
already read from two below to one above, newest first; it is never empty. Paws on a cover: 1 easy, 2 just right,
3 a stretch. The question at a book's end is one answer to the engine, weighted by the book's length (one per 50
words, up to 6, `skills::Item::weight`); a book paged faster than 150 ms a word counts as read but rates and pays
nothing. The first read of a book pays stars at the engine's rate. Telemetry: `book_finished`.

| Shelf | Levels | Words per book | Page font |
| --- | --- | --- | --- |
| English | 2-10 (the first 30 books, once levels 1-3, are now 5-7; `content_books_en.h` adds 2-4, 8, and 9-10 retold from public-domain classics: denser words and an inference question, the length capped by the screen like 8) | `check_content.py` | 8x8, 122 px box |
| Ukrainian | 1: 15-40, 2: 40-80, 3: 80-150, 4: 140-220, 5: 200-300 | `check_books_uk.py` | Andika 48 px for 1-2, 24 px for 3-5 |

Ukrainian level 1 has no я ю є ї or apostrophe; the font has no em dash or guillemets, so dialogue uses " - " and
straight quotes, and the apostrophe is ʼ (U+02BC). Debug: `dbg read uk5` marks book 5 read, `dbg readlevel uk5`
puts the rating at level 5 (`en` the same); `debug` prints `shelf= level= covers= book_level= answer_y=` on the
cover picker and the reader. Playtest: `tests/playtests/05_read.txt`.

## Word length by age

`pet::wordLenRange(age, lo, hi)` (`pet.cpp`) drives the spelling round:

| Age | Spelling word length |
| --- | --- |
| <= 6 | 3-4 |
| 7 | 4-5 |
| 8 | 5-7 |
| >= 9 | 6-8 |

See the `content` skill for how new `Book`/`WordClue` entries are written and checked.
