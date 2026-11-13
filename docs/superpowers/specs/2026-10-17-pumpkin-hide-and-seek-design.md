# Pumpkin Hide & Seek — concept / Codex handoff (2026-10-17)

Status: **Concept and prototype brief, not implemented.** The attached SVG storyboard is a schematic local art reference; earlier generated concept boards in the ChatGPT design discussion are more detailed visual references and are **not** present as raster images in this commit.

## Idea and tone

A playful, slightly spooky Halloween hide-and-seek game for children in grades 1–4 (roughly 6–10). The hero is a small expressive pumpkin who tries to avoid being caught by Grandma in her warm, shadowy house. Grandma is affectionate, funny, and relentlessly determined to bake pumpkin pie. If she finds the hero, a theatrical, horror-film-inspired **orange frosting drip** rolls down from the upper rim and reveals a pie labelled **YOU LOST** in bold icing. No gore, harm, or frightening jumpscares: the absurd pie is the joke. A tap immediately allows a retry.

This is primarily a **visual and animation experience**, rather than complex gameplay. It should feel delightful and readable at a glance.

## Confirmed hardware and platform

- Porthole firmware on Waveshare ESP32-S3-Touch-LCD-2.1, **round 480×480** display, capacitive touch and QMI8658 tilt sensor.
- Implement as a **game within** `apps/porthole/games/pumpkin-hide-seek/`, sharing Porthole shell / OS / board abstractions. **Not** as a standalone `apps/<app>`.
- Consult root `CLAUDE.md`, `apps/porthole/CLAUDE.md`, `docs/hardware.md`, and existing games before coding.
- Prefer the existing **160×160 indexed framebuffer scaled 3×** for chunky pixel art, if practical for the game, and adopt an intentionally distinct palette and visual vocabulary from Pets Club. Use native 480×480 selectively if necessary for readable large text/effects.
- All important action and typography must remain inside the circle; no control at clipped corners.

## Agreed style: retro pixel art

Think cohesive 16-bit Halloween arcade with expressive pixel characters and layered interiors, not painterly 3D. Chunky black outlines, dithering only where useful, limited palette: pumpkin orange, cream, dusty purple, midnight blue, wood brown, warm lantern yellow, sage green. Grandma has grey bun, round spectacles, purple cardigan, patterned apron and a hand-held flashlight. The pumpkin has an oversized readable face, small green stem and bouncy squash/stretch poses. Cozy house interiors: fireplace, couch, dining table, pantry, plant, closet, attic toy chest. Lighting creates playful shadow and silhouette moments.

The attached `pumpkin-storyboard.svg` conveys four frame compositions; it is **not** a final sprite sheet. The previously shown ChatGPT concept images explored title/intro, Grandma searching with flashlight, hiding behind chair/curtains, and pie loss screen, along with room variations. Recreate these visually in native pixel art rather than embedding oversized concept boards into firmware.

## First-playable loop (proposed)

1. **Start:** pumpkin on rug in living room, `TAP TO START`, moon in window; single concise instruction `TILT TO ROLL • TAP TO HIDE`.
2. **Explore:** neutral-grip calibration as already implemented in sibling tilt games; tilt left/right to roll pumpkin across one room. Start with a single horizontal lane, 2–3 distinct hiding spots (behind chair, under table, by large plant). Acceleration/friction tuned to be forgiving.
3. **Hide:** when close to a highlighted hiding spot, tap to duck into it. A hiding pose clearly indicates safety; tap again to emerge, or tilt out depending on tested UX. Grandma enters periodically, searching with a broad flashlight cone.
4. **Search:** sight-based danger is strongly telegraphed (footsteps, yellow light, Grandma silhouette and 'Hmm?'). Staying fully concealed defeats a pass. If the player is exposed in the cone long enough, Grandma notices and a brief comedic catch sequence plays.
5. **Win:** survive a short easy round (target 30–60 seconds, tune for kids). Pumpkin grins, Grandma says she must have misplaced the pumpkin, `YOU DID IT!` and `PLAY AGAIN`.
6. **Lose:** freeze; Grandma leans into frame, smiles, an orange icing drip descends from **top circular rim**, gradually covering the scene like classic horror-film blood drips; transition to giant pumpkin pie with legible `YOU LOST` icing (orange letters on cream frosting, or cream letters on orange frosting—maximize contrast), one big `TRY AGAIN` target. Comedic, not threatening.

