# Quiet backgrounds

The user selected the [second Park mockup](approved-reference.png) because the
physical handheld display is less forgiving than a laptop. This is an approved
art-direction reference, not an emulator capture.

## Contract

Redraw Street, Home, School, Cafe and Park using broad cel-painted shapes,
restrained shading, muted background colors and quiet space behind the dolls.
Characters, useful objects and touch controls must remain the strongest visual
elements. Keep recognizable place landmarks and their existing interaction
positions. Remove tiny foliage, flower, paving, woodgrain and highlight texture.
Judge the converted artwork in actual 480×480 game frames, including movement
through the scene and pictured destination choices.

The primary change is background artwork. It changes no story rules, prices,
rewards or save fields. Foreground dolls, materials,
characters, props, fonts and launcher icon retain their existing asset records.
Keep original source artwork and reproducible conversion inputs.

Independent play also exposed a Cafe problem: tapping the large tray moved the
doll over food and quantity labels. The tray now opens its review, and Cafe
walking stays clear of it, including restored positions from earlier saves.
Mina's menu request explicitly names a sandwich or fruit plus water; cookies
remain optional extras under the existing recipe rules.

## Acceptance

Inspect native runtime frames against the approved direction, run existing
asset/rule/UI checks, full CI, complexity and firmware compilation, and obtain
independent code/design review. An independent age-eight persona must traverse
the affected places through ordinary visible controls, checking scene recognition,
character and clue visibility, walking, mistakes/recovery and exit/resume.
Record exact build, setup, inputs, screenshots, findings and limitations.

Emulator captures establish the rendered composition. They do not establish
physical display contrast, viewing-angle performance or real-child readability.

## Automated validation

The final host executable is recorded in [build-manifest.json](build-manifest.json),
along with firmware and source fingerprints. Existing character, material, prop,
font and launcher-icon records match the previous commit exactly; see
[foreground-identity.json](foreground-identity.json). Independent source review
also reproduced all five background conversions from PNG inputs using pinned
Pillow 12.3.0 and verified unchanged generated C++ after the scene arrays.

| Check | Result |
| --- | --- |
| Full app CI, `REQUIRE_NODE=1` | Passed; 74/74 automated playtests, 64/64 coverage scenarios, zero UI FAIL/WARN |
| Coverage | 98.3% lines (8159/8302), 98.9% functions (1266/1280), 80.5% branches (9854/12239) |
| Complexity | Passed existing 15/60/5 thresholds; no whitelist changes |
| Firmware | Passed; 4,208,562/8,323,072 flash bytes (50.6%) |
| Static internal RAM | 125,464/131,072 safety-budget bytes |
| Runtime artwork | 569,682 bytes, unchanged |

AddressSanitizer and UBSan ran with no findings. Leak detection was disabled
because LeakSanitizer cannot inspect shutdown in this container's ptrace setup.
Converted [background-only previews](background-previews/) are illustrations for
source review; they are separate from the observed gameplay evidence.

## Evidence boundaries

The [independent report](blackbox/README.md) preserves the source-free first play,
observed failures, fixes and subsequent affected-path retests. Its screenshots
are actual native 480×480 emulator captures. The initial School/wardrobe pass,
full Cafe retest on final3, and final4 interaction/Park pass have separate binary
hashes and input traces; they are not presented as a single untouched run.
[Supplementary School comparisons](school-equivalence.json) establish identical
native pixels for four display states across those builds. This source-aware
fixture is supporting evidence, not independent play.

The quiet scenes retain all existing foreground asset records and the same
569,682-byte runtime artwork footprint. The firmware change is 332 bytes over
the initial background-only build, from the Cafe interaction correction. Source
PNG sizes do not count toward firmware flash.

See the [independent Astra review](review.md) for findings, fixes and acceptance.

## Remaining limits

This refinement improves visual hierarchy; it does not establish physical panel
contrast or a 12–20 minute engagement duration. Destination thumbnails still
benefit from their text labels. Cafe item/add/done/tray loops can become repetitive;
the outing's explicit saved stop remains a sensible place to finish. Direct Park
notebook taps inspect the clue, requiring the labeled search action to collect it.
The pictured route is recoverable, but this adds an avoidable detour. The separate
pause/Home navigation can also feel indirect. These are retained minor usability
limits, not evidence of inaccessible actions or lost progress. Gesture and route
coverage gaps are listed in the independent report.
