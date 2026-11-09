# Pets Club farm confirm: child playtest, round 4 (narrow retest)

This is emulator roleplay by an independent agent. It is not a study with a real child and not a test on the physical device.

## Build and setup

- Revision `f06ede9`, clean tree. `make -C apps/porthole snap` reported it was already up to date.
- `build/host/snap` sha256 `35a9a3ff2a45f91a2bf0974aeba32fd8a0f4173ed1070d24ea114054be08ad0b`. The hash was the same after `make playtest`.
- Fixture: `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav`. Before every run it was copied over `apps/porthole/build/host/`. The hashes match round 3:
  - crago-s0 `a048df7b...`
  - porthole-m and porthole-mb `4bf5122f...`
  - porthole-p0 `845f86e0...`
  - sys-ran `ef260e9a...`
- The profile is "Mia" (Stats says "Mia, age 8"). The dog Biscuit shows "Grown dog - day 31". Roleplay treats the profile as Sam's own.
- Harness: `snap --serve` without `--fresh`, run from `apps/porthole/` at the default clock.
  - A small replay driver (`shelter-playtest-4/traces/serve-driver.txt`) copies the fixture, then sends ordinary `down`/`move`/`up`/`tick` lines in logical px. Each serve frame is 33 ms.
  - The driver saves native 480x480 frames. `press X Y MS` means down, then 33 ms ticks, then up.
  - `restart` closes stdin and starts snap again. `kill` sends SIGKILL and starts snap again. Both keep the saves.
  - The sim is deterministic, so each probe replays the shared opening (`traces/base.txt`) from a fresh fixture and then adds its own inputs.
- No `dbg`, `app`, `skip`, `ui` or debug/state queries were used.
- Evidence: `apps/porthole/docs/pets-club/shelter-playtest-4/`. Screenshots are numbered PNGs. Each probe's full input trace is in `traces/*.txt`.

### Disclosure

- I did not read game, os or shell source, `tests/playtests`, `docs/superpowers/` or `.superpowers/`, except the fixture directory.
- To learn the serve wire protocol I read the serve loop and the header comment in `host/sim.cpp`, plus the script-mode helpers next to them.
- **Leak before play:** reading that file auto-loaded `apps/porthole/CLAUDE.md`. It says that farm Yes uses "a longer hold", that `FreshGate` is 450 ms, that a long press is 600 ms, and that Pets Club checkpoints the farm move within a second. It gives no hold length.
  - Every timing below was measured from frames.
  - The 600 ms figure did shape one probe: I tried long presses on No and Go home.
- I read `shelter-playtest-3.md` only after play.

## Persona and brief (verbatim)

> You are an independent child-perspective playtester doing a narrow retest of one changed screen in Pets Club [...]
>
> **Persona:** Sam, 6 years old, can't read yet except his own name and "Biscuit". He presses hard and long when something doesn't seem to work, mashes when bored, and has a 10-year-old sister who sometimes tells him what to do. Simulated perspective only.
>
> **Setup (disclosed fixture):** `make -C apps/porthole snap`; record HEAD and the snap sha256. Before each fresh start, copy `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav` into `apps/porthole/build/host/` (overwrite). The profile is "Mia"; treat it as Sam's own. Biscuit is 30 days old. Run `snap --serve` without `--fresh`, from `apps/porthole/`, at the default clock. Ordinary touches only. No dbg, app shortcuts, clock skips or hidden-state queries.
>
> **Scope:** the question you reach from Biscuit's Stats page about sending Biscuit to the farm.
>
> - Accidents: presses of about 0.3, 0.7, 1.0, 1.5 and 1.9 s on Yes; firm presses repeated after nothing seems to happen; mashing around the screen; sliding off; starting on No. Does Biscuit ever leave without Sam meaning it? What does Sam see while pressing? Is it obvious that something is about to happen, early enough to let go?
> - Intent: the sister says "hold Yes until it's full". Can Sam do it? Does the picture make clear Biscuit is leaving?
> - Right after Biscuit leaves, quit the game from the launcher within a second or two and reopen: is the result kept?
> - Briefly: the farm after the goodbye (Go home), so the flow ends normally.
>
> **Output:** this report with screenshots and the input trace, revision and hash, persona, brief, fixture, ranked findings with repro, qualitative assessment and gaps. No code fixes, no commit.

There were no brief revisions.

