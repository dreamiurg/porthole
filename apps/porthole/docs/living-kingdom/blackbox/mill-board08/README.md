# Final board08 mill-first campaign

The implementer played the final host emulator through ordinary visible taps,
elapsed reading time, screenshots and process restarts on 2026-10-19. This was
a watched rendered campaign, not a staged ending or engine simulation. The
implementer knows the game; this is not a blind test.

Executable SHA-256:
`6f40297c36fccf7d067c7f37aeeeacf2f29f0fa6416d5cca19832efcf33e87ce`

The only fixture was the shell's 20-minute session / 120-minute daily allowance,
using [setup.py](../independent/setup.py). The fictitious Sam/age-nine profile
and fresh campaign were created through the visible keyboard and launcher.
No campaign state, seed, items, skills or outcome was injected or inspected.

## Observed play

- Read the opening and visited Party, empty Inventory and Journal. A genuine
  abrupt process kill on opening page 2/8 restored the identical page, including
  text and controls: [before](21-road-page2.png),
  [after](22-crash-restored-page2.png).
- Reached the opening choices, reread the complete scene and returned to the
  [same options](36-reread-choices-restored.png). The first-page Exit visibly
  returned to the launcher; re-entry retained the rereading position.
- Visited the mill, inspected its wheel and copied its corrected accounts.
  Allocated flour to local baking, heard Fen at the marsh, promised to return,
  inspected the bridge with Lucas and planned a safe repair.
- Save & quit, process restart and Continue restored the identical Day 3 map:
  [before](110-bridge-after.png), [after](112-session-two-resumed.png).
- The real parent timer interrupted the council after page 2/9. Extra queued
  taps stayed on the rest screen. After an ordinary 600,000 ms break, selecting
  the profile and Continue restored that exact page:
  [before](116-council2.png), [break](117-council3.png),
  [after](124-after-break-exact-page.png).
- Read all council pages and inspected the
  [unavailable channel explanation](134-channel-requirement.png). From the
  mill confirmation, reread pages 1 → 2 → 1 and the complete council back to
  its unchanged options. Cancelled confirmation once, selected again and
  committed once: [options](139-council-options-again.png),
  [confirmation](140-mill-confirm-again.png),
  [cancel](141-council-cancel-confirm.png), [result](142-mill-committed.png).
- The camp screen correctly explained that rest requires finishing the current
  scene. After reading the meeting aftermath, rested on the map and travelled.
  The second world day triggered
  [Bread and shallow pools](163-delayed-marsh-first.png): the mill gained flour
  while Fen's nursery needed help. The marsh artwork changed with the state.
- Chose the longer two-day nursery work, preserving the mill schedule. Read
  the deed recognition and the full mill-specific **Loaves and reed roofs**
  ending. Day 8's [map shows one deed](207-final-map.png).
- The [main quest](208-final-journal.png) and
  [river follow-through](211-final-journal4.png) were complete. The
  [bridge](209-final-journal2.png) and
  [supply quest](210-final-journal3.png) remained active.
- Post-ending travel to the marsh remained available, retained its changed
  artwork and returned to the map on a quiet visit. The bridge still offered
  [unfinished repair work](214-postending-bridge-work.png). Final Save & quit,
  restart and Continue retained the identical Day 9 map and deed:
  [before](213-postending-quiet-marsh.png),
  [Continue](216-ending-continue.png), [after](217-ending-exact-resume.png).

## Findings and reproduction

No crash, lost page, duplicate deed, progression dead end, clipped text or
unsafe control was observed. Some capture names describe the anticipated next
screen rather than the actual screen; the account above identifies what the
images really show. The trace includes harmless navigation mistakes and taps
on the parent rest screen. They are retained rather than edited out.

[input.txt](input.txt) records all visible inputs, waits, captures, graceful
reboots and forced crashes. Total simulated elapsed time was 50 minutes
12.4 seconds, including the ten-minute parent break; this is not a claim of
continuous active play time. There were 218 native 480×480 captures.

A fresh replay after the watched run reproduced **218/218 frames exactly**:
[replay.json](replay.json). [frame-hashes.json](frame-hashes.json) records every
live capture; selected evidence images are committed here. From the app folder:

```bash
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/mill-board08/input.txt \
  build/mill-board08-replay --expected docs/living-kingdom/blackbox/mill-board08
```

That command checks the committed selection and regenerates all other frames.
The replay confirms the observed inputs; it does not replace watched play.
See the [original-art comparison](../../visual-comparison/README.md) for native
screens and the explicitly approximate 201px reading preview. No physical
device was played.
