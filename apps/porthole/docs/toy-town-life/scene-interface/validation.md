# Scene-interface validation

Validated 2026-10-27 in the Linux host environment. No physical Porthole or real
child was used. See the [build manifest](build-manifest.json) for exact emulator,
firmware, source and approved-reference fingerprints.

| Gate | Actual result |
| --- | --- |
| `ASAN_OPTIONS=detect_leaks=0 make -C apps/porthole ci REQUIRE_NODE=1` | Passed: asset/content checks, warning-fatal host builds, rule/App tests, 74/74 playtests and 64/64 coverage scenarios. Zero UI FAIL/WARN. |
| Line coverage | 98.3% (8158/8301); functions 98.9%. Existing thresholds retained. |
| `make complexity` | Passed 15/60/5, with no whitelist additions. |
| Firmware | Passed: 4,208,230/8,323,072 flash bytes (50.6%); static internal RAM 125,464/131,072 safety-budget bytes. No partition changes. |
| Art pipeline | Original layered PNGs, alpha masks, local palettes and cached conversion checked reproducibly. Shipped dolls/materials, BODY/TITLE, launcher icon and street records remain identical. |
| Persistence | Frozen real V1/V2 migration, CRC/range/historical rejection, V3 editor/position round trips, both repair orders, new-outing reset and creation-preserving reopening pass. |
| Input/visual regressions | All five School cards retain actual illustrations above complete labels; Cafe food pictures differ. Measured text, touch targets, circle, native NPC/prop pixels, cancelled input and interruption tests pass. |
| Educational/flow regressions | All 256 lunch carts, explicit quantity correction, School outcomes/repair, physical picture placement/replacement, three Park variants/revision, memory skip and profile/rest isolation pass. |
| Code/design review | Independent Astra reviews found four story/state/scene issues; all fixed and regression-tested. [Final review](review.md) accepted source and evidence with no unresolved Blocker/Major. |
| Independent observed play | [Report](blackbox/README.md) records the source-free child persona, ordinary inputs, build boundaries, natural outcomes, qualitative findings and final affected-flow retest. |

The final current host binary matches frozen artifact 8, SHA-256
`b11a83c374aeb6ac1787eb652ee7e4eec43c9f1a9dd20787600c227eddaf00d1`.
School's complete activity/edit/restart/finish was observed on artifact 7.
Artifact 8 changes only Cafe tray placement, Park evidence placement and the
Park idea-change interaction; those outings are retested on artifact 8. Source
review and supplementary native-pixel comparisons confirm School's empty,
feedback, ready and result frames are identical between 7 and 8. The report
labels the exact builds rather than relabeling older screenshots as current.

LeakSanitizer cannot inspect shutdown under this container's ptrace environment.
Only leak detection was disabled; AddressSanitizer and UBSan ran without findings.
Automated scenarios and root development probes are supplementary, not the
independent source-free playthrough. Superseded captures are explicitly marked.

Accepted lesser limitations: returning through Home/town and packing adds
repeated navigation, and wardrobe Back takes an indirect route through Home.
Those paths work and preserve progress, but could be shortened in a follow-up;
their main cost is patience between outings. Mina's practice is a short narrated
exchange, while the hands-on School activity is arranging Theo's pictures.
Park keeps the proposed search idea separate from the last physically inspected
place. Returning to that place through Look for clues surprised the playtester;
direct landmarks made recovery easy, with no progress loss or blocked search.
This is accepted as a minor communication issue. The notebook itself is small,
but a larger landmark target and short clue text support its visible graphic.
This version offers that creative activity, distinct friend outcomes and repair,
editable lunch arithmetic and spatial investigation as its playable core.
The first saved stopping point is a valid end to a session; completing three
outings for test coverage does not demonstrate voluntary sustained engagement.

Coordinator acceptance: the observed persona recognized pictured actions,
approached characters and landmarks, made a personal picture arrangement,
understood unfinished obligations, corrected a real lunch sum and revised an
incorrect search without coaching. Large cards and explicit selection feedback
supported control; creative choices, recognizable consequences and safe recovery
provided reasons to explore. These observations support this first playable
version, with the specific pacing and coverage limitations above and in the report.

Actual child engagement, reading at the physical viewing distance, touch latency
and handheld frame pacing still require device/user feedback. Agent roleplay
does not establish those results or the 12–20 minute session target.