## First play (Sam's own path)

1. **Picker:** "Who's playing?" with one row, Mia, showing a dog face. Sam taps the row (80,67) and gets the launcher.
   - He picks the big orange cartoon dog tile (37,72), sees the PETS CLUB splash, and then the room (`01_room.png`).
2. **Stats:** Sam pokes the little bars at the top (43,23) and gets Stats (`02_stats.png`).
   - He reads "Biscuit" and nothing else.
   - The big light-blue "To the farm" button is the most button-like thing there, so he taps it.
3. **The question** (`03_question.png`): dark blue, two lines of white and yellow text, a green **No!**, a brown **Yes**, yellow "Hold Yes" text and a sitting Biscuit.
   - For Sam, "Biscuit" is the only readable word. He cannot tell which button does what.
   - The green button looks like the "go" button.
4. **A quick tap on Yes:** the button sinks, a thin plum sliver appears at its left edge, and Biscuit stands up.
   - After release, everything snaps back and "Hold Yes" flashes white for about half a second (`04_short_press_release_hint_white.png`).
   - Sam: "It moved but didn't do anything." Following his habit, he presses harder and longer.
5. **Longer presses:** while the finger stays down, a plum fill sweeps steadily left to right over Yes, and Biscuit stands and trots in place (`05_hold_*.png`, `sheet_dog_during_hold.png`).
   - Letting go at any point before the fill is full empties it at once, and Biscuit sits back down.
6. **The sister's instruction, "hold Yes until it's full":** Sam holds.
   - The fill reaches the right edge at about 2.0 s (`07_full_hold_2013ms_question.png`). On the next frame, about 2.05 s with the finger still down, the farm appears (`08_full_hold_2079ms_farm.png`).
   - The "full" in the instruction matches what he sees, so he can do it.
7. **Farm:** blue sky, a fence, a "Biscuit" sign, Biscuit standing on the grass and a big green **Go home**.
   - A tap on Biscuit makes him jump with his tongue out and drop a heart (`19_farm_tap_dog.png`). Sam likes that.
   - Holding a finger on him, or rubbing, does nothing (`32_*`, `33_*`).
8. **Go home** (a short tap): an empty room with a framed little Biscuit on the wall and a big purple "Shelter" sign (`20_go_home_room.png`).
   - Sam: "Where's Biscuit?" The picture on the wall is the only Biscuit.
   - He would tap the big purple sign next, which is out of scope here.
   - The door still leads to Paw Street and back (`21_door_paw_street.png`), so this is not a dead end.

**First voluntary stopping point:** after patting Biscuit two or three times on the farm. Nothing else there reacts, and Go home leads to an empty room. Sam would stop there or go to the Shelter sign. Everything after this was coverage-driven, not play Sam would choose.

## Accident probes on Yes (from the freshly opened question)

| Probe | Trace | Result | Evidence |
| --- | --- | --- | --- |
| 0.3 s press | `p300` | Stays. The fill reaches about 12%, then empties. "Hold Yes" flashes white. | `04_*`, `06_*` |
| 0.7 s press | `p700` | Stays (fill about 30%). | `traces/p700.txt` |
| 1.0 s press | `p1000` | Stays (fill about 45%). | |
| 1.5 s press | `p1500` | Stays (fill about 70%). | |
| 1.9 s press | `p1900` | Stays (fill 135/146 px, about 92%). Release empties it at once. | `05_hold_1914ms.png`, `06_release_after_1914ms_stays.png` |
| Full hold (3 s) | `full` | **Leaves at about 2.05 s with the finger still down.** Releasing on the farm does nothing. | `07_*`, `08_*`, `sheet_full_hold_every_132ms.png` |
| Five firm presses of 1.5, 1.5, 1.5, 1.5 and 1.8 s, 300 ms apart | `rep` | Stays. Every press restarts from zero; nothing builds up. | `09_*`, `10_*` |
| Press on Yes for 1.4 s, slide down off it and hold 2 s, slide back and hold 1 s, release | `slide` | Stays. Sliding off empties the fill; sliding back on starts it again from zero. | `11_*`, `12_*`, `13_*` |
| Start on No for 0.5 s, slide onto Yes and hold 3 s | `startno` | Stays. Yes never starts filling, and No does not fire. | `14_*` |
| Wobbly hold, ±4 px jitter for 2.6 s | `jitter` | Leaves at about 2 s. A shaky finger still counts as a hold. | `15_*` |
| 1 s holds at Yes's edges: (92,102), (136,102), (115,91), (115,113) | `edge_*` | Fills (the edges count). | |
| 1 s holds just outside: gap (80,102), (141,102), (150,102), "Hold Yes" text (80,122) | `edge_*` | No fill. | `16_*` |
| Fast double/triple taps and mixed short presses on Yes (10 presses) | `mashyes` | Stays. | `17_mash_yes_only.png` |
| Mash all over, 5 seeds × 12 s, 30% biased toward Yes, presses 66-500 ms | `mash1-5` | Never left. The mash wandered into No → Stats → Stickers/Hats and back. | `17_mash_seed*.png` |
| Mash, 20 more seeds × 15 s, 40% toward Yes, presses up to 1.2 s | `mashb6-25` | **No farm frame in any run** (checked by pixel test). | `traces/mashb*.txt` |

