# Starfighter: playable first version

Starfighter is the seventh Porthole launcher entry, labeled **Star Fighter** so the
name fits the circular launcher. It is an offline, silent arcade game with no saved
progress: four increasingly busy fighter waves, a 45-second trench, a forgiving
final torpedo shot, and replay after victory or defeat.

![Runtime screenshot tour](contact-sheet.png)

The [black-box playtest report](blackbox/README.md) records the emulator-led
refinements, a complete ordinary-input victory trace, and final gameplay frames.

## Controls

1. Hold the puck upright like a small steering wheel. Keep your comfortable grip
   still for 350 ms. **LIFT UPRIGHT**, **HOLD STEADY** and **READY TO FLY** show
   what to do next; then tap **CONTINUE** and **FLY**.
2. Bank left or right to move the ship. The neutral grip is calibrated, a small
   dead zone rejects wobble, and steering reaches its cap near 20 degrees.
3. Tap the glass for one paired red volley. A held finger does not autofire.
4. At the final port, line up the reticle and tap. A miss allows another pass.
5. Hold the exit arrow marked **HOLD** for 600 ms during flight. Menu and result
   screens have a normal **EXIT** button. Replay returns to calibration.

Small life icons and score digits follow the upper rim; four pips down the left
show wave progress. The compact HUD and smaller ships leave the center clear for
banking, enemies and incoming fire. Clearing a fighter group brings the next
approach after about 1.2 seconds.

These controls use screen-plane gravity from the existing accelerometer input.
Flat tabletop yaw is not measurable, so calibration asks for an upright grip.
Pitch, gyro input, networking and sound are not required. Three lives and 1.6
seconds of invulnerability after a hit prevent a single burst consuming a run.

## Reproduce the screenshots

From `apps/porthole`:

```sh
make snap
./build/host/snap --fresh --script docs/starfighter/tour.txt
```

The checked-in PNGs are lossless conversions of the simulator's 480x480 BMPs.
[tour.txt](tour.txt) documents the provenance: title, calibration, brief and
squadron use the normal tap path; trench, target and defeat use deterministic
stage fixtures; the accepted target shot advances normally through torpedo,
explosion and victory. These captures exercise the actual App, input tracker,
update loop and renderer. They are not physical-device photographs.

The complete mission is also exercised without state shortcuts by the rule tests.
Twelve seeded aiming pilots win in 138.34–144.58 seconds. Two novice models use
400-ms reactions and 600-ms taps: the original model, which recenters after
dodging, wins 22 of 30 seeds; a model that holds its safe lane wins 29 of 30.
Both are retained because the narrower cannons change shot coverage and make
outcomes sensitive to the steering model. These are simulation balance checks,
not claims about children's playtest results.

## Verified results

- All 62 repository simulator scenarios pass, with zero UI failures or warnings;
  this includes Starfighter's AddressSanitizer/UndefinedBehaviorSanitizer chaos run.
- `make check` and `make ci` pass; aggregate C++ line coverage is 98.4%
  (5,903 / 6,000 lines).
- Starfighter control/App tests pass 103 checks; rule tests pass 688,645 checks.
- The SDL host simulator builds and runs with the dummy video driver; the runtime
  screenshot tour and the 144 focused flow/control/stage/navigation checks pass.
- ESP32-S3 firmware builds without warnings: 6,945,610 / 8,323,072 bytes in its app
  slot (83.5%), 157,584 / 327,680 bytes static RAM (48.1%).
- Complexity and generated-art freshness checks pass.

## Verification scope

- Control and App tests: calibration from upright and angled grips; invalid,
  moving, flat and stale sensor rejection; queued taps; replay; backward clocks;
  capped long frames; fresh-page touch guards; exit from every state.
- Rules tests: natural wins and idle defeat, four waves, difficulty ramp, visible
  wing collisions matched to the smaller sprites, heavy fighters requiring
  separate volleys, paired cannon origins, bounded projectile pools, hit
  immunity, turret collisions, prompt reinforcement, HUD clearance, unlimited
  target passes, stable accepted torpedo launch, restart and determinism.
- Simulator scenarios `90`–`94`: natural launcher navigation, all game states,
  steering/fire/life changes, final missed and accepted shots, replay and exits;
  touch/tilt chaos under AddressSanitizer and UndefinedBehaviorSanitizer.
- Generated art is checked against its source. Text and controls are logged for
  the repository's circular-layout, size, spacing and contrast audit.
- A separate code review checked state transitions, timing, buffers, hitboxes,
  circle geometry and the test coverage before the final gates.

No physical board was attached. Firmware compilation and host simulation do not
establish device frame rate, panel/touch operation, or comfort in a child's hands.
The first hardware session should measure update/render/present timing with `M`
and tune banking sensitivity and combat pressure with a child.
