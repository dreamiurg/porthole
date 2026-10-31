# Independent Astra review

Reviewer: `quiet_review`, Astra; read-only, separate from implementation and the
source-free Sol child-perspective playtester. Scope: this refinement relative to
`7289286cb410b29542878382a02eacfb37b9ed14`, including art/conversion, Cafe controls,
regressions and exact-build evidence.

## Source and artifact review

- Reproduced all five background conversions with pinned Pillow 12.3.0.
  Nonbackground cache records and all 72 generated foreground arrays are identical.
  The packed runtime art footprint remains 569,682 bytes.
- Found a material Cafe clearance bypass: Home movement followed by Resume could
  return the doll over the tray. The implementation now uses the actually displayed
  scene for placement and guards navigation, restored state and rendering.
  Focused ordinary-navigation and historical-state regressions cover the fix.
- Final tray target is 50×22 logical pixels with a two-pixel gap above menu cards.
  It retains the observed tray tap and minimum target size. Full automated UI
  audit passes without weakening spacing rules.
- Source, final host, firmware and reference hashes match the build manifest.
  Coverage/size totals agree with the validation logs. School pixel comparisons
  are explicitly supplementary source-aware evidence.

## Observed evidence

Inspected native School arrival, partial and completed art display; final3
resumed Cafe tray; final4 water quantities, Park arrival, found notebook,
returned notebook and restart. These show quieter broad scenery, distinct actors,
readable labels and recognizable places. Notebook art is visible on the fountain
and visibly transfers to Nori. Evidence checksums pass.

The source-free report distinguishes spontaneous discovery from deliberately
continued error recovery, and voluntary stopping from requested coverage. It
preserves the initial failure, its retests and separate immutable build hashes.

## Acceptance

**Verdict: ship. No unresolved Blocker or Major findings in the reviewed scope.**

Review identified a motion/gesture gap in the initial report. The independent
playtester then captured final4 native mid-walk frames, used a visible action
before arrival, canceled a card drag and held/released a control without repeat.
The reviewer inspected that supplement and verified the extended checksums.

The simulated eight-year-old recognized places and actors, completed activities,
recovered from mistakes and retained progress through leave/restart. Mid-walk
frames preserve doll visibility; the Cafe occlusion and resume bypass are
resolved. Customization, picture placement and discovery supported play, while
repeated Cafe steps prompted a voluntary stopping point. This supports bounded
understanding, control and enjoyment, not a claim of sustained engagement.
School acceptance combines independent initial play with unchanged-source review
and explicitly supplementary pixel equivalence.

The remaining disclosed minor limits are small labeled destination thumbnails,
indirect Town/Home navigation, repeated Cafe item steps and the found-notebook
inspection detour. They add friction but do not block the observed paths. No
physical-device or real-child readability claim is made.