**What Sam sees while pressing Yes:**

- On the first frame, the button sinks and Biscuit stands up.
- By about 0.13 s, a plum fill has started at the left edge. It grows steadily and fills the button in about 2 s.
- Biscuit trots in place the whole time; he does not move across the screen.
- So something visibly starts within a tenth of a second, which leaves almost 2 s to let go. At 1.9 s, though, the margin is only about 0.15 s.
- The fill colour (`#7E2553`) against the brown button (`#AB5236`) is about 1.75:1 by luminance. The hue change is easy to see; the brightness change is weak.

## Probes on No and Go home

| Probe | Trace | Result |
| --- | --- | --- |
| No: tap, 0.3, 0.5 and 0.6 s presses | `no`, `nolong300/500/600` | Goes back to Stats (`18_*`, `29_*`). |
| **No: 0.7 s and 1.5 s presses** | `nolong700`, `nolong1500` | **Nothing happens.** No sinks while held (`26_*`, `27_*`); on release the question is still there (`28_*`). |
| Go home: 0.3 and 0.6 s presses | `golong300/600` | Goes to the room (`31_*`). |
| **Go home: 1.0 and 1.5 s presses** | `golong1000`, `golong` | **Nothing happens.** Sam is still on the farm (`30_*`). A quick tap afterwards works. |
| Go home tapped 66 ms after the farm appears | `quitA` (first version) | Ignored (the screen-change guard). A second tap after 0.5 s works. |

## Keeping the result (quit and reopen)

| Probe | Trace | Result |
| --- | --- | --- |
| Farm, Go home at 0.6 s, home button at 1.1 s → launcher at about 1.4 s, reopen Pets Club in the same process | `quitA` | Empty room with the wall picture: kept (`25_*`) |
| The same, then SIGKILL, start again, Mia → Pets Club | `quitA` | Kept |
| Launcher 0.8 s after the farm appears, then SIGKILL and start again | `quitE` | Kept (`22_*`, `23_*`) |
| SIGKILL 0 ms, 0.3 s and 1.0 s after the farm appears (no other input) | `sigkill0/300/1000` | Kept every time (`24_*`) |
| Clean exit (stdin closed) 0, 0.3 and 1.0 s after | `killB/C/D` | Kept |

Round 3's m1 (the move was lost if the process restarted within about 3 s) is **fixed** on this build.

## Gesture and interruption matrix

| Control | Offset | Quick / double tap | Mash | Long press | Slide off / onto | Carried touch | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Confirm: Yes (hold) | The hold area matches the visible button | Safe | Safe (25 seeds) | Commits at about 2.05 s; 1.9 s is safe | Off cancels; back on restarts from 0 | Starting on No and sliding on never fills | Fixes round 3's M1 |
| Confirm: No | Not offset-tested | Exits to Stats | Exits often during mash | **≥0.7 s does nothing** | Off: no fire | Gap | **M1** |
| Farm: Biscuit | Body | Each tap pats | n/a | No reaction | Rub: no reaction | Finger still down from the commit: nothing | m3 |
| Farm: Go home | Centre | Tap at 66 ms ignored; at 0.5 s works | n/a | **≥1 s does nothing** | Gap | Not under the Yes finger | M1 |
| Process kill or restart 0-1 s after the commit | n/a | n/a | n/a | n/a | n/a | n/a | Kept |
| Ordinary leave → launcher → reopen within about 1.5 s | n/a | n/a | n/a | n/a | n/a | n/a | Kept |

