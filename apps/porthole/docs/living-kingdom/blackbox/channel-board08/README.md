# Independent final-art channel playthrough

**Passed:** a complete natural Chapter One channel campaign, its distinct ending,
post-ending exploration, persistent deferred bridge work, and save/resume. A
fresh process replay reproduced **242 of 242 native frames exactly**.

## Build and independence

The independently observed campaign and exact replay used SHA-256
`6f40297c36fccf7d067c7f37aeeeacf2f29f0fa6416d5cca19832efcf33e87ce`.
The tester used only rendered frames, ordinary touches, ordinary elapsed time,
and process restarts. No game implementation, manuscript source, tests, hidden
state, stage jumps, solution traces, or debug hooks were read or used. Prior
play of the shared route on the earlier presentation is documented separately;
this fresh campaign followed the visible channel preparation, costs, and
consequences.

The only setup fixture was the documented shell allowance: 20 minutes per
session and 120 minutes daily, prepared by
[the existing setup script](../independent/setup.py). The profile was created
visibly: name A, first face, age 9, skip PIN. Entry used launcher page three,
Kingdom, and Begin. No profile or story progress fixture was supplied.

The run recorded about **43.2 minutes of ordinary emulator time**, plus one
ten-minute shell rest. That break occurred naturally immediately before the
council choice. Advancing `tick 600000` while the rest screen was visible allowed
the break to finish; no wall-clock jump was used. Re-entry restored the final
council story page, before an uncommitted decision.

Mid-run, the host binary was about to receive a narrow wording correction.
The original executable was archived as `build/kingdom-board08-baseline-snap`.
The controller was then closed normally and restarted against that archive,
with an explicit `reboot` in the appended trace. Subsequent restarts and the
full replay remained pinned to the same original hash. Later wording-fix
verification is separate from this exact baseline result.

## Natural route and outcome

1. Visit the bridge first; inspect the crack with Lucas and arrange safe bank
   deliveries.
2. Visit the tower, copy the channel drawing into the journal, and visit Tilda
   to reserve the work plan. Hear the full timber, wage, upkeep, and deferred
   bridge costs.
3. Hear Elric at the mill and promise to return; hear Fen and record the marsh
   evidence without promising all the water to either side.
4. Read the council discussion, inspect both choice pages, use Reread, and read
   the full discussion again. Cancel a mill-plan review without deciding, then
   explicitly review and decide **Build the old channel**.
5. Visit Bea while the channel work proceeds. Borrowing the rope is unavailable;
   inspect its trust requirement and choose to make a clear bridge warning
   sign. Camp for one world day through the visible Rest flow.
6. Return for **The new gate's bill**. The channel has steadier water, but a
   passing storm has jammed a guide and damaged a brace; the bridge and school
   still wait for their resources.
7. Choose **Earn the wage share**: work two world days on Nell's carrying route
   and pay the apprentices. With no rope, choose **Take the longer route**:
   carry tools for two days while the crew completes the repair and Tilda
   inspects the gate. Named, funded checks remain part of the solution.
8. Read *One deed, still work*, select the available channel ending on the next
   choice page, and read all nine pages of *A gate and a work list*. Keep
   exploring. The map shows Day 14 and one deed.
9. Revisit the bridge after the ending, inspect the completed main quest and
   deferred bridge journal entry, then Save & quit from the Day 15 map.
   Restart through the visible picker/launcher/Continue flow. The completed
   map, one deed, and world state remain unchanged.

The channel route made its tradeoffs concrete. It produced a different water
arrangement, delayed the bridge, required paid work, encountered storm damage,
and required continuing gate care. The ending acknowledged those costs instead
of silently completing the bridge. Neither missing flour nor missing rope
caused a dead end: each offered an explicit, slower work alternative.

## Recovery, controls, and presentation

- **Abrupt restart:** a real SIGKILL after opening story Page 2/8 restored the
  exact same page through Continue. Compare
  [before](004-wheel2.png) and [after](006-crash-resumed-page2.png).
- **Save & quit:** restored the same promise feedback Page 2/2 mid-campaign;
  compare [before](082-promise-feedback2.png) and
  [after](084-savequit-resumed-feedback2.png). The completed map also matches
  [before](240-final-map-before-save.png) and
  [after](242-final-resumed-map.png).
