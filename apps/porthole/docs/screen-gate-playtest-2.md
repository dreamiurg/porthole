# Screen-change tap gate, round 2: black-box kid playtest

Emulator roleplay by an independent playtester agent (`playtester`, opus). Simulated perspectives only: no real
child played and no physical board was used.

## Build

- Revision: `553ced4` (`553ced49ced6b611f8cb84222d04d5b7ad36de59`), clean tree before this report.
- Executable: `make -C apps/porthole snap` (up to date), copied to a private scratch folder; sha256
  `0fd7cad736434ca5e4fd5e26adaa5974b5a52b04e1ccf0b1c050b99ab7bd9c05` (identical to `apps/porthole/build/host/snap`).
- Mode: `snap --serve --fresh`, default clock (Tuesday 16:00). Profiles made through the screens: Zoe (age 8, fox, no
  code) and Leo (name mashed to "Bbbb", age 6, frog, meant to have no code; see M3). No `dbg`, `app`, `profile`, clock
  skips or state queries during play. Tilt through the serve protocol's `tilt X Y Z` (milli-g).

## Dispatch brief (verbatim)

> You are a fresh, independent child-perspective playtester for Porthole, a round 480x480 touchscreen with no buttons, emulated on the host. Follow apps/porthole/docs/playtesting.md and the `playtest` skill. Play source-free: don't read source (apps/porthole/games/, os/, shell/, host/), tests/playtests scripts, docs/superpowers/ or earlier playtest reports until you finish. Disclose anything that loaded automatically. Decide from the screen. Run the sim from your own copy of the snap binary in a private scratch subfolder.
>
> ## Personas (two passes)
> A. Leo, 6, can't read, taps firmly (200-400 ms presses), mashes 3-6 times on one spot, taps "again!" about once a second, and rests a thumb on the screen while tilting.
> B. Zoe, 8, a confident reader who taps quickly and deliberately from button to button and gets impatient with slow buttons.
> Simulated perspectives only.
>
> ## Setup
> - `make -C apps/porthole snap`. Record `git rev-parse --short HEAD` and `sha256sum` of the snap you run.
> - `snap --serve --fresh`, default clock. Create the profile (age 6 or 8, no code) through the screens; ordinary touches only; no dbg, shortcuts or clock skips. Tilt inputs through the serve protocol are allowed for tilt games.
>
> ## Scope
> This build changes how screens treat taps right after they appear.
> - Zoe: navigate quickly through the picker, Home, All games, star picking, Pets Club (feeding, the shop/room, a minigame) and back, Toy Town (any +/- or picking), and Pumpkin. Does any button ignore her or feel slow?
> - Leo: mash on one spot through the same screens. Does a tap ever open or change something on the NEXT screen he didn't aim at? Do his "again!" taps get a response within a few seconds?
> - Pumpkin: two rounds. Double-tap READY/TRY AGAIN, press HIDE short and long, land a thumb on HIDE right as the round starts and keep it there, and use the ghost. Is hiding fair and immediate, and does anything happen that Leo didn't mean?
>
> ## Output
> Write apps/porthole/docs/screen-gate-playtest-2.md, with screenshots and the input trace in apps/porthole/docs/screen-gate-playtest-2/ (no .py files; save scripts as .txt). Include the revision and hash, the personas, this brief, findings ranked with repro steps, the assessment and gaps. Afterwards you may read apps/porthole/docs/screen-gate-playtest.md and say which of its findings recurred. Don't fix code or commit. Reply with ONLY (under 8 lines): a verdict (acceptable / needs fixes), the top findings, and the report path.

Brief revisions: none.

## Disclosure

- Loaded automatically: the root `CLAUDE.md` and `apps/porthole/CLAUDE.md`. The latter states the gate's rules
  (`FreshGate`: 16 px spot, 800 ms, re-armed by ignored presses, 2.5 s cap; 450 ms light gate for in-place changes;
  a keypad's `shownAt` ignores only a press near the opening tap for 800 ms, never re-armed; Pumpkin's sign and ghost
  act as a press lands). I knew these numbers while playing and used them to aim edge probes. No solutions.
- Read on purpose: `apps/porthole/docs/playtesting.md` (its "Timing guards" section repeats the same numbers), the
  `playtest` and `tour` skills, and `apps/porthole/tools/webemu.py` for the serve protocol.