Gaps:

- Night tint on the confirm and the farm.
- The shelter adoption that follows. It is out of scope; I only saw the sign.
- Farm friends and visits. I did not open them this round.
- No offset taps on No.
- Physical-device touch timing and pressure. "Presses hard" can only be modelled as long.

## Findings (prioritized)

### Blocker

None.

### Major

**M1. On the farm question, a long press on No does nothing, while a long press is exactly what Yes rewards**

- **Screen:** farm confirm (and the farm's Go home)
- **Issue:** A press of 0.7 s or more on **No!** sinks the button and then does nothing on release. Go home behaves the same at 1 s or more.
  - Sam's habit is to "press hard and long when something doesn't seem to work". On this screen, his safe answer fails silently with exactly that gesture.
  - The only control that responds to a long press is Yes, the one that sends Biscuit away. Both buttons look alike, and Sam cannot read either.
  - After a long press on No fails, the natural next attempt is the same long press on the other button. Two seconds later Biscuit is gone.
  - The screen also teaches holding ("Hold Yes", the fill), so pressing long feels like the right way to use these buttons.
  - On the farm, a long press on Go home leaves Sam stuck until he happens to tap quickly. This is mild on its own.
- **Concern:** recovery / consistency / feedback
- **Repro:**
  1. Fresh fixture, `traces/base.txt`: tap (80,67), tap (37,72), wait 3.5 s, tap the bars (43,23), tap "To the farm" (80,77), wait 1 s.
  2. `down 45 102`, ticks for 0.7-1.5 s, `up`, wait 0.6 s. The question is still there (`traces/nolong700.txt`, `nolong1500.txt`).
  3. Compare a 0.6 s press, which goes back to Stats (`nolong600.txt`).
  4. Go home: after a 2.2 s Yes hold and 0.8 s of waiting, press (80,136) for 1.0 s. Still on the farm (`golong1000.txt`).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-4/26_no_held_0528ms_pressed.png`, `27_no_held_1023ms.png`, `28_no_after_1500ms_press_still_question.png`, `29_no_after_600ms_press_stats.png`, `30_gohome_after_1500ms_press_still_farm.png`
- **Suggestion for the owner:** let **No** (and Go home) act on release inside the button however long it was held, like the code-pad keys are described as doing. Then the safe answer always works with Sam's gesture.
  - This is the same game-wide long-press behaviour as round 3's m2. On this screen it stops being cosmetic, because it steers a long-presser toward the irreversible button.

### Minor

**m1. Nothing on the question or the hold shows a non-reader that Biscuit will leave**

- **Screen:** farm confirm; Stats "To the farm"
- **Issue:** Sam reads only "Biscuit", and both steps to the farm are text-only.
  - During the hold, Biscuit stands and trots in place and a bar fills. To a 6-year-old that reads as "something good is loading" or "Biscuit wants a walk", not "Biscuit is going away to live somewhere else".
  - There is no farm, barn, gate or direction of travel until it has already happened (`sheet_dog_during_hold.png`).
  - The "Hold Yes" white flash after a short press (`04_*`) nudges him to press longer without saying what that will do.
  - Mechanically, accidents are now well guarded: every probe under 2 s was safe, and 0 of 25 mash runs sent him. A lone Sam can still finish the 2 s hold out of curiosity ("see what the bar does") without understanding the consequence.
  - The outcome is gentle: the dog lives on the farm and stays on the wall. That is why this is Minor, not Major.
- **Concern:** comprehension
- **Repro:** fresh fixture → Stats → "To the farm" → `press 115 102 3000` (`traces/full.txt`).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-4/03_question.png`, `05_hold_0924ms.png`, `sheet_dog_during_hold.png`
- **Suggestion:** add a picture cue. For example, a small farm or fence picture on the question and on the "To the farm" button, and Biscuit walking toward it as the fill grows. Then the hold animation itself says "leaving" and gives a reason to let go.

**m2. A 1.9 s press is only about 0.15 s from committing**

- **Screen:** farm confirm
- **Issue:** The commit comes at about 2.05 s, so a "hard and long" press of about 2 s by a frustrated kid lands right at the line.
  - The fill gives about 1.9 s of warning, which a watching child can act on. It is listed so the coordinator chooses the margin deliberately.
  - If M1 is fixed, a long press on No becomes the safe outlet, and this matters less.
- **Concern:** recovery
- **Repro:** `traces/p1900.txt` (stays) versus `traces/full.txt` (leaves at about 2.05 s).
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-4/05_hold_1914ms.png`, `08_full_hold_2079ms_farm.png`

**m3. On the farm, holding or rubbing Biscuit gets no reaction (carried over from round 3's m3)**

- **Screen:** farm
- **Issue:** Only a quick tap pats him. Sam presses long, so his first pat is often a hold, and Biscuit just stands there.
- **Concern:** feedback
- **Repro:** `traces/farmpet.txt`
- **Screenshot:** `apps/porthole/docs/pets-club/shelter-playtest-4/32_farm_hold_dog_no_reaction.png`, `33_farm_rub_no_reaction.png`

### Nits

- **n1.** The plum fill on the brown button is about 1.75:1 in luminance. The hue change carries it; a lighter or brighter fill would read better at the edge of a 6-year-old's attention (`05_hold_0330ms.png`).
- **n2.** For a non-reader, the green "No!" looks like the "go" button. That is the safe side, so it is not a problem, but it adds to m1.

## Qualitative assessment

**Can Sam recognize the goal and actions, and tell what changed?**

- Stats and the question: no, apart from the name "Biscuit". He presses the biggest button out of curiosity.
- During the hold he can clearly see that something is happening: the button sinks, the bar fills and Biscuit stands up.
- After the commit, the farm picture clearly shows Biscuit outside behind a fence, with his name on a sign. The empty room with his framed picture says he has gone.
- What he cannot tell is that Yes means "leave for good" before it happens (m1).

**Decisions, feedback, pictures versus reading.**

- The question is reading-led. With his sister reading it and saying "hold until it's full", Sam can do it easily. The full bar is a clear and fair target, and even a wobbly finger completes it.
- Alone, the decision is not informed, but it is deliberate: 2 s of continuous pressing on one button, with a visible bar the whole time.
- The farm patting (tap) is a nice goodbye beat.

**Where it becomes confusing or frustrating, and where Sam stops.**

- Frustration point: long presses on No and Go home do nothing (M1). For Sam this feels like "it's broken" and leads to harder presses, which is the risky pattern on this screen.
- The farm is brief. After two or three pats there is nothing else to do; Go home shows the empty room, and Sam would stop or tap Shelter.
- Mashing never sent Biscuit, and Sam would get bored of mashing the question within seconds.

**Recovery from imprecise input, wrong guesses and interruption.**

- Very good for Yes: offset presses, jitter, slide-off, slide-on from No, repeated presses and mashing all behave sensibly.
- Weak for No and Go home under long presses (M1).
- Interruption is solid: the result is kept after quitting to the launcher within about 1.4 s, and even after a SIGKILL at 0 ms.

## Supplementary QA (after play, separate from the black-box pass)

- `make -C apps/porthole playtest` at `f06ede9`: **90 of 90 scenarios pass, 0 UI FAIL and 0 UI WARN** in total. `b0_pets_farm_shelter`: 130 checks.
- I read `build/playtest/report.md`: the scenario table, the empty UI findings table and the contact-sheet list.
- The audit cannot see press-duration behaviour, which is what M1 is about.

## Iteration history

- **Rounds 1-3:** see `shelter-playtest.md`, `shelter-playtest-2.md` and `shelter-playtest-3.md` (`2778979`).
  - Round 3's M1 was that Yes committed at about 0.65 s mid-press.
  - Round 3's m1 was that the move was lost on a restart within 3 s.
- **Round 4** (this report, `f06ede9`):
  - Round 3's M1 is **fixed**: the hold is about 2.05 s, with a visible fill and cancel on release or slide-off.
  - Round 3's m1 is **fixed**: the move is kept even after a SIGKILL at 0 ms.
  - Round 3's m3 (holding or rubbing gets no reaction) is **still present**.
  - Round 3's m2 (long presses ignored) is still present and **escalated to Major (M1) on the confirm**, because there it steers a long-presser toward Yes.
  - I changed no code and committed nothing.

## Verdict

**Needs fixes.** M1 (No and Go home ignore long presses) is the one Major for this persona. After it is fixed, replay the confirm and the farm exit with long presses on every button. m1 (no pictorial cue for leaving) should be fixed or explicitly deferred with the player-impact rationale above.
