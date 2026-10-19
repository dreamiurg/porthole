# Mill-first ordinary-input playthrough — archived pre-visual evidence

**Archived functional playthrough — before the visual revision.** The user
rejected the art and typography captured here. This report preserves the actual
inputs, gameplay findings, build identities, and replay results of that earlier
presentation. Its screenshots and readability judgments do not certify the
replacement based on the original nine-screen SVG, GPT-generated art, and
Atkinson typography. Revised-build visual playthroughs will be added separately.

The implementer played from a fresh device through Chapter One's mill-first
ending using rendered `snap --serve` frames and visible touch input. No debug
commands, state queries, scene jumps, campaign fixtures or state-aware bots were
used. This is emulator testing, not a physical-device test. The only setup file
sets the shell's parental allowance to 20 minutes per sitting / 120 per day.
The profile (Sam, age 9) was created through the visible keyboard and launcher.

## Build and reproduction

The archived pre-visual emulator SHA-256 is
`f5a1d7b8c4acbcd015c632ed0de9a9dcd2a49b5fdea800cd575f3e745336246d`.
The live run began on the preceding build; its only subsequent changes were
three prose corrections on scenes this route did not encounter. A fresh replay
on that build reproduced all 146 captured frames byte for byte (see
[`replay-before-caption.json`](replay-before-caption.json)). The subsequent
renderer-only quiet-visit caption fix was replayed again: **144 frames are
identical; two travel previews differ only by the new caption**. Both updated
frames were visually inspected, with no change to subsequent play or state.
The retained screenshots use the archived pre-visual binary. See [`replay-caption.json`](replay-caption.json). A final internal table-name
change lets the test runner discover Kingdom screens, without altering any
returned name or player behavior. The archived-build replay reproduces all **146/146**
post-caption frames identically: [`replay.json`](replay.json). The complete
trace is [`input.txt`](input.txt).

The following command reproduces the historical comparison only with the
recorded pre-visual build. New art, fonts, pagination, or controls can change its
coordinates and expected frames. From `apps/porthole`:

```sh
python3 docs/living-kingdom/blackbox/kingdom_replay.py \
  docs/living-kingdom/blackbox/mill-first/input.txt \
  build/kingdom-mill-reproduction \
  --expected docs/living-kingdom/blackbox/mill-first
```

The destination must be fresh. The retained screenshot selection is smaller
than the original 146-frame run; the original replay results record all 146.
The helper accepts ordinary input, elapsed time and process restart only. It
uses serve-mode's original 33 ms timing rather than script-mode's 40 ms frames.

## Sessions, decisions and results

1. Read the seven-page opening, traversed Back and Reread, returned to the same
   choices, and chose the mill. Heard Elric and Fen, made both optional return
   promises, then inspected the bridge with Lucas. Used Exit, process restart,
   profile selection, launcher and Continue at the pending bridge decision.
   The same choices returned without a second reward or changed day.
2. Read the council's eight pages. Inspected the unavailable second-channel
   explanation (missing tower drawing), canceled it, then confirmed mill first.
   Read the immediate aftermath. Rested and traveled to the marsh after the
   scheduled change. Its six-page revisit described flour improving while the
   nesting pools became shallow. Chose nursery work and the longer two-day
   carrying alternative, preserving the mill's water allocation.
3. The shell's actual 20-minute limit interrupted deed recognition on Page 2/7.
   Ignored taps during the rest screen were observed, then an ordinary
   `wait 600000` completed the ten-minute break. Entered through the picker and
   launcher; Continue restored Page 2/7 exactly. Finished the recognition,
   seven-page mill epilogue and Keep exploring. Inspected Party, Items and
   Journal, visited the changed marsh preview, then Save & quit, restarted and
   resumed the same Day 8 / one-deed map.

The reader was inspected throughout; many story pages have explicit 12–18
second reading pauses. Later verification pages were traversed more quickly.
These three saved/interrupted sessions form one natural campaign, rather than
three separate seeded endings.

## Historical screenshot review

All retained frames are native 480 × 480. During this archived pass, the tester
reported legible story, choices, explanations, portraits, inventory, quest
status and map, with reachable controls and no crash or softlock. These are
observations of the rejected presentation, not acceptance of its aesthetics or
verification of the replacement font, art, and layout.

| Evidence | What is visible |
| --- | --- |
| [Title](02-title.png), [story](03-road-1.png) | Archived landscape, font and seven-page reader |
| [Choice](10-road-choice.png), [reread](11-reread-first.png), [same options](13-reread-choice.png) | Reading returns to the pending choice |
| [Resume](46-session-resume.png) | The inspected crossing decision survives shutdown |
| [Requirement](69-channel-requirement.png), [confirmation](70-mill-confirm.png) | Missing evidence is explained; council requires explicit commitment |
| [Delayed revisit](85-return-marsh-1.png) | Mill-first changes cause actual marsh follow-through |
| [Parent rest](106-deed-3.png), [break completed](111-after-break.png), [exact page restored](112-resumed-after-rest.png) | Shell intervention preserves the reading position |
| [Epilogue](125-mill-epilogue-5.png), [chapter closure](126-mill-epilogue-6.png), [explore](128-ending-choice.png) | Completed chapter retains unfinished work and a later hook |
| [Party](132-final-party.png), [Items](139-final-items.png) | Skills and six pockets remain accessible |
| [Complete chapter](136-final-journal.png), [active bridge](137-final-journal-2.png), [available supplies](138-final-journal-3.png) | Deed does not silently finish optional quests |
| [Map](141-final-world-map.png), [changed marsh](142-restored-marsh-preview.png) | Day 8 / one deed and persistent landscape |
| [Save & quit](144-finale-save-quit.png), [Continue](145-finale-continue.png), [resumed map](146-finale-resumed-map.png) | Post-ending world survives another full process restart |

Some original capture names describe the intended next screen rather than the
actual frame: `106-deed-3` is the parent rest screen. The table above describes
its actual content. Misnamed, unused captures are preserved in the complete
trace but are not presented as evidence of a different screen.
