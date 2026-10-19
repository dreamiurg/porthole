# Woods copy and final inventory validation

The implementer watched these narrow ordinary-input emulator passes on:

`780fd0add1892afa4af35e29a9d1acc266d5baf7798e6bcb0ab3a8c2bd4eb246`

Four final copy corrections remove unearned witness attribution and clarify a
return destination. IDs, effects, conditions, controls and artwork are unchanged.
The [marsh addendum](../marsh-board08/copy-addendum/README.md) covers the corrected
journal and mill guidance. This report covers both corrected woods outcomes.

## Fixtures and observed paths

These are disclosed progress fixtures from actual watched campaigns, permitted
for narrow presentation checks. Save files were copied without reading or
editing their contents. Runs A and B start from the completed
[mill campaign](../mill-board08/README.md), Day 9/one deed; run C starts from the
completed [marsh campaign](../marsh-board08/README.md), Day 12/one deed. No seed,
items, relationships or progression were injected. Every pass entered through
the picker, launcher and Continue.

- **A: optional gathering.** Read all eight woods pages and gathered thyme.
  The result returned to the map on Day 10 with the existing one deed. Its
  [six-slot inventory](a/14-earned-thyme-inventory.png) shows the earned thyme
  icon and readable label. An accidental tap on an empty slot safely opened
  [the empty-pocket explanation](a/15-earned-thyme-detail.png). Save & quit
  returned to the launcher.
- **B: dry low path.** Reread the known woods scene from the same original
  fixture and chose its available low footpath. The corrected
  [result page 2/2](b/04-low-path-result2.png) refers to **the careful markings**
  without claiming the party met Pippa. Back/Next preserved the result. An
  actual forced process kill, then picker/launcher/Continue, restored the
  [identical page 2/2](b/07-result-restored-after-crash.png), verified pixel for
  pixel. Onward opened the watchtower scene; Save & quit remained available.
- **C: wet low path.** The other legitimate campaign seed showed the low path
  unavailable and the hillside route available. Its three-page result explains
  the one-day detour, then says
  [**The marked alternative path**](c/12-watchtower-after-hillside.png).
  Back/Next and Save & quit/restart retained result page 2/3. A second
  re-entry retained page 3/3; Onward opened
  [the watchtower](c/15-watchtower-first.png), followed by a normal exit.

The corrected prose and all controls fit the round glass. No invented witness,
clipping, lost result page, duplicate reward or navigation failure was observed.
Snapshot names sometimes anticipate the next screen: C's `12-watchtower...`
actually shows result page 3/3; A's `15-earned-thyme-detail` shows the harmless
empty-pocket mis-tap. The images and account above identify the actual screens.
These were implementer checks, not blind tests or physical-device play.

## Fresh reproduction

Each folder includes its watched `input.txt`, native captures, their hashes,
and `complete-input.txt`. The complete trace first replays the original
legitimate campaign, restarts, then reproduces the narrow watched pass. No
save fixture must be downloaded or constructed to reproduce it.

Fresh replays match **17/17 A**, **10/10 B** and **17/17 C** narrow captures,
with zero changed frames. From the app folder, choose `a`, `b` or `c`:

```bash
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/copy-woods/b/complete-input.txt \
  build/woods-copy-replay --expected docs/living-kingdom/blackbox/copy-woods/b
```

The whole mill campaign also replays **218/218 pixel-identical frames** on the
new binary ([report](full-mill-replay.json)). The full marsh replay differs
only in its expected corrected return instruction. Those full replays verify
that these copy edits preserve the already watched campaign behavior; the
narrow live passes inspect the actual changed screens and recovery paths.
