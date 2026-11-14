# Sand Jar: black-box first play

An agent played this through the emulator in roleplay. No real child and no
physical device were involved. The playtester did not read Sand Jar source,
its AGENTS.md, its tests or its playtests. It used no `dbg`, hidden-state
queries, `app` shortcuts or bots. Every decision came from rendered frames.

- **Build:** `aac8fb5` on branch `claude/sand-jar-port`. The working tree was
  clean apart from this folder. `build/host/snap` sha256 starts `b9b9e638332f565a`.
- **Driver:** `snap --serve` with `down/move/up/tick/tilt/shake/frame` only. A
  tap is `down X Y`, `tick 66`, `up`. Coordinates are logical 160x160. The
  complete input trace is [inputs.log](inputs.log), with `# snap` markers.
  The trace covers the first process; restart steps are described below.

## Persona and brief

**Persona:** A 6-year-old early reader. Curious and impatient with text, with
imprecise fingers. Has used a tablet but never this round puck. Tilting the
puck is a normal input, and so is shaking it.

**Brief (summary):** A new toy, "Sand Jar", was just added. Start from a fresh
profile made through the visible screens. Find Sand Jar in the launcher: it is on
the second page of All games, and holding a finger on a tile shows its name.
Open it and work out what to do from the screen alone. Tilt, touch and shake,
and try every mode you can find. Mis-tap around its buttons, drag off buttons
before release and carry a touch across a screen change. Leave with the home
button, come back and check that the jar remembers. Judge four things: whether
the kid can tell what to do, whether it is fun to fiddle with for a few
minutes, whether anything looks broken, clipped, hard to tap or confusing, and
whether the launcher picture is recognisable.

## Paths played

1. **Setup:** `reset`, then the name keyboard ("Ada"), the frog face, age 6
   and **skip** on the PIN screen. This opened the launcher. Then All games,
   the ▶ page and a long press on the striped tile, which showed **Sand Jar**
   ([01](01-launcher-name.png)). Releasing the finger opened the game.
2. **Sand (the first mode)** ([02](02-first-open.png)):
   - Tilt at about 45° each way, then 90° to the side, then upside down. The
     sand slid, held a slope and fell to the top.
   - A tap and a 1.5 s hold in empty air did nothing.
   - A finger dragged through the sand pushed a round hole along its path
     ([03](03-finger-dig.png)).
   - Tilts of 12° and 24° did nothing; the sand kept its angle of repose.
3. **Rainbow layers:**
   - Tilting both ways moved the layers slowly.
   - Six shakes only flattened the top a little.
   - Upside down gave a rainbow arch ([04](04-rainbow-upside-down.png)).
     Turning it back upright mixed all the colours.
   - A finger dragged down to the pill carved a V-shaped canyon.
4. **Sea (water over sand)** ([05](05-sea.png)):
   - A tilt made a wave.
   - Upside down, the sand sank through the water to the top. Upright, it sank
     back down.
   - A finger pushed the water.
   - The water surface took about 15 s to become level again.
5. **Glitter / space swirl** ([06](06-glitter.png)):
   - Tilt, upside down and upright again.
   - Swipes pushed the glitter, and a shake scattered it.
6. **Mis-taps on the bottom mode pill:**
   - Ten taps on the dots, in the gaps, at the ends, above and below the pill.
   - A double-tap.
   - A press on the pill, then a drag up into the jar before release.
   - A drag from the jar down onto the pill, then release.
   - A 1.5 s hold on the pill.
7. **Home button:**
   - A tap went home.
   - An 800 ms hold, then a drag into the jar, cancelled.
   - In the launcher, a held tile with its name shown, then a drag off,
     cancelled. Nothing opened.
8. **Leave and resume:**
   - The sand was sculpted, then home, then back: the shape was the same
     ([08](08-before-restart.png) shows a comparable state).
   - The mode was also kept across leave and return.
9. **Process restart:**
   - First case: the emulator was killed while in the game. The new process
     went through profile select and opened Sand Jar.
   - Second case: the rainbow jar was sculpted, then home, then the emulator
     was quit and restarted ([08](08-before-restart.png) to
     [09](09-after-restart.png)).