No lives, no score, no persistence requirement, no waiting after failure, no irreversible consequences. A short retry should bring player back immediately.

## Art storyboard beats

- Frame A: pixel pumpkin center foreground, cozy room background, start prompt.
- Frame B: pumpkin rolls behind chair while Grandma searches from right with flashlight; clear yellow cone.
- Frame C: close call: face peeks below curtains/table as cone passes, no cluttered HUD.
- Frame D: pie covers most circular viewport; orange frosting drips from top, large readable **YOU LOST**, tappable retry.

See `pumpkin-storyboard.svg` in this folder for a compact 4-scene visual map. Existing rendered concept artwork in the original conversation is additional inspiration, not a required external dependency.

## Scope guidance to Codex

First build a **single-room playable vertical slice**, not five rooms or a giant game. Prioritize memorable pixel art, squash/stretch animation, a convincingly sweeping flashlight cone, and the dripping frosting end sequence. Avoid complex pathfinding. Extend the same room with a few randomized hiding and search patterns only after the core loop is fun.

Potential expansion later: kitchen/table, living room/sofa, attic/toy chest, hallway/closet, and pumpkin patch/other pumpkins; Grandma's slippers sound cue, disguise as a basketball, pumpkin among lanterns, a playful ghost friend.

## Acceptance criteria

- Available in the existing Porthole launcher as a distinct game; shell back navigation works.
- Runs in host simulator and browser emulator, with a plausible tilt substitute in emulator.
- Correct no-corners screen design: no clipped text or buttons; tap targets are child-friendly.
- Playable by tilt and touch on hardware; neither requires impossible gestures.
- Pixel art crisp and distinct from existing game palettes/assets.
- Lose sequence shows **animated frosting entering from top of the circular screen**, then pie with **YOU LOST** readable and simple **TRY AGAIN**.
- Win and retry paths work. No startling audio; honor global mute.
- Run app `make check`, `make ci`, relevant firmware compile and scripted playtests; report actual outcomes and any gaps.
- Follow existing project process: document UX states, create/update golden screenshots with pixel-preserving scaling, and open a PR for review. Do not merge automatically.

## Gameplay update: phase 1 (2026-10-17)

Playtesting the slice showed one dominant strategy: hide at once and wait out the timer.
Phase 1 gives the pumpkin a reason to move and Grandma some surprises, still in one room:

- **Goal:** collect three candy corns on the floor, then roll out the door at the left wall.
  The 40-second timer is gone; hiding only stalls.
- **Grandma's moves vary every round:** full sweep, fast sweep, a sweep that turns back,
  and a spot check ("THE TABLE?") that finds a pumpkin hiding there, so the player has to
  pick a different spot. Every move is announced first; checks get the longest warning.
- **Near misses:** the hidden pumpkin shivers while the beam passes over it, then PHEW!. *(Superseded: PHEW!
  now waits for the search to end; see "Amendment: hold to hide, fix pass".)*

Later phases (not built yet): more rooms with one new idea each, and optional stars and hats.

## Gameplay update: phase 2 (2026-10-17)

- **Hold still while hidden:** a hard, sustained wiggle rustles the hiding spot (SHH!);
  Grandma heard it, and her next move checks that spot, with the usual long warning.
- **Ghost friend:** once a round, tap the ghost at the window to knock something over;
  Grandma stops what she is doing and goes to look, buying a few safe seconds.
- **Endings that change:** catch lines, pie captions and escape lines vary per round.
Exact numbers live in `apps/porthole/games/pumpkin-hide-seek/CLAUDE.md`.

## Amendment: hold to hide (2026-11-13)

A child-perspective playtest (`apps/porthole/docs/pumpkin-hide-playtest.md`) found a 6-year-old who presses
1-2.5 s sitting in the flashlight until he let go (HIDE acted on lift), and a worried double tap that hid and
popped him straight back out. Hiding is now a hold, not a toggle:

- The pumpkin is hidden exactly while a finger that pressed down on the action sign stays on the glass. It
  hides on the press if it is within 13 px of a spot, otherwise the moment it rolls within 13 px while still
  held ("hold and roll into cover"), and comes out when that finger lifts. A finger that drifts off the sign
  keeps it hidden.
