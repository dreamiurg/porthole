# Copy correction validation

Current host binary SHA256:
`780fd0add1892afa4af35e29a9d1acc266d5baf7798e6bcb0ab3a8c2bd4eb246`.
The [269-frame watched campaign](../README.md) remains evidence for the earlier
`6f40297…` visual baseline, not a claim that it was watched on this new binary.

Four presentation-only strings changed: the repair-stones clue no longer
attributes evidence to Pippa when it can be learned from Bram/Lucas; the marsh
council result now names Elric at the mill as the return destination; two woods
results refer to marked paths without assuming Pippa provided them. No IDs,
conditions, effects, controls, or art changed. The content compiler regenerated
the tables and its freshness/graph validation passed. This report covers the
clue and council changes; the root tester separately covers the woods results.

## Watched narrow pass

The fixture is legitimate prior play, not a constructed story state. The
archived baseline binary replayed the original observed campaign through
`137-marsh-confirm` and its following 15-second reading pause. All 137 captures
matched ([fixture-prefix-replay.json](fixture-prefix-replay.json)). Only the
resulting ordinary save files were copied into a fresh validation directory;
their contents were not inspected or edited. This is a disclosed progress
fixture under the playtesting procedure's allowance for reaching affected UI.

On the new binary, the tester entered through the profile picker and launcher,
continued the saved confirmation, canceled it, and opened the journal through
Menu. At this point the party had inspected the bridge with Lucas but had not
read Pippa's eyewitness scene. The clue now says:

> Bram brought stones to repair the bridge; they are not evidence of sabotage.

It fits below the readable two-line heading, with no witness invented by the
journal ([008](008-journal-next.png)). Back/Next returned to the same entry.
Menu → Return restored the uncommitted council choices. Confirmation → Reread
opened page 1/9; Next and Back traversed 2/9 and 1/9 correctly. The previously
read council was traversed again to its choices before the marsh plan was
selected and committed.

The corrected result remains two pages. Page 2 says:

> Mira asks you to meet Elric at the mill after two days, when the delivery gap
> will be clearer. The pantry covers meals while a slower supply plan takes shape.

The full text fits comfortably on the continuation card
([017](017-corrected-result2.png)). Back returned to result page 1 and Next
restored the corrected page ([019](019-result-reread2.png)). Save & quit,
process restart, profile selection, launcher, and Continue restored exactly
the same result page 2/2 ([021](021-corrected-result-resume.png)). Onward opened
the expected next story, **After the chairs leave**
([022](022-next-scene.png)). No clipping, pagination shift, unexpected commit,
or recovery defect was observed. The formerly reported destination ambiguity
is resolved in the council result; the general river journal remains brief.

The 22 native captures and [input.txt](input.txt) record this watched pass.
The tester previously authored the chapter; this is not an independent,
source-free review. No hidden-state queries or progression cheats were used.

## Fresh replay of the complete campaign

The entire original campaign trace was replayed from a fresh profile/save
directory on the new binary, with the same sole shell allowance fixture.
[full-marsh-replay.json](full-marsh-replay.json) reports **268 identical frames
and one expected changed frame**, `140-marsh-result2.png`.
The [changed native frame](full-marsh-140-corrected.png) was inspected and contains
only the clarified mill destination. All later frames, including the full
ending, post-ending repair, and final saved Day 12/one-deed state, remain
pixel-identical to the watched baseline.

[full-reproduction.txt](full-reproduction.txt) combines the legitimate fixture
prefix, a process restart, and the narrow watched input trace so this pass can
also be regenerated from a fresh directory without distributing save files:

```sh
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/marsh-board08/copy-addendum/full-reproduction.txt \
  build/kingdom-copy-validation-replay-new \
  --expected docs/living-kingdom/blackbox/marsh-board08/copy-addendum
```

The fresh narrow replay matched all **22 watched captures**, with zero changed
frames ([narrow-replay.json](narrow-replay.json)). Scope is host-emulator behavior and native rendered frames; no
physical-device or child-user validation is claimed.
