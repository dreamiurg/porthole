# Chapter One: final emulator evidence

These are watched ordinary-input campaigns on the completed board08 visual
build, not engine simulations or staged endings. The frozen emulator SHA-256 is:

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

The revised build passes the reader/reread/confirmation loop, one-time
commitments, inventory use, companions, journal, consequences, endings and
continued exploration. No blocking UI, save or progression defect was found.
The marsh report records one minor guidance issue: its return instruction
could name Miller's Mill more explicitly. The tester found it naturally and
completed the route.

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