- **Shell break:** [rest screen](109-council-choices1.png) recovered the
  [uncommitted council page](111-resumed-council.png).
- **Overlays:** Party overview and character details, empty Inventory grid,
  Journal entries, Menu, Return, Back, Save & quit, and Rest were inspected.
  Overlay return preserved the current story page. This route copied its
  evidence and used work alternatives, so it acquired no physical items;
  filled-inventory behavior is outside this pass.
- **Mistaps and canceled actions:** a wrong destination preview was canceled
  before Go, with no day spent. A mill decision review was canceled before
  Decide. Council Reread preserved the previous choice-page position; the
  current visible options were then used to select the channel.
- **Unavailable choice:** [Borrow the rope](168-rope-requirement.png) explained
  the trust requirement, and Back returned to the reachable sign alternative.
- **Post-ending world:** the main quest is
  [Complete](238-journal-complete.png), while the bridge remains
  [Deferred](239-journal-deferred.png). A
  [bridge revisit](236-postending-bridge-preview.png) did not reopen it or
  award another deed.

All story pages and consequence pages traversed were inspected at native
480 × 480 resolution. The final typography, parchment panels, choice cards,
portraits, and navigation were readable, with no observed clipping, overlap,
dead screens, crashes, or progression failures. The story-art first page and
full-prose later pages kept text and controls distinct. The map's labels were
legible against the illustrated landscape.

## Findings and limits

One actionable story-coherence finding was reported: the bridge-first route
had not met Pippa, but [a journal clue](147-journal-gate.png) attributed its
evidence to her. The implementer subsequently changed that note to be witness
agnostic. That wording correction does not change the baseline result above;
its verification is recorded separately with the newer build.

The [tower illustration](046-tower-preview.png) is visibly softer than other
location art. The implementer accepted this as the original-source constraint:
a 240 × 63 crop enlarged about 3.8 times, preserving the supplied artwork. It
was not a decoding defect and did not affect text or interaction.

This is emulator evidence, not physical-device testing. It covers the complete
channel route and the listed recoveries; alternate payment/rope routes, every
optional quest, inventory capacity, and all other endings are outside this
tester’s final pass.

## Reproduction

The complete ordinary-input audit trail is [input.txt](input.txt). It includes
three graceful reboots and one forced crash. Selected native frames accompany
this report; all 242 original frames remain in
`build/kingdom-independent-art-final/`.

Run from `apps/porthole/`:

```bash
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/channel-board08/input.txt \
  build/kingdom-channel-replay \
  --binary build/kingdom-board08-baseline-snap \
  --expected build/kingdom-independent-art-final
```

[replay-baseline.json](replay-baseline.json) records the executable hash and
242/242 exact comparisons. Replay uses the same 33 ms emulator protocol and
only the recorded touches, elapsed time, screenshots, reboots, and SIGKILL.

## Current copy-corrected build

The full observed trace was also replayed from a fresh directory on current
binary `780fd0add1892afa4af35e29a9d1acc266d5baf7798e6bcb0ab3a8c2bd4eb246`.
[replay-current.json](replay-current.json) records **241 identical frames and
one expected correction**, `147-journal-gate.png`.
The [current native frame](147-journal-current.png) now attributes the repair
stones to Bram's repairs without claiming a Pippa encounter. The implementer
inspected this difference; its actual journal navigation and recovery were
also watched in the [copy addendum](../marsh-board08/copy-addendum/README.md).
No conditions, effects, controls, art or other campaign frames changed.
The full paid-work ending, deferred bridge and final saved map remain
pixel-identical to this independent watched campaign.

The archived baseline executable above is a local test artifact, not a bundled
binary. For the current build, the committed selection can be compared with:

```bash
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/channel-board08/input.txt \
  build/kingdom-channel-current-replay-new \
  --expected docs/living-kingdom/blackbox/channel-board08
```

That comparison reports the intentional old `147-journal-gate.png` difference;
`147-journal-current.png` preserves its corrected pixels. All other selected
frames match. [frames-sha256.json](frames-sha256.json) records all 242 original
baseline captures. This supplements the complete baseline watched pass rather
than claiming the entire campaign was watched again after the wording edit.