- No toggle: every press hides while held and every lift comes out, so a double tap can never flip it out
  mid-press.
- Unchanged: tilt never ejects a hidden pumpkin, the rustle rule, no candy while hidden, a thumb resting on
  the glass when the round begins does nothing until it lifts, the ghost is a tap, Home is a 600 ms hold.
- Sign: HOLD TO HIDE near a spot, FIND COVER away from one, HIDING while hidden; it draws pressed while held.
  RUN AWAY! and QUICK! HIDE! now mean "press and hold". *(Superseded: the sign draws pressed only while hidden,
  and a slide-on arms only within reach of cover; see the two fix-pass amendments below.)*

## Amendment: hold to hide, fix pass (2026-11-13)

The kid playtest of the hold (`apps/porthole/docs/pumpkin-hold-playtest.md`) found a non-reader holding a
gold-lit FIND COVER in the open and being caught (M1), plus four minor gaps. Changes:

- **The sign tells the truth.** It draws pressed (gold) only while the pumpkin is actually hidden. Away from
  cover it is flat and dim, in the game's own WALL and GREY, reads FIND COVER beside an arrow that bobs toward
  the nearest spot, and a touch there shakes it side to side for about half a second: a "no" without words.
  Seen while hidden (a Check of the kid's own spot), it reads RUN!, agreeing with RUN AWAY!. *(Superseded by the
  fourth fix pass: the pumpkin startles out of a named spot, so the RUN! sign is gone.)*
- **A finger that slides onto the sign hides**, wherever it came from (scenery, the ghost, Home), and keeps
  hiding until it lifts. A thumb resting since before the round is still ignored until it lifts, and a press
  slid from the ghost onto the sign never fires the ghost.
- **Mashing can't beat a Check.** Hiding no longer resets exposure; only leaving the beam or hiding where she
  isn't looking does, so tapping the sign during a Check of that spot is caught like holding there.
- **Rolling into cover while held is not a rustle.** The hard tilt counts only after it has once eased below
  the rustle threshold since that hide began.
- **PHEW! waits for her to give up.** It shows when a search ends after the beam was on the pumpkin, not each
  time the beam passes, so it never invites a 6-year-old to let go mid-search.
- Candy positions were checked, not changed: each round's seed mixes the millisecond clock and a play counter,
  and consecutive rounds do differ, but only within the three narrow lanes (15, 15 and 7 px), which reads as
  "the same spots".

## Amendment: hold to hide, second fix pass (2026-11-13)

The second kid playtest (`apps/porthole/docs/pumpkin-hold-playtest-2.md`) found that a Check of the kid's own
spot looked safe until too late (M1), and that a press on the dark sign showed nothing a 6-year-old noticed
(m2). Review added two gaps. Changes:

- **A Check of your own spot never looks safe.** Hidden in the spot Grandma names, from the warning's first
  frame, the sign turns rust and reads RUN!, and the pumpkin shivers and sweats in its spot. Seen at all
  during a Check, mashing included, the label is RUN AWAY!, never QUICK! HIDE!: hiding can't help there.
  *(Superseded by the fourth fix pass: it startles out instead; the shiver and sweat are back to near misses.)*
- **The sign never invites a hide in the named spot.** Out in the open, FIND COVER's arrow points to the
  nearest spot she isn't naming, and within reach of the named spot the sign stays FIND COVER rather than
  HOLD TO HIDE and the furniture's in-reach strip stays dark. *(Superseded in part: this pass still let a
  press there hide, under a RUN! sign; the third fix pass below refuses it.)*
- **A finger crossing the sign in the open arms nothing.** A finger slid onto the sign arms only where the
  pumpkin can hide at once. A press that began on the sign still hides on arrival, wherever it drifts.
- **"No" you can see.** The old 2 px, 480 ms shake was in the playtest's frames but went unnoticed. A finger
  reaching the sign away from cover now shakes it 3 px and swaps the words for three cream chevrons
  marching toward cover for 1 s, visible in any single frame. No "no" if the pumpkin is already rolling into
  reach (within 0.5 s at its speed). *(Superseded by the third pass below: the look-ahead rolls a copy, and only
  an armed hold skips the "no".)*

