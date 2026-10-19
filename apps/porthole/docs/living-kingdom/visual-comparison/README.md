# Board08 reference and final emulator comparison

These comparisons use the untouched selected
[08 — Chapter One: Polished UX](../../../../../docs/superpowers/specs/living-kingdom/concept-art/08-chapter-one-polished-ux.png)
and actual screenshots from the frozen emulator executable:

`6f40297c36fccf7d067c7f37aeeeacf2f29f0fa6416d5cca19832efcf33e87ce`

The later four-string copy correction has emulator SHA-256
`780fd0add1892afa4af35e29a9d1acc266d5baf7798e6bcb0ab3a8c2bd4eb246`.
A full fresh mill replay on that binary matches all 218 watched frames,
including every comparison capture below. The artwork, typography and controls
are unchanged; see the [copy validation](../blackbox/copy-woods/README.md).

The screenshots came from the ordinary-input session
`build/kingdom-final-mill-board08`. Files in [native/](native/) are byte-for-byte
copies of its 480×480 captures. No game screens were regenerated or retouched
for this comparison. [manifest.json](manifest.json) records source/capture
hashes and the exact original-board crop rectangles.

## Comparison artifacts

| Artifact | What it compares |
| --- | --- |
| [Title, party and inventory](board08-vs-final-overview.png) | Original board panels beside actual native screens |
| [First page, continuation and choices](board08-vs-final-reading.png) | Illustration use, paragraph space, typography and live controls |
| [World map](board08-vs-final-map.png) | Original labelled map beside the GPT-cleaned terrain and six live destinations |
| [Journal](board08-vs-final-journal.png) | Original journal example beside the actual campaign overview |
| [Native 480px and 201px reading preview](reading-native480-and-201px.png) | Exact first/continuation screenshots beside a smaller size approximation |

Each comparison labels the original reference and actual emulator columns.
The original roughly 300px board panels are enlarged with nearest-neighbor
sampling for side-by-side viewing; the emulator column remains exactly 480px.
The board and game sometimes show different story content or campaign states:
these are visual comparisons, not claims of identical story screens.

Direct screenshots:
[title](native/08-title.png), [party](native/10-party.png),
[inventory](native/11-items.png), [first page](native/20-road-first.png),
[continuation](native/21-road-page2.png), [choices](native/29-road-choice.png),
[journal](native/13-journal.png), [map](native/74-ledger-after.png).

## Visual review

The title visibly reuses the original wordmark, forest, castle and party.
Princess, Lucas and Dribble retain their distinct original expressions and
costumes; the final portrait crops contain no white wedges from the board's
circular frame. The first-page village is the original illustration crop.
Navy backgrounds, textured warm parchment, gold actions and lavender secondary
controls retain the selected board's color direction.

The runtime layout is deliberately different in several visible ways. Its
larger footer controls replace the board's small baked controls. The title
cuts off lower character bodies to make room for Begin and Exit. Party
portraits occupy less of the screen than in the enlarged reference. Live map
buttons cover more terrain than the original signs. Later reading pages omit
the illustration so paragraphs have more room. The journal capture is a
campaign overview rather than the board's sample diary entry.

Body text uses the exact PR #80 **Atkinson Hyperlegible Regular** font, following
the user's readability preference, with decorative serif headings. The
reviewed native first and continuation pages have clear letter shapes,
separated paragraphs and strong dark-on-parchment contrast. No text or control
clipping is visible in these captures. These judgments describe the inspected
screens; they do not establish child reading performance or physical display
quality. [Font provenance and metrics](../../../games/living-kingdom/art/fonts/README.md)
record the deliberate typography change.

The Old Watchtower illustration is noticeably softer than village, bridge and
map artwork. Review of the independent final session's `046-tower-preview.png`
and `047-tower1.png` confirms the source constraint: the unobscured original
[ruins crop](../../../games/living-kingdom/art/board08/tower.png) is only 240×63.
Fitting the 2:1 viewport uses roughly its central 126×63 region, resized to
240×120 and enlarged exactly 2×. That is about 3.8× enlargement of the original
detail, versus roughly 2× for the village. The result is a softer close-up of
masonry and steps. This is a documented original-source resolution limitation,
not a decoder defect; no runtime assets were changed during this review.

The inventory screenshot shows a new campaign with six empty slots. It
validates the slot layout and labels. A later watched optional gathering pass
also shows [earned thyme in the grid](../blackbox/copy-woods/a/14-earned-thyme-inventory.png)
on the current binary, covering a collected item icon. Missing scenes, consequence artwork and map label removal use GPT Image
Gen with the selected board/crops as references. Exact source provenance and
runtime transformations are documented in the
[art source README](../../../games/living-kingdom/art/board08/README.md).

## Reading-size limitation

The standalone [first-page](reading-first-201px.png) and
[continuation](reading-continuation-201px.png) reading previews are exactly
**201 pixels in diameter**. At 96 pixels
per inch and 100% display scale, that is approximately **2.09 inches**. Monitor
density, browser zoom and image scaling change their apparent physical size.
The downsample also differs from a real 480px panel: it cannot reproduce the
device's pixel density, brightness, reflections, viewing distance or touch
experience. This is a monitor-size approximation, **not a physical-device
test**. Native screenshots remain available for inspecting the actual glyphs.

## Reproduction and historical material

[compose.py](compose.py) only crops the original board and lays out copied
screenshots with documentation labels. Run it with Python/Pillow from any
working directory; it does not launch the game or change artwork/runtime data.
It uses the installed DejaVu Sans font for documentation labels only.

The eight images in [before/](before/) archive the rejected initial
implementation. The older SVG-era comparisons are under
[before/svg-era/](before/svg-era/). Neither group is current acceptance evidence;
only the final files linked above describe the frozen board08 implementation.
