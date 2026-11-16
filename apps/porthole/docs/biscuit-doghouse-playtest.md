# Biscuit stars, Doghouse and trick puzzles: kid black-box playtest

Date: 2026-11-15. Tester: independent `general-purpose` agent (Claude), no source access, played only through the
host emulator. This is emulator roleplay of a 7-year-old early reader, not a test with a real child or on a device.

## Build

- Base `24fa211` plus the uncommitted feature branch `feat/biscuit-doghouse-tricks` (before the fixes below).
- Executable: `build/host/snap`, sha256 `68470d07f55fed054ecbaad5bb35e7c95875b0ae319fa5003e07a8a74f247232`.

## Brief (summary)

Persona: 7, start of second grade, reads short sentences slowly, impatient with text, taps a few pixels off and
double-taps. A: natural first play from a fresh profile (name the pup, find Puzzles and the Doghouse, try to buy
before affording, earn and buy the cheapest thing, finish a round that teaches a trick, press Show me), no debug
commands. B: disclosed fixture `dbg read` (every story read, 45 stars) to explore the shop. C: imprecise fingers on
the new screens.

## Findings and what changed

| # | Severity | Finding | Change |
|---|----------|---------|--------|
| 1 | Major | Earning stars is invisible: a round of 4 right ends at "0 stars" and stars appear later with no message. | The answer that earns a star says "+1 star for you and {pet}!"; the round's end says "+N stars!" or "N more right for a star." |
| 2 | Major | Show me is not rewarding: a generic line, the trick is never named. | The pup names the trick: "Watch me try Sit! Lesson 1 of 3 done." or "Watch me! I can do Sit now!" (also after Training). |
| 3 | Minor | "learned a bit of Sit" is the same after every lesson. | "{pet} learned Sit: 1 of 3." |
| 6 | Minor | A slow double-tap (460 ms+) on the last Next lands on Show me or More and skips the round's end. | The round's end ignores taps for its first second; playtest 50 checks it. |
| 4 | Minor | Nothing says how to get stars; an unaffordable Buy does nothing. | Partly: the round's end now ties stars to right answers. Follow-up: a hint on the item page. |
| 5 | Minor | The balance is small grey text, absent on Home. | Follow-up. |
| 7 | Minor | Tap areas are exactly the drawn buttons; bottom buttons hug the round edge. | Follow-up (shared ui565 layout, all games). |
| 8 | Minor | Disabled Previous/Next/Buy look nearly active. | Follow-up (shared ui565 style). |
| 9-11 | Nit | "<" vs "Back"; no buy celebration; text-only shop rows. | Follow-up. |
| 12 | Context | Stories are long for this reader, so puzzles are the practical star source. | None. |

Positive: shop paging, Back keeping its page, owned items ("at home", "See it"), the furnished room, and safe buying
(three fast taps on Buy spend once; holding and dragging off cancels).

Earning rate seen: about one star per five right answers; the Ball (3) after about 3-4 rounds, all 9 things (83)
about 415 right answers.