Deferred findings, one line each (`apps/porthole/docs/playtesting.md`, acceptance rule 5):

- Playtest 1 m3 (named spot is a reading-only rule): the RUN! sign and shiver fix it for a pumpkin already
  hidden there; it is fixed outright only by the third fix pass, which refuses a new hide in the named spot.
  *(The fourth fix pass replaces the RUN! sign with the startle.)*
- Playtest 1 m4 (rustle explained in words): the spot jiggles with marks and "!" over Grandma; rolling in is no
  longer heard, and a heard rustle only brings a warned Check, so it costs a visit, not the round.
- Playtest 1 m6 / playtest 2 m5 ("YOU LOST"): out of scope for this pass; the pie captions keep it playful
  and neither persona was upset. A wording change is game-designer's call.
- Playtest 1 m7 (Grandma returns 0.5-1 s after leaving): out of scope; the 2.5 s minimum rest holds and it
  cost no round in playtest 2. Tuning is game-designer's.
- Playtest 1 n1 / playtest 2 n3 (stale bubble after she leaves): harmless; "HIDDEN!" and "PHEW!" are true
  statements, and an all-clear beat would be a new mechanic.
- Playtest 1 n2 (a candy under the pumpkin or her shoe): rare and self-correcting once the pumpkin moves.
- Playtest 1 n3 (used ghost silent): it is redrawn dark and still, which reads as spent; once a round is
  the rule.
- Playtest 1 n4 / playtest 2 n4 (no pressed look on START / TRY AGAIN; TRY AGAIN deaf just after the pie): the
  screen-change guard keeps a press carried from the round or a mash through the frosting from skipping the
  pie, and START, READY and TRY AGAIN all act on lift of a 1-3 s press, which both personas managed.
- Playtest 1 n5 / playtest 2 n2 (win card overlap, win picture): cosmetic, for pixel-artist.
- Playtest 1 n6 / playtest 2 n1 (pumpkin slow for an 8-year-old): suits the 6-year-old; speed is tuned for
  the youngest players and the 0.7 s-late bot.
- Playtest 1 n7 / playtest 2 m1 (wall-picture exit, 600 ms hold): out of scope; to outlast a 6-year-old's
  ordinary 1-3 s press the hold would have to exceed 3 s, which fights a way out that must always be
  reachable. The cost is real: a mistaken exit ends the round, and the next visit calibrates again.
- Playtest 1 m5 / playtest 2 m3 (same candy lanes, one room): out of scope; varying them is a design change
  for game-designer (more rooms are the planned later phase).
- Playtest 1 m5 (the ghost trivializes the first visit): it is once a round, so spending it on the opener,
  the easiest visit, leaves every later Check and fast sweep unaided; a quick round is the reward for a kid
  who found the trick, not a hole.
