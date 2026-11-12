# Starfighter: agent brief

A short, silent space arcade run for the round 480x480 Porthole. The launcher
name is **Star Fighter**, which wraps inside its tile; the title art says
**STARFIGHTER**. Namespace `starfighter`; design and visual storyboard live in
`docs/superpowers/specs/2026-10-17-starfighter-hybrid-game-design.md` and
`starfighter-storyboard.svg` at the repository root.

## Player contract

- Every launcher entry begins at Title. Start opens the grip lesson; hold the
  puck upright like a steering wheel and steady for 350 ms, then tap Continue.
  A short mission brief leads to a fresh three-life run. Flat tabletop yaw
  cannot be measured by the current accelerometer-only firmware, so flat or
  implausible readings never pass calibration.
- Rotate the upright puck clockwise to bank right, counterclockwise to bank
  left. Calibration subtracts the comfortable neutral screen-plane gravity
  angle. A 3-degree dead zone rejects wobble, 20 degrees gives full steering,
  larger banks are capped, and a 100-ms filter smooths changes. Pitch is not a
  second required control. Flat or invalid readings during flight give zero
  steering rather than an unstable gravity angle.
- One short tap anywhere on usable glass fires one paired red volley. It fires
  on release; a held finger does not autofire. Events survive render frames
  shorter than the fixed simulation step. Even a short tap on the exit sign
  fires during flight; only a deliberate 600-ms hold on that sign exits.
- Title, calibration, brief and both results have a visible tap-to-exit sign.
  Other flight stages have a visible hold-to-exit sign. The press must begin
  inside it and remain inside; drifting off resets the hold, and dragging in
  from elsewhere cannot leave. FreshGate protects every page or stage change
  from carried touches and rapid double taps.
- Four enemy waves lead through a brief entrance to a trench, then a generous
  final targeting opportunity. A missed final shot offers another pass. The
  torpedo and explosion lead to Victory; zero lives leads to Game Over.
  Play Again/Try Again returns to calibration and starts a complete fresh run.
- No saves, collectibles, daily progression, personal data, network or sound.
  Leaving and returning starts again at Title. Wall-clock changes do not
  change a run. Large frame gaps are capped at 100 ms; backward monotonic-clock
  jumps advance nothing. The shell's daylight setting and tint are respected.

## Module map

- `tune.h`: allocation and difficulty bounds, fixed 20-ms simulation step.
- `rules.h` / `rules.cpp`: bounded, deterministic `Run`, seeded `restart`,
  normalized-bank / tap `step`, stage progression and collisions. Native panel
  coordinates, positive y downward. No input or drawing dependencies.
- `controls.h`: validated upright calibration and direct gravity-angle banking.
  `tilt::Steady` supplies the recent average; elapsed coverage and a stable
  anchor also reject an isolated sample or moving grip. Generic `tilt::from`
  flattens gravity for two-axis tilt games; this game explicitly measures
  rotation in the screen plane for a steering-wheel gesture.
- `game.h` / `game.cpp`: App contract, four menu/flight pages, fresh touches,
  hold exit, queued taps, elapsed-time safety, no persistence.
- `render.h` / `render.cpp`: full-frame RGB565 pixel-art rendering and shared
  logical-pixel hit boxes. `assets.h` and `tools/` own the original sprite art.
- `host/test_starfighter.cpp`: control, calibration and app-flow proof.
- `host/test_starfighter_rules.cpp`: pure-rule collisions and natural run proof.

Every text and hit-box corner must stay inside the round glass. Main is
`{43,106,74,24}`, static Exit `{62,132,36,24}`, flight Hold
`{64,9,32,24}` in logical 160x160 pixels; every target is at least 72 physical
pixels in each dimension. Flight objects use native 480x480 pixels.

## Simulator and test hooks

Screens: `sf_title`, `sf_calibrate`, `sf_brief`, `sf_squadron`, `sf_waveclear`,
`sf_entry`, `sf_trench`, `sf_target`, `sf_torpedo`, `sf_explosion`,
`sf_victory`, `sf_defeat`.

`dbg title|calibrate|brief` opens a teaching/menu page.
`dbg start|squadron|waveclear|entry|trench|target|torpedo|explosion|victory|defeat`
starts a deterministic fresh fixture at that stage, using the current upright
grip when valid. These are reachability fixtures, not evidence of natural
progression. `dbg hit` applies one life loss only outside invulnerability.
`dbg lastwave` sets up the last wave-clear transition; `dbg trench-end` opens
the final 100 ms of the trench, exercising the normal stage transitions.
`debug` reports stage, lives, score, wave, volley count, player x, bank in
thousandths, invulnerability ms, target passes, run clock, calibrated gravity,
calibration readiness, target x and alignment.

## Hardware validation still required

The host can prove the sensor geometry and bounded response. A child holding
the printed case must still validate the 20-degree banking range, steering
feel, difficulty, and the real device's frame rate before those are treated as
final tuning.
