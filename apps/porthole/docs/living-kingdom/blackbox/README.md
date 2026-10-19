# Chapter One: final emulator evidence

These are watched ordinary-input campaigns on the completed board08 visual
baseline, followed by narrow watched checks of four copy corrections. They are
not engine simulations or staged endings. The visual baseline SHA-256 is:

`6f40297c36fccf7d067c7f37aeeeacf2f29f0fa6416d5cca19832efcf33e87ce`

| Campaign | Evidence | Result |
| --- | --- | --- |
| Mill first | [Implementer report](mill-board08/README.md) | Complete ending, delayed nursery work, retained optional quests, post-ending visits; fresh replay 218/218 identical frames |
| Marsh first | [Designer report](marsh-board08/README.md) | Complete ending, flour consumed by relief work, post-ending bridge repair; fresh replay 269/269 identical frames |

The mill tester implemented the game; the marsh tester previously authored its
story. Their reports disclose that prior knowledge. A separate source-free
tester is currently playing the costly second-channel route; its final report
will be added when that watched campaign and fresh replay finish. All four
routes also have engine coverage across 128 seeds.

Each campaign starts fresh. Only the shell's parental allowance is configured
by [setup.py](independent/setup.py); profiles, story choices and progress come
from the visible interface. The traces include realistic reading waits, genuine
20-minute shell breaks followed by ordinary ten-minute waits, voluntary exits,
restarts and an abrupt process kill. Exact page/result/map recovery is verified
in the linked reports. No hidden progression commands are accepted by
[kingdom_replay.py](kingdom_replay.py). Replays check the recorded watched inputs
and do not replace the original live runs.

The current copy-corrected emulator SHA-256 is
`780fd0add1892afa4af35e29a9d1acc266d5baf7798e6bcb0ab3a8c2bd4eb246`.
The [clue/return-guidance addendum](marsh-board08/copy-addendum/README.md) and
[both woods routes](copy-woods/README.md) inspect the changed screens, backward
navigation, exits and exact save recovery. A fresh full mill replay matches all
218 baseline frames on this binary. The full marsh replay matches 268 frames;
its sole expected difference is the now-explicit mill destination, inspected in
the addendum. No conditions, effects, controls or art changed.

The revised build passes the reader/reread/confirmation loop, one-time
commitments, inventory use, companions, journal, consequences, endings and
continued exploration. No blocking UI, save or progression defect was found.
The final copy pass fixes the marsh return destination and removes witness
attribution from clues that can be earned before meeting Pippa.

## Current screens

![Original-art title](mill-board08/08-title.png)
![Illustrated story with Atkinson text](mill-board08/20-road-first.png)
![Readable continuation page](mill-board08/21-road-page2.png)
![Rereadable council choices](mill-board08/132-council-options.png)
![Original companion portraits](mill-board08/10-party.png)
![Six-slot inventory](mill-board08/11-items.png)
![Delayed marsh consequence](mill-board08/163-delayed-marsh-first.png)
![Completed chapter journal](mill-board08/208-final-journal.png)
![Persistent world and deed](mill-board08/217-ending-exact-resume.png)

[Original-versus-final comparisons](../visual-comparison/README.md) include
native 480px captures and a 201px reading-size approximation. No physical-device
playtest or child usability study is claimed. The softer watchtower crop and
intentional layout differences are documented there.

Earlier rejected presentations and their functional test history remain in
the [historical index](historical.md). They do not certify the final visuals.
See [verification](../verification.md) for automated checks and firmware limits.