- Playtest 2 m4 (ghost's purpose invisible to a non-reader): out of scope; Grandma visibly turns to the crash
  and leaves, which an 8-year-old decoded. A clearer cue is game-designer's.
- Playtest 2 n5 (sign hit area has no padding): 80x24 already exceeds the 24x22 floor.
- Playtest 2 n6 (arrow flips between two spots): it always points at the nearest real cover, so a flip at the
  midpoint is truthful. *(Superseded by the fourth fix pass: near the midpoint it points the way it rolls.)*

## Amendment: hold to hide, third fix pass (2026-11-13)

The third kid playtest (`apps/porthole/docs/pumpkin-hold-playtest-3.md`) judged the hold acceptable, with one
minor worth fixing (m1); review found the named spot still took a new hide. Changes:

- **The named spot is not cover.** Once a Check names a spot it is decided: a new hide there does nothing, so a
  press there gets the chevron "no", a finger slid on there doesn't arm, and a pumpkin held through it rolls
  on and hides in the next spot. One hidden there before she named it stays hidden under RUN!. *(Superseded
  by the fourth fix pass: one hidden there startles out.)*
- **"No" while rolling, unless it really arrives.** The 0.5 s look-ahead rolls a copy of the pumpkin at the
  current tilt, friction included, toward a spot she isn't naming. Speed alone promised arrival for a pumpkin
  just levelled, which stops short, so a 6-year-old's press while it coasted showed nothing (m1).
- The shiver and sweat show only in Play, not on the catch and frosting pages. *(Moot after the fourth fix
  pass: they show only for a near miss, which never outlasts Play.)*

Deferred playtest-3 findings, one line each:

- m2 (a forgotten thumb keeps it hidden and frozen): the hold lasting wherever the finger drifts is the fix
  for playtest 1's mid-search pop-out; lifting recovers at once, the spot jiggles under SHH!, and a heard
  rustle brings a warned Check with the RUN! sign, so holding on costs a visit before it costs the round.
  *(The fourth fix pass startles it out of that Check instead.)*
  A "lift your finger" picture would be a new cue for game-designer and pixel-artist.
- m3 (a mash carried through the frosting restarts from YOU LOST): out of scope; the 2.2 s catch and frosting
  already show the loss, and a longer guard on TRY AGAIN would make it deaf to a deliberate press.
- m4 (wall-picture exit after a firm press): as playtest 1 n7 above; Leo never hit it by accident in play.
- m5 (repetition, passive sweeps): out of scope; candy lanes and Grandma's moves are game-designer's, and the
  bot suite keeps every round winnable at the current pacing.
- n1 (bubble flicker during a rustle): each label is true when shown and the rustle cue is meant to be loud;
  debouncing the label row is a separate change.
- n2 (no pressed look on START / READY / TRY AGAIN): as playtest 1 n4 above.
- n3 (the "?" bubble, pointing arm or beam pool left on a result card): cosmetic, and it would change the
  victory golden; a follow-up for pixel-artist.
- n4 (the READY! label draws a press): the stumble cost Leo nothing, and his next press found the button
  just below it.
- n5 (unseen at Grandma's feet): the beam is the rule, and it is drawn; a 6-year-old can read the light.
- n6 (slow pumpkin): as playtest 1 n6 above.

## Amendment: hold to hide, fourth fix pass (2026-11-13)

The fourth kid playtest (`apps/porthole/docs/pumpkin-hold-playtest-4.md`) found a 6-year-old non-reader caught
in three of eight checks of his own spot: the rust RUN! sign was text only and the pumpkin stayed tucked in, so
he held on (M1). Changes, decided by the lead:

- **A named spot stops being cover for everyone.** On the step a Check's warning begins on the spot the pumpkin
  hides in, the pumpkin startles out at that spot's x: it hops 4 px with a cream "!" for 400 ms. A finger still
  held stays armed, so it hides at the next spot it rolls into; the named spot takes no new hide. A heard Check
  startles it the same way; the teaching opener is never a Check. A ghost during that warning hands the move
  back, so the spot is cover again and the held finger hides it there on the next step. The RUN! sign state is
  gone; the shiver and sweat are back to meaning a near miss.
- **In reach of the named spot during the warning** the label says RUN AWAY! and the sign is the dark FIND
  COVER, its arrow toward the nearest other spot.
- **The "no" lasts while the finger stays.** The chevrons start on the frame a finger reaches the sign where it
  can't hide and last as long as it stays there with no cover, at least 1 s after a quick tap. A press that
  stopped just short of cover with the finger down showed a calm sign, which Leo took for hiding (m1).
- **The arrow doesn't flip while coasting.** Within 6 px of the midpoint between two spots it points the way
  the pumpkin rolls, so coasting past turns it once, ahead of the pumpkin (n1).
- The bots still win 100 of 100 seeds sharp, 0.7 s late and fidgety; Grandma's timings are unchanged.

Deferred playtest-4 findings, one line each:

- m2 (Grandma returns 2-3 s after leaving, long holds): out of scope; her timing is game-designer's, and the
  2.5 s minimum rest and the 0.7 s-late bot keep every round winnable.
- m3 (a forgotten hold plus tilt rustles, explained in words): out of scope; a rustle picture is new art for
  pixel-artist, and the startle now ends the heard Check's trap without the kid lifting.
- n2 (tapping the pumpkin or furniture does nothing): out of scope; tap-to-hide in the room would be a second
  hide control beside the sign, a design change for game-designer.
- n3 (caught at Grandma's feet while her beam points away): as playtest 3 n5; the beam is the rule and drawn.
- n4 (the loss card doesn't show what caught him): out of scope; a cause picture on the pie card would change
  the pie golden, for pixel-artist and game-designer.