There is nothing to win or lose, and no natural ending; it is a sandbox toy.

### Gesture / interruption matrix

| Case | Result |
| --- | --- |
| Tap on a pill dot | Goes to the **next** mode, not the dot that was tapped (F1) |
| Off-centre tap inside the pill, its ends, about 1 px below | Same: next mode |
| Tap above the pill (y 128) or 5 px below it (y 158) | No change |
| Double-tap the pill | Skips two modes, and the jar is reset twice |
| Press the pill, drag up into the jar, release | Cancelled; the finger does not dig during the drag ✔ |
| Drag from the jar onto the pill, release | No mode change; the finger dug a canyon ✔ |
| Hold the pill 1.5 s, release | Mode changes on release |
| Hold home, drag away, release | Cancelled ✔ |
| Tap home | Goes to All games page 2 ✔ |
| Tile long press shows the name, release | Opens the game (same as tap) |
| Tile long press, drag off, release | Cancelled ✔ |
| Carry a touch across a screen change | Not possible: home, the tiles and the pill all act on release, so no touch carries over ✔ |
| Leave with home and return (same process) | Jar shape and mode kept ✔ |
| Process restart, after home or while in the game | Jar and mode lost: fresh flat sand (F2) |
| `ui` geometry audit, `make playtest` | **Gap, not run.** A scripted `ui` dump was denied by the session's permission check, and the suite was not run in this pass |

## Findings

### Major (should fix)

**F1. The mode pill looks like four choices but works as one "next" button.**
- **Screen:** Sand Jar, the bottom pill.
- **Concern:** comprehension and consistency.
- **Repro:** Open Sand Jar (sand mode). Tap the purple dot at about (97,143):
  you get rainbow, not glitter. From sea mode, tap the tan dot (63,143): you
  get glitter ([07](07-tapped-tan-dot-got-glitter.png)). Every tap anywhere on
  the pill moves one step through sand → rainbow → sea → glitter → sand.
- **Impact:** A 6-year-old taps the picture they want and gets a different
  one. With the double-tap, they also lose two jars. Either make each dot select
  its own mode, or make the pill look like a single "next" control.

**F2. The jar is not saved: a power cycle or restart loses it.**
- **Screen:** Sand Jar.
- **Concern:** recovery.
- **Repro:** Pick rainbow, tilt it and dig a valley. Press home, then restart
  the emulator. Select the profile, open All games ▶ Sand Jar. You get the
  first-mode flat sand ([08](08-before-restart.png) to
  [09](09-after-restart.png)). The same happens when the process is killed
  while in the game.
- **Detail:** Inside one process, leave and return does keep the jar. No save
  file for the jar appeared in `build/host/` after play; only the profile and
  launcher saves did.

### Minor

**F3. Changing mode throws away the current jar with no warning or undo.**
- **Repro:** Sculpt the sand, tap the pill four times to go round the modes,
  and you are back to flat fresh sand.
- **Detail:** With F1 and the pill sitting where a thumb rests, an accidental
  bump wipes the child's creation.

**F4. Shake gives almost no visible reaction in sand and rainbow modes.**
- **Repro:** Rainbow mode, upright. Send `shake` six times, 200 ms apart. Only
  the top edge flattens slightly. In glitter mode the same shake scatters
  particles clearly.
- **Impact:** A child who shakes the jar expects the sand to jump.

**F5. Touching empty space in sand modes does nothing.**
- **Repro:** Sand mode. Tap or hold at (80,50), above the sand: no visible
  response. Only touching the sand itself reacts.
- **Impact:** A first tap in the big empty area gives no feedback, so the child
  may think touch does nothing until they happen to poke the sand.

**F6. The launcher picture is only partly recognisable.**
- **Repro:** All games ▶. The tile is horizontal colour stripes in a brown
  frame with a small lid tab. At tile size it reads as a striped flag or a
  beach picture rather than a jar.
- **Detail:** The game opens in plain tan sand mode, so the first screen does
  not match the rainbow tile either.

### Nit