- Not read: source under `games/`, `os/`, `shell/`, `host/`; `tests/playtests/*.txt`; `docs/superpowers/`; the first
  screen-gate report (read only after this report's findings were written, for the recurrence section).
- Setup slip, disclosed: my first Zoe replays ran in a folder with no `build/host/`, so the sim silently dropped every
  save and Zoe's stars "vanished" after a picker round trip. With the folder present, they persist. Not a game finding;
  every screenshot here comes from a final replay with the folder present.

## Method and input evidence

`screen-gate-playtest-2/drive.py.txt` (the private driver, saved as text) replays a trace into a fresh
`snap --serve --fresh` and writes the frame at each `shot`. Trace lines are serve commands plus macros: `T X Y MS`
(down, hold MS, up), `D`/`M`/`U` (down/move/up), `W MS` (tick), `shot NAME`. Coordinates are logical px. Each step was
chosen from the latest frames, appended, and the whole trace replayed from a fresh device; both full traces were
replayed once more in fresh processes to make the sheets.

| File | Contents |
| --- | --- |
| `zoe/trace.txt` | Zoe: name, face, age, skip; All games, page 2, star mode, Home full, swap a star, check; Pets Club (package, adopt, name, house, feed, sticker, tricks, a lesson, Fetch, book shelf, a story, back out); Toy Town (doll, dress-up, Town, Park, bag, Save & leave); Pumpkin win, play again, hold-exit; picker and back |
| `leo/trace.txt` | Leo: mashed name, OK, face, age (code pad carry), All games (Radar), star mode, check, house; Pets Club (package, puppy, ? name, house, food, sticker, paw, Sit lesson, Fetch, exit); Toy Town (doll, dress-up, next, Done, back, leave); Pumpkin 2 rounds (double-tap READY, thumb resting while tilting, loss, double-tap TRY AGAIN, ghost, long HIDE, HIDE mash, loss); picker and his code pad |
| `probes/*.txt` | Self-contained branch traces (each begins with the persona trace up to a point): `pk-a..e` READY variants, `zp-pa` PLAY AGAIN double tap, `pk-m`/`pk-pn` panic mash, `pk-g150`/`pk-g450`/`pk-v*` mash gaps, `age-a3`/`age-a4` age mash, `pick-p` picker row mash, `package-taps` Pets Club package |
| `sheets/*.png` | Labeled contact sheets (frame names match `shot` lines) |
| `zoe/`, `leo/`, `probes/*.png` | Native 480x480 frames for each finding |

## What happened

### Zoe, 8 (100 ms taps, 250-300 ms apart, reads everything)

- Fast where it matters: typing ZOE across the N-Z page switch, face, age 8, skip: every tap acted
  (`sheets/zoe-1-profile-and-stars.png`). All games, the page arrow, the star button, "Home full! Stars: page 1"
  (clear to a reader), back a page, unstar Marble Kick, forward, star Toy Town, check, house: eight targets at 350 ms
  spacing, none ignored. Toy Town came first on Home. Book arrows at 350 ms spacing all turned.
- Same-spot repeats are slow (`sheets/zoe-3-home-button-stack.png`): leaving a story with the house (same spot on
  story, shelf and room), taps 2-4 at 400 ms spacing were ignored; she reached the room on the 5th, about 2.7 s in.
  Library to Home to picker: the second house tap at 400 ms ignored. See m2.
- Pets Club: tapping the splash's "tap to start" then the package (both centre screen) dropped three package taps over
  2 s; the fourth opened it (`probes/package-c03.png`, m3). After the "First Meal" sticker card, she dismissed it on
  the spot where the paw button sits and her paw tap 300 ms later was ignored (`zoe/z45_paw3.png`). Feed, cookie,
  tricks, the Sit lesson (two glow spots, "Good dog!"), Fetch (9 bones, OK) were all instant.
- Toy Town (`sheets/zoe-4-toytown.png`): in-place picks lock every other quick tap. Buns then Pony 350 ms later: Pony
  ignored. Tan then Done 300 ms later: Done ignored. Bag: Notebook packed, Water bottle ignored, Picnic cloth packed,
  Go ignored. And on "Choose your doll", Mini 350 ms after "Meet my doll" was ignored (11 px from that button), so
  she pressed Dress up and got the default doll. See m1.
- Pumpkin (`sheets/zoe-5-pumpkin.png`): START, READY, tilt to a candy, HIDE under the chair, hide under the table and
  behind the plant while the flashlight swept, three candies, "TO THE DOOR!", "YOU ESCAPED! WITH THE CANDY" in about
  45 s. PLAY AGAIN went straight to a new round. A tap on the wall picture did nothing during play; holding it about
  0.4 s showed a ring and left to Home, and the held finger's lift did nothing there.

### Leo, 6 (300 ms presses 200 ms apart, mashes 2-6, "again!" at about 1 Hz)

- No carry (good): name OK x5 onto faces; frog x4 onto ages; All games x4; page arrow x3; Toy Town tile x4; star
  button x5 into star mode; tiles in star mode acted once per mash; check x5; house x5 in Library and in Pets Club;
  Pets Club tile x5 (the package did not open); food, Kibble, paw, Sit; "Meet my doll" x5 (Mini, 11 px away, untouched);
  Done x5; Toy Town's back and Save & leave x4-5 (`sheets/leo-2-shell.png`, `leo-3-petsclub.png`, `leo-4-toytown.png`).
- Carry: age "6" x5 typed "222" into "Pick 4 numbers" (the "2" key sits under the "6"). His next press made "2222",
  "Once more!" appeared, and his mash on the same spot confirmed it: **a 6-year-old who cannot read now has a secret
  code**. The picker shows a lock on his row, and tapping it asks "Bbbb's code?" (`sheets/leo-1-profile-code-carry.png`,
  M3). Mashing his name row x4 typed two dots into that pad (`probes/pick-pk4.png`).
- Carry at the cap: All games x6 opened Radar (`leo/l13_all2.png`); house x6 in a trick lesson went lesson, tricks menu,
  room (`leo/l53_home2.png`). See m4.
- "Again!" taps: the package opened on his first again-tap, the puppy screen's next tap adopted, the celebration
  closed on the first, a tap on the dog started the lesson. Fastest answers in the whole session.
- The Sit lesson: mashing the glowing tummy counted his second press as a wrong spot, "Oops! Watch again."
  (`leo/l49_tum.png`, m5).
- Pumpkin round 1 (`sheets/leo-5-pumpkin.png`): his double tap on READY hid the pumpkin under the table on the round's
  first frame (`leo/l82_r1.png`, M1). He tapped COME OUT, then tilted right with his thumb resting at the left rim:
  the resting thumb did nothing (good). He rolled into the light, "QUICK! HIDE!", "PEEKABOO!", frosting, "YOU LOST"
  pie.
- Round 2: his double tap on TRY AGAIN hid him at the round's start again (`leo/l86_t1.png`, M1). The ghost worked at
  once ("WHAT WAS THAT?", sparkle, Grandma leaves, ghost dims). A 1.5 s press on HIDE hid as it landed and stayed hidden
  after the lift. Then, hidden, Grandma arrived and he mashed the sign: the first press brought him **out**, and every
  later press was ignored while the sign read HIDE, until he was caught (`sheets/probe-pumpkin-panic-mash.png`, M2).

### Pumpkin probes

| Probe (from the calibrate screen or play) | Result | Evidence |
| --- | --- | --- |
| Single READY tap | round starts, sign HIDE, pumpkin out | `probes/pk-a.txt` |
| Double tap READY, 100/100 ms or 300/150 ms | **hidden on the first frame** | `probes/pk-b_1.png`, `leo/l82_r1.png` |
| READY, thumb lands 100 ms later and stays | hides at once (the first report's M1 is fixed) | `probes/pk-c_held.png` |
| READY, second tap 500 ms later | hides | `probes/pk-d.txt` |
| PLAY AGAIN double tap (Zoe) / TRY AGAIN double tap (Leo) | **hidden on the first frame** | `probes/zp-pa_1.png`, `leo/l86_t1.png` |
| HIDE short tap / 1.5 s press | hides as the press lands; lift does nothing | `leo/l89_h0`, `l89_h2` in the sheet |
| Out and settled, mash 4 with gaps 150-350 ms | hides once, stays hidden | `probes/pk-g150.txt` |
| Same, gaps 450 ms | toggles hide / out / hide | `probes/pk-g450.txt` |
| Hidden, Grandma arrives, mash 11 x (300 ms, 200 ms gaps) | out on press 1, presses 2-11 ignored for 5 s, caught | `probes/pk-m.txt`, `pk-m4/m8/m9/m12.png` |
| Hidden, mash 6, then one tap 0.8 s later | still caught | `probes/pk-pn.txt` |
| Taps about 1 Hz | toggle each time | `probes/pk-v3.txt` |
| Thumb resting on the rim while tilting | nothing | `leo/trace.txt` (l84) |
| Ghost | immediate, once a round | `leo/trace.txt` (l88) |

## Findings (prioritized)

### M1 (Major): a double tap on READY / TRY AGAIN / PLAY AGAIN hides the pumpkin as the round starts
- **Screen**: Pumpkin, round start.
- **Issue**: the second press of a double tap lands on HIDE (same spot) and hides at once; Leo starts every round
  under the table without meaning to.
- **Concern**: recovery / comprehension (a non-reader sees his pumpkin vanish and the sign change; one more tap brings
  it out, so nothing is lost, but it is exactly the carry this build sets out to stop).
- **Repro**: `probes/pk-b.txt` (on the calibrate screen: `T 80 130 100`, `W 100`, `T 80 130 100`); `leo/trace.txt`
  around `shot l82_r1`; `probes/zp-pa.txt` for PLAY AGAIN.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/probes/pk-b_1.png`, `leo/l82_r1.png`, `leo/l86_t1.png`,
  `probes/zp-pa_1.png`.
- Note: a thumb that lands after the round starts and stays now hides immediately, which the brief asks for. The
  fix has to keep that while dropping a press that is the rest of the starting tap's double tap or mash.

### M2 (Major): a panicked mash while hidden brings the pumpkin out and then ignores HIDE until he is caught
- **Screen**: Pumpkin, play, hidden when Grandma arrives.
- **Issue**: the mash's first press toggles to COME OUT; each later press 200 ms after the previous lift is treated as
  the same mash with no time limit (5 s, 11 presses here), while the sign reads HIDE and Grandma searches.
- **Concern**: challenge / fairness / feedback (he pressed the hide sign the whole time and lost; the shell's 2.5 s cap
  does not apply here).
- **Repro**: `probes/pk-m.txt`: hidden (Leo trace to `shot l89_h2`), `W 800`, then repeat `D 80 130`, `W 300`, `U`,
  `W 200`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/probes/pk-m4.png`, `pk-m8.png`, `pk-m9.png`, `pk-m12.png`.
- Possible direction (owner's call): cap the mash window as the shell does, and/or ignore COME OUT for a moment after
  Grandma appears, or show the hidden state with a picture so the toggle reads without words.

### M3 (Major): mashing a button that opens a code pad types digits, and can set a code by accident
- **Screen**: new profile "How old are you?" to "Pick 4 numbers" / "Once more!"; picker row to "<name>'s code?".
- **Issue**: the pad ignores only the first nearby press for 800 ms, so the 3rd and later presses of a firm mash type
  the key under the finger. Age "6" x3 typed one "2", x4 two, x5 three; one more and a mash on "Once more!" saved code
  2222 on a 6-year-old's profile. A name-row mash x4 typed two digits into the sign-in pad (wrong tries feed the
  code waits).
- **Concern**: recovery / meaningful choice (a hidden, persistent state the child did not choose and cannot read).
- **Repro**: `probes/age-a3.txt`, `age-a4.txt`; `leo/trace.txt` from `shot l05_face2` to `shot l11_again2`, then
  `shot l95_pin`; `probes/pick-p.txt`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/leo/l06_age.png`, `leo/l09_4th2.png`, `leo/l95_pin.png`,
  `probes/age-age3.png`, `probes/pick-pk4.png`.
- This is a regression against the first screen-gate build, where four presses on "6" typed nothing.

### m1 (Minor): Toy Town picks drop the next quick tap anywhere for about 450 ms
- **Screen**: Toy Town dress-up, bag, "Choose your doll".
- **Issue**: after an in-place pick, a press anywhere within about 450 ms is dropped with no cue (Pony after Buns, Done
  after Tan, Water bottle after Notebook, Go after Picnic cloth); Mini 350 ms after arrival was dropped as near the
  opening tap, so Zoe dressed the wrong doll.
- **Concern**: pacing / feedback.
- **Repro**: `zoe/trace.txt` at `shot z89_buns`/`z90_pony`, `z93_tan`/`z94_done`, `z106_b1` to `z110_go2`, `z87_mini`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/zoe/z90_pony.png`, `zoe/z94_done.png`,
  `zoe/z107_b2.png`, `zoe/z109_go.png`, `zoe/z87_mini.png`.

### m2 (Minor): a reader's "back, back" on a house that sits on the same spot waits up to 2.5 s
- **Screen**: Pets Club story, shelf and room; Library to Home to picker.
- **Issue**: each quick re-tap on the house re-arms the gate; three taps die and the fourth acts at about 2.5 s. Ignored
  presses show no pressed look.
- **Concern**: pacing / feedback (the documented trade-off; no visual "wait").
- **Repro**: `zoe/trace.txt` `shot z75_h1` to `z82_h7`; `z144_back`, `z145_back2`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/zoe/z78_h4.png`, `sheets/zoe-3-home-button-stack.png`.

### m3 (Minor): Pets Club's package ignores a reader's taps for about 2.4 s after she taps the splash
- **Screen**: Pets Club adoption, "Knock knock! A package for you."
- **Issue**: the splash and the package are both mid-screen, so three taps at about 0.6 s spacing are dropped; "tap to
  open" appears after a delay and blinks. Same pattern after the sticker card over the paw button (`zoe/z45_paw3.png`).
- **Concern**: pacing / feedback.
- **Repro**: `probes/package-taps.txt`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/probes/package-c03.png`, `sheets/probe-package-taps.png`.

### m4 (Minor): a 6-press mash still reaches the next screen
- **Screen**: Home "All games" to Library (Radar opens); trick lesson house to tricks menu to room.
- **Issue**: the 6th firm press begins after the 2.5 s cap and acts on whatever is under it. Recoverable with one tap.
- **Concern**: recovery (documented trade-off; 6 is within the persona's 3-6).
- **Repro**: `leo/trace.txt` `shot l12_all`/`l13_all2`, `shot l52_home`/`l53_home2`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/leo/l13_all2.png`, `leo/l53_home2.png`.

### m5 (Minor): mashing a glowing spot in a trick lesson counts as a wrong answer
- **Screen**: Pets Club trick lesson, "Your turn!".
- **Issue**: tummy x3 then tail x3 gives "Oops! Watch again."; a masher can never pass. Likely predates this change.
- **Concern**: challenge / comprehension.
- **Repro**: `leo/trace.txt` `shot l49_tum`.
- **Screenshot**: `apps/porthole/docs/screen-gate-playtest-2/leo/l49_tum.png`.

### Nits
- n1: Fetch menu: "Fetch" runs into the bone and the lock covers "Word Fetch" (`sheets/zoe-2-petsclub.png`, z56).
- n2: "Read to me!" fills its button edge to edge (z66 in the same sheet).
- n3: Toy Town doll labels sit on the cards' bottom borders (`zoe-4-toytown.png`, z86).
- n4: Pumpkin's win shows the pumpkin back in the room next to Grandma; the loss says "YOU LOST" over a pie
  (`leo-5-pumpkin.png`). Out of scope.
- n5 (tooling, not game): `snap --serve` keeps playing when its save folder is missing and drops every save silently.

## Qualitative assessment

- **Can they see the goal, the actions and what changed?** Zoe reads everything and moves fast; most of her quick
  taps now land (profile, stars, Library, Pets Club care, Pumpkin). Leo navigates by pictures; Pumpkin's candies, the
  glowing door and the pumpkin squashing under furniture are visual, but HIDE / COME OUT / FIND COVER and Grandma's
  bubbles are words, so he cannot tell hidden from out except by looking at the pumpkin. The gate's dropped taps give
  no cue at all.
- **Decisions, skill, fairness.** Pumpkin has real choices (which spot, when to come out, when to spend the ghost) and
  the ghost is a delight. Fairness breaks for Leo: rounds start hidden (M1), and a scared mash brings him out and
  then ignores him (M2). Losses are short and retry is one tap.
- **Repetition, frustration, stopping.** Zoe would stop after a Pumpkin escape (about 45 s a round, a satisfying end)
  or once she starts Toy Town's Park quest (not finished here); she would call the house stack and Toy Town picks "laggy" but not quit. Leo would stop
  after his second pie: he pressed the sign the whole time and lost; the trick lesson's "Oops" loop would also end
  it. Fetch kept both happy for the full 30 s.
- **Recovery.** Every screen had a way back and every carry cost one or two taps, except M3: an accidental code that
  needs a grown-up (or luck) to clear.

## Gesture / interruption matrix

| Gesture | Shell | Pets Club | Toy Town | Pumpkin |
| --- | --- | --- | --- | --- |
| Quick tap to a new spot | instant | instant | dropped within 450 ms of a pick (m1) | instant |
| Quick re-tap, same spot | waits to 2.5 s (m2) | waits to 2.5 s (m2, m3) | not tested | toggles >450 ms apart |
| Firm mash 3-5 | no carry, except into code pads (M3) | no carry; lesson counts it wrong (m5) | no carry | acts once; from hidden comes out (M2) |
| Firm mash 6 | carries (m4) | carries (m4) | not tested | ignored for as long as it lasts (M2) |
| Double tap onto next screen | no carry | no carry | not tested | **hides (M1)** |
| "Again!" at 1 Hz | acts by tap 2 | acts on tap 1 | not tested | toggles |
| Long press | not tested | not tested | not tested | HIDE ok; exit picture hold ok |
| Held across a change | ok (Pumpkin exit hold to Home) | not tested | not tested | ok |
| Thumb resting while tilting | n/a | n/a | n/a | ignored (ok) |
| Drag | n/a | Fetch ok | n/a | n/a |
| Drag off a button | **gap** | **gap** | **gap** | **gap** |
| Leave / return | ok | ok | Save & leave ok; resume not tested | ok |
| Process restart | **gap** | **gap** | **gap** | **gap** |

## Gaps

- Not played: Biscuit, Marble Kick, Tilt FC, Rally, Kingdom, Starfighter, Radar beyond the accidental open; the
  brightness screen; Pumpkin's FIND COVER press; Toy Town resume; process restart.
- The sim takes one touch, so a thumb on the glass plus another finger cannot be modeled.
- Timing probes were black-box (frames only), but chosen with the documented numbers in mind.
- No real child and no physical board.

## Automated suite (supplementary)

`make -C apps/porthole playtest` at `553ced4`: **93 of 93 scenarios pass, 0 UI FAIL, 0 UI WARN**
(`apps/porthole/build/playtest/report.md`, read in full: all-PASS table, empty findings table, contact-sheet
links). None of M1-M3 is covered. The probe traces above are ready to become regression scenarios once the owner picks
the intended behavior. I did not add any, so the suite stays green.

## Recurrence from the first screen-gate report (read after the above)

| First report | Now |
| --- | --- |
| M1 resting thumb on HIDE never hides | **Fixed** (`pk-c_held.png`), but the fix lets a double tap's second press hide (new M1) |
| M2 quick reader ignored 2-3 s after most changes | **Mostly fixed**: taps to a new spot act at once. It remains for same-spot re-taps (m2, m3) and Toy Town picks (m1) |
| m1 "again!" taps up to 3.1 s | **Improved**: the first or second again-tap answered every time |
| m2 6-press mash reaches the next screen | **Recurs** (m4, Radar again) |
| m3 HIDE / COME OUT toggle exposes a hidden pumpkin | **Recurs, worse**: a panicked mash now comes out and then ignores HIDE until caught (M2) |
| m4 FIND COVER press, no cue | Not retested |
| n1 Marble Kick hold-home | Not retested |
| n2 Pumpkin back picture / win beside Grandma | Win beside Grandma recurs (n4); whether Leo reads the picture as "back" was not judged |
| n3 docs drift (450 ms vs 800 ms) | Fixed: `playtesting.md` now says 800 ms |
| (first report: age "6" x4 typed nothing on the code pad) | **Regressed**: it now types digits and can save a code (M3) |

## Verdict

**Needs fixes.** For shell and Pets Club navigation the gate now works for both kids: Zoe's quick taps to new targets
land, Leo's 3-5 press mashes do not carry, and "again!" taps get an answer within a tap or two. Three Major issues
remain: M1 (Pumpkin round starts hidden after a double tap), M2 (a panicked HIDE mash comes out and is then ignored
until caught), M3 (a mash on age or a name row types code digits and can save a code by accident). Retest Pumpkin
round start, the HIDE mash and both code pads with a fresh playtester after the fixes.
