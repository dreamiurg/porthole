# Rally: black-box emulator playthrough

Source-free tester pass, per `apps/porthole/docs/playtesting.md` ("Required black-box playthrough").
Played on 2026-10-31 (Pacific). I did not read any Rally source, Rally tests or Rally playtest scenarios.
I did not use `dbg`, `debug`, `screen`, `ui`, `app` or any other hidden-state query. Every decision came from rendered frames.

## Build and setup

- Build: a frozen `snap` copy of revision `967d550` (`feat(porthole): Rally racing game (wip)`), run from its own scratch directory so it kept its own saves in `./build/host/`.
- Fixture (the only one): `snap --script` with `reset`, `tilt 0 1000 0`, `profile Kid 8`. After that, everything went through `snap --serve` (no `--fresh`) using only `down/move/up/tick/tilt/frame`.
- Client: a small Python process that held `snap --serve`, saved 480x480 frames and logged every command. The steering "driver" looked only at frame pixels. On Desert, Industrial and Snow it found the road by its asphalt color; on Night it found the neon edge stripes. It located the red player car on screen and set the tilt angle toward the road center. Boost and Rocket were pressed by tapping where their rings are drawn (logical (144,80) and (18,80)). I reviewed the frames every 2 s of play myself, and every 0.1-0.25 s around interesting moments.
- Harness note (unverified): race times are the game's own clock. Over the Desert race, the clock ran about 13 s longer than the summed `tick`s (80.2 s of ticks against 1:33.1). A check during the countdown showed that 90 `tilt` commands and 90 `frame` commands, sent without `tick`, did not visibly advance the 3. So I don't know where the difference comes from. The trace still replays deterministically.
- Scope: emulator only. Not tried on the physical board, so the real IMU feel, buzzer and touch latency are untested.

## Paths played

1. Launcher (Rally is on the 5th page, found with the visible arrows) -> Rally -> track cards -> How to race (Back/Next) -> Hold like a wheel. Lying flat showed "Lift it up" and kept Go disabled; tapping it did nothing. Upright showed "Hold it steady", then "Ready! Turn to test". Turning clockwise moved the preview car right, and counter-clockwise moved it left. Go -> 3-2-1-GO countdown -> race.
2. A full 3-lap race on each track, every one ending on the results page (below). Within one app session, How to race is skipped after the first time. Results -> Replay starts a fresh race at 1/3 with 1/1 charges and no how-to. Results -> Tracks opens the cards. Results -> Home goes to the launcher (checked on the replayed trace).
3. Pause: holding the lap chip for 1.5 s pauses, and a progress bar fills under "Hold". Pressing, then sliding off after 0.4 s, cancels it. Resume shows a "Ready" banner for about 1 s, then driving continues. Recenter opens the wheel page, and Back returns to Paused. Restart, Tracks and Home each ask for confirmation. I answered No to each (returns to Paused) and then Yes to each: Restart gives a fresh countdown, Tracks gives the cards, Home gives the launcher.
4. Flat mid-race (`tilt 0 0 -1000`): within 1 s the page becomes "Hold like a wheel" with Resume disabled, and tapping Resume does nothing. Lifting upright enables Resume, and Resume shows "Ready".
5. Adversarial input:
   - Taps on the road and the sky during the countdown, and on both Boost and Rocket during the countdown: no charge spent.
   - A press that starts on the road and is dragged onto Boost, then released: does not fire.
   - Holding Boost for 1.5 s: spends exactly one.
   - Tapping Boost or Rocket at 0 charges: nothing happens, no crash.
   - Double-tapping Boost or Rocket at 1 charge: spends one, and the second tap does nothing.
   - A double tap on the No confirm button: the second tap landed in a gap on Paused, so nothing extra happened.
   - Holding at the edges of the lap chip: logical y 50 and x 28 still pause, while y 54 and x 64 do not.
6. Item use and pickups: boosts and rockets were used on every track. Boost and rocket pickups were collected, and the counters reached 3 and 3. I drove over a slow patch on every track: mud (Desert), a dark slick (Industrial), ice/slush (Snow) and a teal patch (Night). I went over ramps with a visible shadow under the car. I drove off-road and along the outside of the guardrails.

## Results