**N1. Water takes about 15 s to become level after sloshing.**
- **Repro:** Sea mode. Tilt to (900,400) for 2.5 s, then upright. The slope is
  still visible at 4 s and 6.5 s and is level by about 21 s.

**N2. A single water pixel stays stuck on the glass wall.**
- **Repro:** Sea mode, upside down and back, then wait 15 s. One light-blue
  pixel stays at about (7,49), floating above the water.

**N3. The glitter mode is dim, and its dot is a plain lavender circle.**
- **Detail:** At native size the swirl is pretty but low-contrast, and the
  plain circle gives no hint of sparkles.

**N4. Tilts under about 25° do not move the sand.**
- **Detail:** This is a realistic angle of repose. Rainbow layers also move
  slowly at 0.8 g. Listed as a pacing observation only.

No clipping outside the round glass was seen. The pill, the home button, sand
at the rim and upside-down sand all stay inside the circle. The home button
draws over sand that is piled at the top.

## Qualitative assessment

**Can the child see the goal and actions?** There is no goal to read, which
suits an early reader: there is no text at all in the game.
- The jar of sand says "tip me" by itself.
- The first tilt gives instant, satisfying feedback.
- Touch is found by poking the sand: the finger leaves a round hole, and that
  reads clearly (F5 is the exception).
- The pill's dots read as "pick a jar", but they do not do that (F1). This is
  the main point of confusion.

**Decisions, feedback, pictures:** Pictures carry everything, and nothing needs
reading. Cause and effect are clear and physical.
- Turning the rainbow jar upside down makes an arch. Turning it back mixes the
  colours, which is an "uh-oh / whoa" moment a 6-year-old would repeat.
- In the sea jar, the sand sinks through the water.
- The finger carves canyons through the layers.
- There is no challenge and no failure, which suits a fidget toy.

**Where it gets repetitive or frustrating:**
- Sand mode on its own is quiet: small tilts do nothing, and shaking does
  little (F4).
- The persona would leave sand mode within about 30 s and look for something
  else.
- Rainbow and sea held attention longest, roughly 1–2 minutes each.
- Glitter is pretty but passive.
- Estimated first voluntary stop: about 3–5 minutes, after flipping the rainbow
  jar a few times and making waves. That is a fine stopping point for a toy.
- Frustration comes from F1 (the wrong jar appears) and F3 (my picture went
  away).

**Recovery from imprecise input and interruptions:**
- Drag-off cancels work on home, the tiles and the pill. Dragging from the jar
  onto the pill does not switch modes.
- Leave and return keeps the jar.
- A stray tap on the pill silently wipes the jar (F1, F3).
- A power cycle loses everything (F2).
- No adult help is needed to get out: home is always visible.

**Positive case:**
1. Pick rainbow.
2. Turn the puck upside down: the layers pour into an arch ([04](04-rainbow-upside-down.png)).
3. Turn it back: the colours tumble and mix.
4. Drag a finger down through them: a V-shaped valley shows the stripes.

This is the toy at its best.

**Friction:** F1 and F3 together. The child taps the purple dot for "sparkles",
gets rainbow and taps again; the rainbow they had mixed is gone.

## Iteration history and judgment

- This is the first pass, with no fixes yet. No brief changes are needed.
- Suggested brief addition: say whether the jar is meant to survive a power
  cycle, so that F2 can be judged against the intended behaviour.
- **Gaps:** The `ui` geometry audit, so pill and dot hit sizes were measured
  only from taps. `make playtest`. Per-profile jar separation. A real device
  and a real child.

**Judgment: needs another pass.** F1 and F2 should be fixed, then the mode
pill and resume should be replayed on the new build.

## What was done about it

- **F1 (pill reads as four choices):** not changed. Picking a dot per tap needs four
  targets inside the 48-cell pill, 12 cells each, and the UI audit wants 18 (6 mm)
  for a child's finger. A wider pill or a single "next" look is a design call left
  to Дмитро.
- **F2 (jar not kept across a restart) and F3 (a new mode starts fresh):** as
  designed. Sand Jar saves nothing; its mode is kept only until the device restarts.
- **F4-F6 and the nits:** left as they are for now; the launcher picture can be redrawn later.
