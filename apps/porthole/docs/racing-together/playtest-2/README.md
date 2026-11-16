# Rally "Together" playtest, round 2

Build: 06475ce, `build/host/snap` sha256 07c4ec446b027424848b48f73353f9d5e7aa3c3eff7dd4235a23d02389b90438.
Same setup as round 1 (`../playtest/README.md`): two `--serve --fresh` sims over UDP 127.0.0.1, 66 ms interleaved
ticks, siblings ~6 (non-reader) and ~8, profiles Mia (A) and Kai+Leo (B), Rally opened from the launcher.
Same agent as round 1, so this is not a source-free first play. `dbg lastlap` used only in `inputs/r4.py results`.
`make playtest`: 104/104 pass. Emulator roleplay only.

## Round-1 fixes re-checked
- Both kids tap Together on the same tick, or 0.33 s apart: both in the lobby. `01_simul_*`, `02_stagger_*`. Fixed.
- Together tapped while the invite shows: joins. `03_*`. Fixed.
- Not now: host goes back to the cards with "Leo said not now"; tapping Together again re-invites and Join works.
  `04_*`. Fixed.
- Together has a 2 px gap now; a tap at (80,31-32) does nothing and the suite's spacing WARN is gone. Fixed.
- Leaving reads "Leo left" / "Mia left" in the lobby, on the grip screen and on results; "Mia left. Keep racing!"
  mid-race; a killed device still shows "Lost Leo, keep racing!". `09`-`15`. Fixed.
- Guest lobby line moved: it now sits on the Back button's top border instead of the track name. `06_lobby_B.png`.

## New findings (all Minor)
1. Not now, then the guest taps Together within about 2 s: the guest gets a "Joining..." lobby with "..." for the
   friend, then "Race is off" after 2.2 s; the host never sees an invite. After 2 s or more it works. `inputs/r2.py`,
   `r3.py`; `05_*`, `08_t*`.
2. "Mia picks the track" now overlaps the Back button's border. `06_lobby_B.png`.
3. Pacing: in the natural run the second kid finished 40 s later, and the first kid waited on a static
   "Leo racing..." screen. `28_a_done_A.png`, `28_b_still_B.png`. My steering was crude, so the gap is not a difficulty
   verdict.

## Fresh-eyes pass (`inputs/r5.py`, off-centre taps)
Cards, invite, lobby (host changed the track), grip with "Waiting for Leo...", countdown, Industrial Ruins race with
friend faces, tailwind streaks on the trailing kid, a pause ("Mia paused" on B), results with both faces, "+2 stars",
"Leo wants again!", Again into a shared lobby. No dead ends. `20`-`31`.

Still open from round 1 (not changed): reading load for the 6-year-old (Major 4) and Minors 7, 8, 10, 11, 12.

## Resolutions after round 2 (Claude)

- Minor 1 (Not now, then the guest taps Together within ~2 s): fixed. Not now now forgets that invite, so the stale one
  can't pair; regression test `notNowThenTogether` in `host/test_racing_together.cpp`.
- Minor 2 (the guest's "picks the track" line on Back's border): fixed. The guest's heading now reads
  "<Name> picks the track" at the top, and the bottom line is gone.
- Major 4 (reading load for a non-reader): mitigated. Join is now a green button with green text and Not now is a dim,
  quiet one, so yes stands out under the inviter's face. A 6-year-old alone may still need a sibling to read the
  invite; accepted for this MVP, since the kids race in the same room and a wrong pick is now recoverable.
- Deferred as known gaps (never block play or lose progress): minors 3, 7, 8, 10, 11, 12 from round 1, pacing for a
  much slower second kid, real wifi latency, a physical device, restarting a dropped device.