| Track | Place | Time (results page) | Notes |
| --- | --- | --- | --- |
| Desert Canyon | 2nd | 1:33.1 | Green won in 1:31.9. A full input trace for this race is saved here (see below). |
| Industrial Ruins | 2nd | 1:49.9 | Green won in 1:48.1. Includes an intentional off-road excursion to the right. |
| Snow Track | 1st | 1:55.5 | Two boosts used to pass Green on lap 3. |
| Night Run | 1st | 1:48.4 | |
| Night Run (Replay, weaker steering) | 3rd | 1:50.5 | Also covers lying flat mid-race. |
| Night Run (Replay, barely steering) | 4th | 2:00.5 | Results read "Finished! 4th" with the same calm layout. No harsh wording. |

Pacing lands on the 1.5-2 minute target (1:33 to 2:00). Laps take about 30-38 s, with "Lap 2" and "Final lap" banners.

## Findings

### Major

**M1. Recenter can leave a steering offset that a later Recenter does not clear.**
- Page: Hold like a wheel, opened from Paused -> Recenter, and the race after it.
- Repro: run `repro-recenter-offset.txt` (raw `--serve` commands) in a fresh process after the fixture. It does this:
  1. Recenter while held at 10° clockwise roll. The page says Ready, but the preview car sits right, at about x 277 instead of 240.
  2. Go and race for 3.5 s at that 10° grip.
  3. Pause, then Recenter while leaned back 45° with no roll.
  4. From then on, holding the puck upright puts the preview car at x 203 instead of 240, which is a left pull. A slight clockwise roll of about 15° is needed to go straight.
- I reproduced it in four fresh-process variants (full replay, two shortened bisections, and the shipped repro file), all at x≈203, plus the original session.
- Recentering upright immediately afterwards did not clear it (3 attempts across two processes), and neither did lying flat and lifting. It did clear once: after another race segment at a 10° grip, followed by an upright Recenter. So the offset depends on the history of previous grips, not just the current one.
- In the original session the car then drifted off the left side of the road within about 3 s of a restart.
- Steering is the core control, and the one tool offered to fix it makes it worse.
- Screenshot: `06-recenter-stale-offset.png`, taken upright with no roll after the repro.

**M2. The rocket gives almost no readable feedback.**
- Page: race.
- On firing, the ring flashes orange and the count drops. The projectile is a 3-4 px orange speck that flies straight up the road and shrinks within about 0.5 s. It did not curve toward a rival 30-60 px to the side.
- In 5 firings across Desert and Snow (rivals 1-3 car-lengths ahead), I never saw a hit, an explosion, a spin-out or a "Hit!" message.
- A 5-11 year old will not know whether a rocket did anything.
- Repro: Snow Track, lap 3. With Green visible ahead and a lane to the right, tap Rocket at logical (18,80), then step 100 ms frames.
- Screenshot: `04-rocket-speck-and-mud.png`. The speck is just right of the gold car, and the mud spray is under the player.

### Minor

**m1. Guardrails and roadside objects do not block anything.**
- Steering hard pushes the car through the guardrail. It then drives clamped outside the rail at reduced speed for as long as you keep steering.
- Rock spikes, cacti, barrels and pylons pass right beside or behind the car with no contact.
- I never saw a collision with any obstacle or rival. Rivals moved out of the way when I aimed at them, and side-by-side sprite overlap with Gold and Blue caused no bump.
- So "hit something" (rock, crates, barrier or rival) could not be shown. Either collisions are not implemented on these tracks, or they never trigger on the road center line my driver followed.
- Screenshot: `05-offroad-outside-guardrail.png` (Industrial, lap 3).

**m2. Night Run off-road is the same color as the road.**
- Asphalt is (24,20,49) or (24,24,57). Off-road ground is (16,16,41), and in some bands it is exactly (24,24,57).
- Only the neon edge stripes show where the road ends. When the car slid outside the left rail on a right-hand curve, it looked like it was still on the road.
- Also, the Industrial "slick" hazard is near-black on dark asphalt and readable only by its purple outline.

**m3. Finishing has no finish moment.**
- Crossing the checkered line switches to the results page within about 0.1 s. There is no flag, banner, slow-down or celebration, and 1st and 4th get the same static page.
- It is gentle, which is good, but flat. 4th place feels OK (neutral wording, own time shown in yellow).

