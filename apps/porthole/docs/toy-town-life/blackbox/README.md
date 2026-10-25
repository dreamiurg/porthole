# Toy Town Life: observed emulator playthrough

2026-10-25, implementation based on design commit `5edfb9e`. The coordinator
personally chose ordinary visible controls in the live `tools/webemu.py` emulator,
inspected native 480x480 frames, and completed School, Cafe and Park. The server
uses the same `snap --serve` renderer and touch input as the browser canvas.
No browser-extension connection was available; this pass used the emulator's
normal touch endpoint and rendered frames. No `dbg`, `app`, hidden-state query,
source-derived solution or state-aware bot selected actions during this pass.

Setup fixture: one otherwise fresh shell profile, Sam, age 8. The fixture created
only the profile; play entered through the visible picker and launcher. All input
is preserved in [input.jsonl](input.jsonl). A fresh-process replay is preserved in
[playthrough.txt](playthrough.txt): run it from an empty directory containing
`build/host/`, using the built `snap --fresh --script <absolute trace path>`.
The trace includes the profile fixture and power-cycle. Its first Toy Town tap
was adjusted to the final centered tile after the launcher preservation fix;
that final tile was also entered live at frame 31. Every game decision matches
the observed session. Fresh replay completed successfully; all 39 native frames
were inspected, individually for major decisions and together as a contact sheet.

The final source hashes are in [build-manifest.json](build-manifest.json). Only
three frames changed in the final-source replay: the launcher before entry,
launcher after exit, and the wardrobe's corrected `peach` label. Every story,
budget, clue, consequence, memory and stopping frame was pixel-identical to its
live capture. The corrected wardrobe and launcher were also inspected live after
restarting the final executable (frames 31 and 36).

## Paths and observed outcomes

| Flow | Ordinary actions and result | Native evidence |
| --- | --- | --- |
| Entry/style | Picker → launcher → title → doll → wardrobe. Changed hair and skin, then later verified both after a power-cycle. Cycled fashion, plush, clay and keychain. | [title](02-title.png), [wardrobe](36-final-wardrobe.png), [plush](37-plush.png), [clay](38-clay.png), [keychain](39-keychain.png) |
| Plan | Chose School and packed a notebook in the three-slot bag. | [map](05-map.png), [bag](06-bag.png) |
| School | Asked Mina what worried her, planned cue cards, saw Mina begin successfully while Theo hung fewer pictures, then helped Theo finish. | [listening](08-listen.png), [dilemma](09-promises.png), [consequence](10-school-result.png), [repair](11-school-repair.png) |
| Optional memory | Opened details, changed caption and sticker, returned and kept the School memory. | [choice](12-memory-choice.png), [details](13-memory-details.png), [stop](14-stop.png) |
| Cafe recovery | Three sandwiches cost 15; the tray correctly needed 5 more coins. Revised to sandwich + water = 6, saved that idea, then added a cookie = 8. Compared `2 more coins`, ordered, and saw 2 remaining. | [menu](16-menu.png), [over budget](17-over-budget.png), [6 coins](18-six-coin-tray.png), [comparison](20-comparison.png), [lunch](21-lunch-result.png) |
| Skip | Skipped Cafe memory and reached the same easy stopping point, with School memory preserved. | [skip result](22-skipped-memory-stop.png), [scrapbook](35-scrapbook.png) |
| Park recovery | Nori last drew by the fountain. Examined the bench, tested an incorrect bench idea, revised it, and inspected the fountain's pink-ribbon clue. | [last seen](24-last-seen.png), [bench clue](26-bench-clue.png), [wrong idea](27-wrong-idea.png), [evidence](28-fountain-clue.png) |
| Interrupted session | Used Back → Save & leave at the fountain clue, terminated/restarted the emulator process, selected the same profile and entered through the launcher/title. The exact clue and pending idea returned unchanged. | [pause](29-pause.png), [exit](30-exited.png), [resume](32-resumed-clue.png) |
| Park completion | Tested the revised fountain idea, found and returned the notebook, kept a simple memory and reached the natural stop. | [found](33-found.png), [stop](34-park-stop.png) |

## Findings and resolutions

- Major visual: the original prose panel obscured most of the doll. Moved it down
  and put two decisions side by side; replayed the full three-outing flow.
- Minor visual: `Menu` wrapped into two lines and the hairstyle met the bezel.
  Replaced it with a single Back chevron and moved the doll inside the glass.
- Major autonomy: the initial memory flow required an entry. Added explicit Skip
  and a separate persisted details page; exercised both through normal input.
- Minor clarity: wardrobe labels named clothing shapes the art did not draw.
  Labels now describe actual fabric patterns, shoe colors and accessories.
- Minor arithmetic language: replaced negative comparison differences with
  `fewer`, `more` or `same cost`; observed 6→8 as `2 more coins`.
- Integration: the first nine-game layout moved existing tiles and broke a
  Starfighter visible-entry regression. Final layout preserves the first eight
  games exactly and appends Toy Town on page four. Added a shell regression.
- Hardware budget: initial firmware overflowed its slot. Lossless RLE encoding
  of Biscuit discovery pictures saved about 729 KB without changing pixels;
  original-pixel checks cover every picture and its 3x rendering.

## Supplementary verification and limits

Rule/App tests and `95`–`99` playtests separately cover all cart combinations,
all School outcomes, all Park placements, missing paper, full bags, rejected
saves, duplicate and carried touches, slow holds, canceled gestures, book rollover,
profile isolation, actual shell rest interruption/expiry, reboot and ±14 days.
Those probes are supplementary and did not guide the live player's decisions.

Validation totals and commands are recorded in [validation.md](../validation.md).
No physical board or child playtest was performed. Display viewing size, touch
latency and the 12–20 minute session target still need parent/child hardware
feedback. Native emulator frames confirm software layout, not physical usability.

V1 has fixed character poses, limited hair silhouettes, fabric patterns and shoe
colors rather than a large garment-shape catalog. Scrapbook stickers are named
markers with scene art and an outcome caption, not stored bitmap photographs.
Room placement, Mall, free typing, weather/companion variants and broader NPC
memory are deferred. The three complete authored outings remain replayable;
no extra reward or clock mechanic pressures another visit. Park landmark overlays
are deliberately simple and could benefit from richer illustrations later.
