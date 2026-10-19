# Independent emulator playthrough — archived pre-visual evidence

**Archived functional playthrough — before the visual revision.** The user
rejected the art and typography captured here. This report preserves the actual
inputs, gameplay findings, build identities, and replay results of that earlier
presentation. Its screenshots and readability judgments do not certify the
replacement based on the original nine-screen SVG, GPT-generated art, and
Atkinson typography. Revised-build visual playthroughs will be added separately.

Independent tester used only the rendered emulator, touch input, ordinary elapsed
time, process restart, and the documented emulator protocol. The tester did not
read the game implementation, manuscript source, tests, hidden state, or design
solutions. All screenshots are native 480 × 480 frames.

## Complete shared-schedule run — before visual revision

**Functional outcome: passed a complete natural Chapter One run.** The party
earned one deed, read the shared ending, returned to the changed world map, and
resumed that same completed map after Save & quit and a process restart. This
report precedes the subsequently requested font/art/composition revision and
does not certify that revised presentation.

The run began on emulator SHA-256
`72bc418b1e084df44471038332d02b39b860d545f76626be37eb1e3f2538c5b9`.
During the pass, process restarts loaded `85451edd…` (three optional encounter
prose corrections), then `c3cd4b82…` (quiet-visit caption fix). The last
post-ending restart of this archived pass loaded
`f5a1d7b8c4acbcd015c632ed0de9a9dcd2a49b5fdea800cd575f3e745336246d`
(internal screen-name discovery change). No story progress fixture or hidden
state was used. A fresh replay of that archived binary was deferred at the
implementer's request because the user requested a visual revision while this
pass was finishing. This is therefore natural-play evidence, not a claim of
pixel-identical replay of that archived build.

Setup: `setup.py RUN_DIRECTORY` supplied only documented shell limits of a
20-minute session and 120-minute daily allowance. Profile creation remained
visible: A, first face, age 9, skip PIN. Entry used launcher page three and
Kingdom > New Adventure. The run recorded 260 native screenshots, five process
restarts, and approximately 51.5 minutes of ordinary emulator time excluding
two ten-minute rest breaks. Reading usually advanced 15 seconds per page.
Both automatic shell breaks were allowed to finish using ordinary elapsed time
on the resting screen; no wall-clock jump was used.

The natural route was:

1. Ask the village, hear Pippa, and record both safe woodland paths.
2. Inspect the bridge with Lucas and plan a safe repair.
3. Check the mill wheel, copy the corrected accounts, seek substitute supplies,
   and help Nell carry them.
4. Take the marked hillside detour around the wet low footpath, carry the tower
   map, hear Fen, and promise to help with the nests.
5. Record the channel's full cost. At council, choose **Share by a schedule**
   and confirm the displayed decision.
6. Return and carry tools around the bank while the bridge crew repairs and
   inspects the crossing. Correct the bridge rumor in Nell's delivery notices.
   Teach the slower pantry recipe at the mill.
7. Visit the marsh, review the failed handovers, and **Train joint watchers**.
   Read *One deed, still work* and *A useful disagreement*, then keep exploring.
8. Save & quit from the Day 15 / 1 good deed map. Restart, use the visible
   picker/launcher/Continue flow, and verify the same completed map.

The shared route made its costs understandable: neither side got all the water,
the first timetable failed between handovers, and the solution required named
people and paid time to perform checks. Returning after the council led to
substantial work and a distinct ending. Unavailable choices explained their
requirements and offered reachable alternatives. The wet route had a safe
detour; missing rope and thyme each had a slower work alternative.

Save/resume checks passed mid-scene (*A repair that held*, Page 2/6), on the
world map, after a shell rest interrupted a requirement explanation, after a
second rest interrupted reading, and after the completed ending. Back,
Reread, Party, Items, Journal, travel preview cancellation, and re-entry were
exercised. The journal correctly showed the repaired bridge and missing
supplies as Complete. No crash, progression dead end, or duplicated deed was
observed. This pass covers an emulator, not physical hardware; other council
routes and every optional encounter were outside this tester's scope.

Two findings were reported and fixed:

- **Unreadable menu pagination:** preliminary Party, Items, and Journal entry
  counters collided with page counters. The corrected shared footer was
  inspected on the full run; see `009-party-fixed.png` and
  `053-journal-fixed.png`.
- **Blank quiet-visit caption:** after completing local activities, a travel
  card showed a blank activity title. Repro: shared council plan, bridge work,
  village rumor correction, then another village preview. See
  `197-village-review-card.png`. The fix was inspected after restart on a
  cleared mill preview: `214-mill-review-card.png` displays **A quiet visit**;
  Back returned to the map normally.

The exact input audit trail and selected screenshots are in `pre-visual/`.
All 260 source frames remain in
`build/kingdom-independent-final/`. Selected evidence:

- `065-inspection2-before-save.png` / `067-session2-resumed-inspection2.png`:
  same mid-scene page across restart.
- `159-council-choices1.png` / `160-sharing-feedback1.png`: readable council
  alternatives and explicit decision review.
- `220-sharing-work1.png` / `235-sharing-revision-choice.png`: consequences
  and the second meaningful choice.
- `258-postending-map.png` / `260-postending-resumed-map.png`: completed
  persistent world before and after restart.

## Preliminary development build

This exploratory pass preceded the final manuscript/build freeze and does not
certify the revised Chapter One presentation. A new profile was created entirely through the
visible UI (name A, first face, age 9, skip PIN). The tester navigated to launcher
page three, selected Kingdom, and opened New Adventure.

Covered: opening story pages and Back, Pack, Party, Items, Journal, Save & quit,
process restart, profile picker, launcher, and Continue. Save & quit returned to
the launcher; restart/Continue restored the same opening page. No crashes or
dead ends occurred in these flows.

One actionable finding was reported: Party, Items, and Journal drew an entry
counter on the pagination baseline, making both counters difficult to read.
See `preliminary/013-party.png`, `019-items.png`, and `020-journal.png`. The
implementer reported a shared footer fix; the correction was subsequently inspected in the archived full run above.
The preliminary tester reported legible title artwork, story text, touch targets,
and round-screen layout in those early frames. That observation does not
verify the later replacement presentation.

The exact exploratory input trace and representative screenshots are in
`preliminary/`. The trace contains process restarts and is an audit trail, not
a claimed replay-certified scenario.
