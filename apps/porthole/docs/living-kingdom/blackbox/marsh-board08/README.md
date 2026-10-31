# Marsh route: final board08 visual build

This watched campaign records the board08 visual baseline. Four subsequent
copy corrections are covered by the [current-build addendum](copy-addendum/README.md),
including a fresh full-trace replay and watched validation of changed screens.
The baseline screenshots and SHA below remain unchanged historical evidence.

Completed ordinary-input emulator campaign on binary SHA256
`6f40297c36fccf7d067c7f37aeeeacf2f29f0fa6416d5cca19832efcf33e87ce`.
This evidence uses the actual original board08 artwork, the completed location
art, parchment/gold/violet interface, and Atkinson reading font. It supersedes
the archived pre-visual runs for the screens traversed here.

The tester previously authored the chapter and contributed design guidance.
This is an implementer playthrough, **not a source-free independent review**.
During this campaign, choices came from inspected visible pages and controls;
no hidden-state queries, debug commands, source-derived action scripts, staged
story saves, or clock jumps were used. This was the host emulator, not hardware
or a child usability study.

## Setup and reproducibility

The fresh run directory was `build/kingdom-marsh-final-board08`. Its only fixture
was the existing shell allowance of 20 minutes per session and 120 minutes per
day. Profile A (age 9, first face, no PIN) was created through the visible UI;
Kingdom was opened through the third launcher page. Ordinary elapsed-time ticks
allowed reading and two genuine ten-minute shell breaks. Save-and-quit and
process restarts exercised persistence.

[input.txt](input.txt) contains every observed touch, elapsed-time step,
snapshot, and restart. The watched run produced 269 native 480×480 captures.
Thirty-nine key captures are preserved here; [frames-sha256.json](frames-sha256.json)
records all 269 original PNG hashes. Full local captures remain in the run
directory. Snapshot names describe the intended next screen; where an extra
result page or shell break intervened, the actual pixels are authoritative.

A second, fresh process replayed the exact trace with the same sole shell
fixture. **All 269 captures matched pixel for pixel; zero changed frames.**
See [replay.json](replay.json). Reproduce from `apps/porthole` with a fresh output
directory:

```sh
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/marsh-board08/input.txt \
  build/kingdom-marsh-board08-replay-new \
  --expected docs/living-kingdom/blackbox/marsh-board08
```

The checked-in subset permits 39 direct image comparisons; the trace still
regenerates every capture. The recorded full-run replay used all 269 originals
as its expected set.

## Played path and results

- Asked the village, read the supply ledger, helped Nell carry supplies, and
  received a flour parcel. Inspected a locked trust option and its explanation.
- Heard Elric, promised to return, then heard Fen and promised nest help.
  Inspected the bridge safely with Lucas and recorded the repair plan.
- Read every council page, selected **Keep the marsh flow**, canceled its
  confirmation, selected it again, and committed with Decide. Camped twice;
  returned to the village for rumor correction and Pippa's account.
- Returned to the mill and encountered **A quiet oven**, the delayed cost of
  protecting marsh flow. Read **The carrying agreement**, exchanged the flour,
  and spent a day sorting and delivering relief. The parcel disappeared from
  the inventory. Both the item-free alternative and an earned ingenuity option
  were visible.
- Read **One deed, still work** and the complete marsh ending, **The longer
  lunch table**. Returned to the map on Day 10 with one deed. The chapter was
  Complete while the optional bridge quest remained Active.
- Continued exploring after the ending, repaired the bridge by carrying tools
  around the bank, and finished on Day 12 with the same one deed. The bridge
  journal became Complete and its actual panorama changed to a repaired span.
  A final quit/restart preserved Day 12, one deed, and the repaired bridge.

## Persistence, navigation, and visual evidence

| Check | Native evidence |
| --- | --- |
| Original title and readable first story page | [008](008-title.png), [009](009-road1.png) |
| Overlays, save/quit, restart preserve exact page 4/8 | [012 before](012-road4.png), [023 after](023-resume-overlay.png) |
| Reread, next/back, and return to uncommitted choices | [028](028-road-choice.png), [032](032-returned-choice.png) |
| Locked option explains its requirement | [065](065-trader-why.png) |
| Mill wheel and marsh nests visible above the paper | [072](072-mill-preview.png), [085](085-marsh-preview.png) |
| First shell break preserves bridge page 5/8 | [113 before](113-crack5.png), [114 break](114-crack6.png), [118 after](118-resumed-crack.png) |
| Principal choice confirmation and safe cancellation | [137](137-marsh-confirm.png), [138](138-confirm-cancel.png) |
| Flour description, later consumed by real relief work | [143](143-flour-actual.png), [209](209-carrying-choice.png), [212](212-spent-flour.png) |
| Mid-result save/restart restores exact page 2/2 | [211 before](211-exchange-result2.png), [214 after](214-relief-result-resume.png) |
| Second shell break preserves ending page 6/8 | [220 before](220-ending6.png), [221 break](221-ending7.png), [225 after](225-ending6-resume.png) |
| Full route ending and retained unfinished work | [231](231-marsh-ending1.png), [237](237-lunch7.png), [244](244-postending-map-real.png), [245](245-complete-journal.png), [246](246-unfinished-bridge.png) |
| Three companions, six skill pips, readable details | [259](259-post-party.png), [260](260-serena-detail.png), [261](261-lucas-detail.png) |
| Post-ending repair and retained world after restart | [256](256-tools-result1.png), [263](263-repaired-journal.png), [266 before](266-repaired-preview.png), [268 map](268-persisted-day12.png), [269 after](269-persisted-repair.png) |

The parchment texture remains quiet behind the dark body text. Gold actions,
violet secondary controls, and the navy surrounding field give the current
game a coherent relationship to board08. Location panoramas show the relevant
wheel, nests, and broken/repaired bridge within their visible bands. Reading,
choice, confirmation, party, inventory, journal, travel, camp, and ending screens
were usable through visible controls; no confirmed clipping, dead end, or save
loss was found. An apparent missing word prefix in one tool image preview was
reinspected by the root reviewer in the actual native PNG and was not a defect.

One minor navigation issue remains: the council result says Mira asks the party
to return after two days ([140](140-marsh-result2.png)), and the river journal
does not name a destination ([186](186-journal-river.png)). The delayed relief
scene is offered at Miller's Mill ([190](190-mill-preview-real.png)). The tester
first checked the village twice, then found the scene naturally by honoring the
promise to Elric. Naming the mill in that guidance would reduce unnecessary
search. It did not prevent completion, and no runtime/content changes were made
during this frozen campaign.

The test also included harmless mis-taps: an empty inventory pocket explained
itself, a canceled travel preview did not advance the day, and Back in the
journal navigated to the prior entry; Menu → Return restored the map. Two
queued Next taps during each shell break stayed on the resting screen. These
inputs remain in the trace rather than being edited out.
