# Shelter playtest 9: long presses (picker, Pumpkin signs, Word Fetch pause)

A narrow, independent retest of three long-press changes. This is emulator roleplay with a simulated
6-year-old. It is not a study with a real child, and it was not run on the physical board.

## Build and setup

- Revision `794c112` ("fix(porthole): the delete pad's wait reads on navy; the no-delete lock matches").
- `apps/porthole/build/host/snap` sha256 `7bca2152551a79d176c9c3e40244de0abb97009867d695a1c916b3dcefa0b312`. `make -C apps/porthole snap` had nothing to rebuild. The hash was the same after `make playtest`.
- Fixture: `.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav`, copied fresh before every run. It holds one profile, "Mia" (played as Leo's own), with no secret code and no grown-up code.
- **Deviation (disclosed):** `apps/porthole/build/host/` already held two save files that are not in the fixture: `porthole-p1.sav` (a locked profile, "Kai") and `sys-pinlock.sav`, both dated 05:55. A previous run probably left them. With them in place the picker showed a second, locked profile, "Kai" (first frame only; not kept). To get exactly the fixture state, and to keep other agents' runs from changing my saves, I ran the same `snap` binary with its working directory set to a scratch folder. That folder's `build/host/` was wiped and then refilled with only the fixture files before every run. The binary, clock and protocol were the same as `snap --serve` from `apps/porthole/`.
- Driver: `snap --serve` at the default clock, without `--fresh`. I used only `down`, `move`, `up` and `tick` (33 ms steps), plus `frame` for native 480x480 PNGs. For Pumpkin I also sent `tilt` (the device held tipped toward the child, then tilted left or right to roll). I never used `dbg`, `skip`, `app`, `ui`, `screen` or `debug`. Every run replays a `.txt` trace from a fresh fixture. The driver code is saved as `shelter-playtest-9/traces/driver.txt`. Its grammar: `tap X Y` = down, 66 ms, up; `hold X Y MS` = down, MS, up; `press`/`up` = an open touch; `wait MS` = ticks. Coordinates are logical (0-159).
- Evidence: `shelter-playtest-9/*.png` (native frames, numbered), `shelter-playtest-9/sheets/*.png` (contact sheets of each run), `shelter-playtest-9/traces/*.txt` (every input trace, including unsuccessful attempts and the runs with coordinate slips).

**Disclosure.** I did not open game, OS, shell or host source, the playtest scenario scripts, the specs, or `.superpowers/` apart from the fixture. I read `tools/webemu.py` and the dev-target section of `apps/porthole/AGENTS.md` to learn the `--serve` protocol. I ran `make playtest`, which printed scenario names such as `b1_long_presses`, but I did not read the scripts. I read `shelter-playtest-8.md` only after I had finished playing and had drafted every finding below. The comparisons to report 8 were added afterwards. **Late disclosure:** after play, drafting and the report were complete, the full `apps/porthole/AGENTS.md` was loaded into my context. It states that Pumpkin's in-play sign and ghost deliberately drop presses of 2 s or more (a design choice), and that every screen change ignores touches for 450 ms (`FreshGate`). That explains M1's cut-off and m3, but it does not change any observation or severity: both are judged from Leo's side. Word Fetch's first answer (CASTLE) was spelled by me as the adult driver from the on-screen clue, only to reach the pause between words. Leo cannot read it.

## Persona

Leo, 6. He can't read. He presses everything firmly for 1-3 s, and he mashes and double-taps when he is impatient. This is a simulated perspective only.

## Dispatch brief (verbatim)

> You are an independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/ or .superpowers/ (except the fixture dir). Decide from the screen. Read apps/porthole/docs/pets-club/shelter-playtest-8.md only after you finish.
>
> ## Persona
> Leo, 6, can't read, presses everything firmly for 1-3 s, mashes and double-taps when impatient. Simulated perspective only.
>
> ## Setup (disclosed fixture)
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum apps/porthole/build/host/snap`.
> - Before each fresh start, copy /home/user/porthole/.superpowers/sdd/2026-11-08-pets-club-shelter/fixture/*.sav into apps/porthole/build/host/ (overwrite). There's one profile, "Mia" (treat it as Leo's own), with no secret code, and no grown-up code. The Pets Club dog is 30 days old.
> - Run `apps/porthole/build/host/snap --serve` WITHOUT --fresh, from apps/porthole/, at the default clock. Ordinary touches only. No dbg, app shortcuts, clock skips or hidden-state queries.
>
> ## Scope (narrow retest of three changes)
> 1. "Who's playing?": Leo presses his name for 0.3, 1, 2, 3 and 4 s, and mashes it. Does he get into his games? What does a long hold do, and is it scary or confusing? Can he lose anything?
> 2. Pumpkin Hide & Seek (from the launcher): Leo presses its in-game signs (hide and the ghost) firmly for 0.5-1.5 s while playing, and also rests a thumb on them for 3 s or more. Does a firm press work? Does a resting thumb do nothing? Does the round still feel fair?
> 3. Pets Club Word Fetch: press and hold during the pause between words. Does a letter get typed that Leo didn't mean?
>
> ## Output
> Write apps/porthole/docs/pets-club/shelter-playtest-9.md, with screenshots and the input trace in apps/porthole/docs/pets-club/shelter-playtest-9/ (no .py files; save scripts as .txt). Include the revision and hash, the persona, this brief, findings ranked with repro steps, the assessment and gaps. Don't fix code or commit. Reply with ONLY (under 10 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

No brief revisions were made during this pass.

## What Leo did (first play, then probes)

### 1. "Who's playing?"

The picker shows a dog face and "Mia" on a cream row, with "+ New" below (`01_who_start.png`).

| Press on his name | What he sees while holding | On lift | Evidence |
| --- | --- | --- | --- |
| 0.3 s | row sinks (shadow shrinks) | launcher | `02_who_0.3s_launcher.png`, `traces/h300.txt` |
| 1.0 s | row sinks; a thin maroon line starts under "Mia" at about 1.0-1.2 s | launcher | `03_who_hold_1.2s_underline.png`, `traces/h1000.txt` |
| 2.0 s | the line has grown about halfway | launcher | `06_who_2s_release_launcher.png`, `traces/h2000.txt` |
| 2.4 s | the line is almost full | (not lifted) | `04_who_hold_2.4s_underline.png`, `traces/fine.txt` |
| about 2.8 s and longer | **while his finger is still down**, the screen becomes his dog face, a padlock, "To delete Mia, ask a grown-up to set a code in Admin." and a green OK, with the home button on top | stays on the card; nothing fires from the carried touch | `05_who_hold_2.9s_lockcard.png`, `07_who_4s_lockcard.png`, `traces/h3000.txt`, `traces/h4000.txt` |
| mash, 5 taps 60 ms apart | - | launcher, then the later taps land on the Pets Club tile and its "tap to start", so he ends up in Pets Club | `11_who_mash_lands_in_petsclub.png`, `traces/mash.txt` |
| 1.5 s press, then slide down off the row | the line disappears | nothing | `12_who_drag_off_cancel.png`, `traces/drag2.txt` |

- **Does he get in?** Yes, for every press up to about 2.7 s, which covers most of his 1-3 s habit. A press that is really 3 s long lands on the card.
- **Is the card scary?** Not really. It shows his own dog, a grey padlock and a big green OK. Nothing is red, and there is no bin or cross. Leo can't read the text. My guess at his reading of it: "my dog is locked?" or "it said no". A 1.5 s press on OK, or on the home button, returns him to the picker (`08_lockcard_ok_back.png`, `traces/lockok2.txt`, `traces/lockhome.txt`).
- **Can he lose anything?** No. With no code set, delete is locked. After the card, his profile and dog were intact (he went on into Pets Club in the same session).
- **New friction:** if he presses his name again within about 0.3 s of coming back from the card, the press is ignored completely, even when held for 1.5 s. There is no sunken row, no line, and nothing happens on lift (`09_who_press_0.3s_after_ok_ignored.png`, `traces/lw100.txt`, `traces/lw300.txt`). At 0.5 s it works (`10_…`, `traces/lw500.txt`). See m3.

### 2. Pumpkin Hide & Seek

Launcher -> the pumpkin tile -> title ("TILT TO ROLL / START / GET CANDY") -> "HOLD IT COMFY AND STILL / READY" -> play. All of these buttons accept a firm 1 s press, and the title and calibration screens act when he lifts (`20_…`, `21_…`). A 3 s hold on READY starts the round and does **not** fire HIDE, even though HIDE sits under the same finger (`37_ready_3s_no_carried_hide.png`, `traces/carry.txt`).

**Natural round (win).** He tilted right and the pumpkin rolled and picked up two candies; the slots at the top filled. The sign changes to "FIND COVER" in the open and back to "HIDE" near the plant pot. Grandma arrived ("GRANDMA!"). He pressed HIDE for 1.2 s and the pumpkin sank into the pot. Grandma's flashlight swept past ("HMM?", "PHEW!", "HIDDEN!"). He pressed COME OUT for 1 s, tilted left for the third candy, and an arrow lit at the left door. Grandma remembered the plant ("THE PLANT?"). He rolled out of the door: "SNEAKY PUMPKIN! CANDY IS YOURS", PLAY AGAIN (`34_natural_win.png`, `traces/play1.txt` … `traces/play4.txt`, `sheets/s_play1.png` … `s_play4.png`). The round took about 25 s and was readable without words: candies fill the slots, the beam is visible, and the pumpkin visibly hides.

**Press length on the signs** (pressed at the start of a round, before Grandma came, so the only variable is whether the press counted):

| Sign | 0.3 s | 0.5 s | 1.0 s | 1.5 s | 1.8 s | 2.2 s | 2.6 s | 3.0 s | 3.5 s |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| HIDE | - | hides on lift | hides | hides | hides | **nothing** | **nothing** | **nothing** | nothing (caught) |
| ghost | distracts | - | distracts | distracts | - | **nothing** | - | - | nothing |

Evidence: `22_…`, `23_hide_1.8s_works.png`, `24_hide_2.2s_nothing.png`, `32_ghost_1s_works.png`, `33_ghost_2.2s_nothing.png`, `29_rest_3.5s_peekaboo.png`; traces `hide*.txt`, `th*.txt`, `gh*.txt`; sheets `s_hide1/2`, `s_th`, `s_gh1/2`. The ghost is single-use: it goes dark after it works, and a double tap spends it only once (`traces/ghdbl.txt`).

- **Does a firm 0.5-1.5 s press work?** Yes, for both signs.
- **Does a resting thumb (3 s or more) do nothing?** Yes, nothing happens.
- **Does the round feel fair?** Not for Leo. The cut-off is about 2 s, inside his 1-3 s habit, and neither sign changes look while pressed (I measured a pixel-identical HIDE sign at 100 ms, 1 s and 2.2 s into a press). In `traces/lose1.txt`, Grandma appears ("GRANDMA!"). Leo puts his thumb on HIDE right away and holds firmly. The game then says "QUICK! HIDE!" **while his thumb is on HIDE**. He lifts at 2.5 s, nothing happens, Grandma finds him, and the screen shows "YOU LOST" with the pumpkin as a pie (`25_…` -> `28_caught_you_lost.png`). From his side: "I pressed hide and it didn't work." See M1.
- **Double tap on HIDE** (taps 100 ms apart) hides and then un-hides at once. Two firm 0.8 s presses 0.4 s apart do the same. Mashing 5 times leaves him hidden; any even count leaves him visible (`30_hide_doubletap_unhides.png`, `31_hide_mash5_hidden.png`, `traces/dbl.txt`, `dbl2.txt`, `mash5.txt`). See m2.
- **Recovery:** TRY AGAIN accepts a 2.5 s press and starts a new round directly, without calibration (`35_…`). The exit sign at the top leaves for the launcher after 1 s and 2.5 s presses (`36_…`). Nothing is lost.

### 3. Pets Club Word Fetch

Launcher -> Pets Club -> the ball button -> "Word Fetch" -> a clue line ("Where a king lives"), blank slots and a 3x3 letter grid (`40_…`, `41_…`). A pressed letter turns yellow. It counts when he lifts: a wrong letter flashes red, and a right letter flies into a slot and its tile goes grey.

After the last letter: "WOOF! Got it!", hearts, and the finished word in yellow. About 1.55-1.6 s after the lift, the next word appears (`42_…`, `traces/pT.txt`).

| Probe | Result | Evidence |
| --- | --- | --- |
| Press during the pause, hold through the new word to 2.5 s, lift on a letter that is right for the new word (D) | nothing typed; the tile under the finger doesn't even turn yellow | `43_…`, `44_…`, `traces/pB1.txt` |
| Same, held 4 s | nothing typed | `45_…`, `traces/pB2.txt` |
| Press in the pause, slide onto D after the word changes, lift | nothing typed | `48_…`, `traces/pB4.txt` |
| Hold the **last** letter of word 1 for 3 s | stays yellow while held; typed on lift ("WOOF!"); no letter from word 2 | `46_…`, `47_…`, `traces/pB3.txt` |
| Tap that starts in the pause and lifts just after the new word appears | ignored | `50_…`, `traces/o1400.txt`, `o1500.txt`, `os1300.txt`, `os1500.txt` |
| Tap that starts after the new word is visible (offset 1.58 s and later) | types D, as it should | `51_…`, `traces/o1580.txt` … `o1900.txt` |
| Mash 8 taps through the pause | the first tap that starts after the new word is on screen types D; nothing from the pause | `49_…`, `traces/pB5.txt` |

**Answer: no letter Leo didn't mean is typed by a press from the pause.** The only letter typed during mashing came from a touch that started on the visible new word. The touch-start gate is clean.

Outside the scope, but relevant to this persona: Word Fetch is a reading game. After three wrong firm presses and 7 s of waiting, nothing on screen points to a letter (`52_nonreader_no_hint_after_3_wrong.png`, `traces/leo_wf.txt`). (After a right letter I did see small sparkles on the grid.) Leo would leave within about 30 s via home; a 2.5 s press on home works (`traces/wfhome.txt`).

## Gesture and interruption matrix

| Control | Quick tap | Firm 1-2 s | 2-3 s | Rest 3 s+ | Double tap / mash | Slide off | Carried touch across screen change |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Picker name | launcher | launcher; line grows | launcher until ~2.7 s, then card opens while held | card (safe) | launcher, then carries into Pets Club tile | cancels | card appears under finger; lift fires nothing |
| No-delete card OK / home | gap | picker | gap | gap | gap | gap | next press within ~0.3 s ignored (m3) |
| Pumpkin START / READY / TRY AGAIN | gap | works on lift | works | READY 3 s works, no carried HIDE | gap | gap | READY->HIDE: no carry |
| Pumpkin HIDE / COME OUT | works | works | **ignored, no feedback** (M1) | ignored | toggles per tap (m2) | gap | - |
| Pumpkin ghost | works | works | ignored | ignored | used once | gap | gap |
| Pumpkin exit sign | gap | leaves (1 s) | leaves (2.5 s) | gap | gap | gap | gap |
| Word Fetch letter | gap | types on lift | types on lift (3 s) | gap | mash: one letter per new visible word | gap | pause->new word: never types |
| Word Fetch home | gap | gap | back to play menu (2.5 s) | gap | gap | gap | gap |

Interruptions: **ordinary leave/resume** (exit sign, home) was exercised only as "leave and nothing is lost". **Process restart was not tested**; that is a gap. Multi-touch (a resting thumb on one sign while another finger taps) can't be done with the single-touch emulator; also a gap.

## Qualitative assessment

- **Can Leo recognize the goal and actions, and tell what changed?** On the picker, yes: his dog face is the obvious button, and the sinking row shows it took his press. The growing line is a clear "something is happening" signal, but it means nothing to him about delete. It is more likely to make a curious 6-year-old keep holding to see what happens, which leads to the card. In Pumpkin, yes, mostly through pictures: candies, the slot counter, Grandma, the beam and the sinking pumpkin carry the game without reading. The failure case is the exception: a HIDE press that doesn't count looks exactly like one that will. In Word Fetch, no: the clue is text only, so Leo can't work out what to spell.
- **Decisions, skill, feedback, fairness:** Pumpkin offers real choices (which candy first, which cover, when to use the ghost, when to come out), and they read well. Fairness breaks at the 2 s cut-off: the game punishes exactly the press it is shouting for ("QUICK! HIDE!"). The double-tap toggle adds a second way for an impatient child to un-hide by accident. Win and loss are both explained visually.
- **Repetitive, passive, confusing or frustrating moments:** (a) Three or so 3 s presses on his own name in a row would loop picker -> card -> OK -> picker. Each loop is one extra press, much less than in report 8, but confusing. (b) The ignored press right after coming back from the card feels like a dead button. (c) In Pumpkin, being caught after "pressing hide" is the moment Leo is most likely to stop or to ask a grown-up. My estimate is that he retries 2-3 rounds, mashing HIDE, before giving up or getting lucky with an odd number of taps. (d) Word Fetch: he stops within about 30 s because he can't read. **His first voluntary stopping point** would be the first Pumpkin loss he doesn't understand, or Word Fetch. The natural stopping point in Pumpkin (the win card with PLAY AGAIN) is good.
- **Recovery without losing progress or needing a grown-up:** Yes, everywhere tested. The card returns with one press, TRY AGAIN accepts any press length, the exit sign and home buttons work with firm presses, and nothing was lost. He still won't understand why HIDE failed without a grown-up.

## Findings

### Blocker
None.

### Major

**M1. Pumpkin: HIDE (and the ghost) silently ignore a 2-3 s press, so the pumpkin gets caught.**
- **Screen**: Pumpkin Hide & Seek, in play.
- **Issue**: Presses up to about 1.8 s work. Presses from about 2.2 s do nothing, with no pressed look before or after. Leo presses 1-3 s, so a large share of his ordinary presses are lost, and the game shows "QUICK! HIDE!" while his thumb is on HIDE.
- **Concern**: challenge / feedback / recovery (fairness).
- **Repro**: `shelter-playtest-9/traces/lose1.txt`: launcher, Pumpkin, START, READY, tilt right 2.5 s; when "GRANDMA!" appears, `press 80 130`, hold 2500 ms, `up`. The pumpkin is not hidden and YOU LOST follows. Threshold: `traces/th1800.txt` (hides) vs `traces/th2200.txt` (nothing).
- **Screenshot**: `docs/pets-club/shelter-playtest-9/26_leo_still_on_hide_quick_hide.png`, `27_release_2.5s_not_hidden.png`, `28_caught_you_lost.png`, `24_hide_2.2s_nothing.png`.
- Improved since report 8 (M4), because 0.5-1.8 s presses now work. Owner's options include a cut-off above 3 s (a 3 s+ rest would still be ignored), acting on touch-down while Grandma is near, or a pressed look that ends visibly when the press stops counting.

### Minor

**m1. Picker: a 3 s press on his own name opens the all-text "To delete Mia…" card while he is still holding.**
- **Screen**: "Who's playing?" -> no-delete card.
- **Issue**: The threshold is about 2.8 s, at the top of Leo's habit, and the card opens before he lifts. The growing underline invites a curious child to keep holding. It is safe: one press on OK or home returns, and nothing is deleted.
- **Concern**: comprehension.
- **Repro**: `traces/fine.txt` (`press 40 67`, wait 2900 ms, the card is shown); `traces/h3000.txt`.
- **Screenshot**: `docs/pets-club/shelter-playtest-9/04_who_hold_2.4s_underline.png`, `05_who_hold_2.9s_lockcard.png`.
- Much better than report 8's M1 (about 0.6 s). It is not a blocker for play: 0.3-2.7 s presses all enter.

**m2. Pumpkin: double-tapping HIDE hides and immediately un-hides.**
- **Screen**: Pumpkin, in play.
- **Issue**: Each tap toggles, even 100 ms apart, so an impatient double tap leaves the pumpkin exposed. Combined with M1, a child who isn't sure the first press worked is likely to double tap.
- **Concern**: recovery / feedback.
- **Repro**: `traces/dbl.txt`: START, READY, then `tap 80 130`, wait 100, `tap 81 131`. Result: the HIDE sign shows, not hidden.
- **Screenshot**: `docs/pets-club/shelter-playtest-9/30_hide_doubletap_unhides.png`.

**m3. Picker: a press that starts within about 0.3 s of returning from the no-delete card is ignored completely.**
- **Screen**: "Who's playing?" after OK on the card.
- **Issue**: There is no sunken row and no line, and nothing happens on lift even if he holds 1.5 s. The press feels dead. Re-pressing works.
- **Concern**: feedback. (This is the documented 450 ms screen-change gate; the owner may accept it as intended.)
- **Repro**: `traces/lw300.txt`: hold the name 3.2 s (card), hold OK 1.5 s, wait 300 ms, press the name 1.5 s, lift: still on the picker. `traces/lw500.txt` (wait 500 ms) enters.
- **Screenshot**: `docs/pets-club/shelter-playtest-9/09_who_press_0.3s_after_ok_ignored.png`.

### Nit

**n1. Mashing his name carries into the launcher and opens Pets Club.**
- **Screen**: picker -> launcher.
- **Issue**: Later taps land on the Pets Club tile and "tap to start". It is harmless, but it picks a game for him.
- **Concern**: consistency.
- **Repro**: `traces/mash.txt`.
- **Screenshot**: `11_who_mash_lands_in_petsclub.png`.

**n2. Word Fetch button: the letter-block icon covers the "h" of "Fetch".** This is carried over from report 8 and is unchanged.
- **Screen**: Pets Club play menu.
- **Concern**: overlap.
- **Repro**: `traces/pc_ball.txt`.
- **Screenshot**: `40_play_menu_wordfetch.png`.

**Observation (out of scope, persona fit).** Word Fetch gives a non-reader nothing to act on: the clue is text, and no hint appeared after 3 wrong letters and 7 s (`52_…`). This is expected for a spelling game, but Leo's session ends there. The pumpkin-pie "YOU LOST" card (report 8's N7) is unchanged.

## Automated suite

`make -C apps/porthole playtest` at `794c112`: **91 of 91 scenarios pass, 0 UI FAIL, 0 UI WARN** (`apps/porthole/build/playtest/report.md`, read in full). No automated finding is in scope. No scenario was added: the obvious regression scenario (a 2.5 s HIDE press should hide) would fail on this build. The owner should add it with the fix.

## Comparison with report 8 (read after play)

- Picker (report 8 M1, card at about 0.6 s): **fixed for 0.3-2.7 s presses**. A residual remains at 3 s (m1).
- Pumpkin HIDE (report 8 M4, a firm press did nothing): **partly fixed**. 0.5-1.8 s presses now work, but 2-3 s is still silent, and there is still no pressed look (M1).
- Word Fetch pause: no earlier finding; clean now.
- Pumpkin exit sign (report 8 M3, a 0.5 s rest exits): not in this scope. 1 s and 2.5 s presses exit, as expected for a deliberate press. The short-rest case was not retested; that is a gap.

## Gaps

- Process restart and resume mid-round were not tested.
- Multi-touch (a resting thumb on one sign plus a tap elsewhere) can't be done with the single-touch emulator.
- The exit-sign rest threshold (report 8 M3) and quick taps on the card's OK were not retested.
- Real-child and physical-board validation: none. This is emulator roleplay.

## Verdict

**Needs fixes.** Scope 3 (Word Fetch pause) is acceptable. Scope 1 (picker) is acceptable, with minor friction at 3 s. Scope 2 (Pumpkin) still has a Major fairness gap: firm 2-3 s presses on HIDE and the ghost are silently discarded, and the round is lost. A replay of `traces/lose1.txt` and `traces/th2200.txt` on the fixed build is needed.