**m4. Recenter does not adopt the grip you are holding.**
- Recenter while rolled 10° or 20°: the page says "Ready! Turn to test", but the preview car stays offset (or pinned at the right edge for 20°).
- The label promises recentering, but it only re-checks that the puck is upright-ish. This may be intended (a fixed zero at upright), but then "Recenter" and the "Hold it steady" step mislead.

**m5. Boost feedback is weak.**
- You get a small exhaust flicker in the first ~200 ms and a cyan ring flash. Then nothing on screen shows that you are boosting, except passing cars.

### Nit

- **n1. Paused page title stays "Hold like a wheel" after lifting upright,** while Resume is already enabled (`08-flat-midrace-pause.png` shows the disabled state).
- **n2. The "Industrial Ruins" card label nearly touches the card border.** The text spans x 248-391 inside a border at 243-246 and 395-396, so it has 1-3 px of padding.
- **n3. Confirmation titles differ in size.** "Start over?" is larger than "Pick a new track?" and "Go home?".
- **n4. Yes sits on the left and No on the right on the confirm pages.** Elsewhere, forward actions (Next, Go) sit on the right.
- **n5. How to race shows again after every app relaunch.** It is skipped within a session. That is acceptable, but a returning kid taps through it each time.
- **n6. The shell's "Kid is resting. Back in 10 min" break cut in mid-countdown.** That page belongs to the shell's screen-time limit, not Rally. After OK and 10 minutes it returned to "Who's playing?", which is the expected shell behavior. Noted for the shell owner: there is no grace period to finish a race.

### Passes worth recording

- Flat start is blocked.
- Clockwise steers right.
- Hold-to-pause and canceled hold both work.
- Lying flat mid-race pauses and gates Resume.
- Every pause button and both answers on every confirm page work.
- Drag-on, hold, 0-charge and countdown taps on the items behave.
- Lap ("1/3", "2/3", "3/3") and place ("4th"…"1st") are large and outlined, and readable over every background.
- The charge digit deliberately overlaps the ring as a badge and stays legible.
- Every HUD glyph and ring sits inside the circle (the Rocket ring's outer edge is at x≈15, radius ≈225 from center).
- The road ahead stays visible above the side rings on every curve I inspected.
- The "Turn right: car goes right" caption measures 5.7:1 contrast.
- No dead ends: every page has Back, Home or Resume.

## Limitations

- **Pickup at full inventory was not observed.** Counters reached 3/3, but I did not capture a moment of driving over a pickup while full.
- **Double tap at 2+ charges not tested.** Only tested at 1 charge.
- **Pass-through collisions are partly a driver artifact.** The driver mostly followed the road center. One extra Desert lap and a half was driven with a lane offset: the car ended up riding outside the right guardrail for most of it, passing cacti and rock spikes with no contact. Even so, on-road obstacles in the outer lanes may exist unseen.
- **Night Run took two attempts.** On the first, my driver's asphalt-color mask failed on the dark palette and the car left the road on lap 1, so I restarted the process and rewrote the driver to follow the neon edge stripes. The 1st-place result is the second attempt. This was a driver problem, not a game problem. The restart is also why How to race showed again (n5).
- **Emulator only.** Tilt is idealized gravity with no noise. Real IMU jitter could change how M1 and m4 feel.

## Evidence

- Frames (native 480x480): `01-track-cards.png`, `02-wheel-flat-go-disabled.png`, `03-race-go-hud.png`, `04-rocket-speck-and-mud.png`, `05-offroad-outside-guardrail.png`, `06-recenter-stale-offset.png`, `07-paused.png`, `08-flat-midrace-pause.png`, `09-results-desert-2nd.png`, `10-results-night-4th.png`.
- `trace-desert-canyon-full-race.jsonl`: every `--serve` command from the first post-fixture command through the Desert Canyon results page (launcher navigation, how-to, wheel check, full race; 2245 commands).
  - Replay: run the fixture, then pipe the `cmd` values into a fresh `snap --serve` and take a final `frame`.
  - I replayed it in a fresh process. The final frame is pixel-identical to `09-results-desert-2nd.png` (2nd, 1:33.1).
  - Appending `down 80 139`, `tick 66`, `up`, `tick 800` reaches the launcher (Results -> Home).
- `repro-recenter-offset.txt`: raw `--serve` commands for M1. Run it after the fixture. The final frame shows the preview car at x≈203 instead of 240 while held upright.
