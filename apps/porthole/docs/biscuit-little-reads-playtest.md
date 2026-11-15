# Biscuit "Little reads": black-box kid playtest

Emulator roleplay by an independent, source-free agent. This is not a study with real children and not a test on
the physical board.

## Build identity

| Item | Value |
| --- | --- |
| Revision | `1067d72`, working tree dirty: `git status` lists uncommitted changes to `games/biscuit/content_reads.h`, `games/biscuit/screens_reads.cpp` and `games/biscuit/tools/check_content.py` (not read) |
| Executable | `apps/porthole/build/host/snap`, sha256 `4e6ee3a764781ba18e6a70f731a7f1fd6bd013463edbea8d7a9aea735ebd1e69` (played from a private copy with the same hash) |
| `snap-asan` (suite only) | sha256 `5d4df20bf35ee82ec2f5d922b73fb8844739d32fca671ebc3742dc82728ba0b3` |
| Driver | `snap --fresh --script`, default clock (Tuesday 16:00), native 480x480 frames inspected after every action |

## Brief (verbatim from the coordinator)

> You are an independent child-perspective playtester. [...] You did not build this feature. Do not read any game
> source (nothing under apps/porthole/games/, apps/porthole/host/*.cpp, or tests/playtests/52_biscuit_reads.txt), do
> not use `dbg` or hidden-state queries to choose actions, and do not edit game code. Play only through ordinary taps
> on rendered frames.
>
> Game: Biscuit, a virtual-pet reading game on a round 480x480 touch screen. New activity to test: "Little reads",
> the first row of the Library (Biscuit's Read area). A round is three short passages; after each passage there is
> one question with three answer buttons; then a round summary. Right answers earn stars (shared with the rest of
> Biscuit). Difficulty adapts to the kid.
>
> Persona A: age 6, start of first grade, sounds out short words slowly, knows a handful of sight words, impatient
> with long text, taps a few pixels off, double-taps after uncertain feedback. Create a fresh profile aged 6 through
> the visible shell, name the pup, find Little reads without coaching, play at least two full rounds, including
> deliberately wrong answers and Back in the middle of a passage and on the question screen.
> Persona B: age 8, start of third grade, reads chapter books, gets bored by babyish content. Fresh profile aged 8,
> play at least three rounds, aim for all right answers, and judge whether the passages and questions get harder
> and whether they stay interesting rather than trivially easy.
> Imprecise fingers on the new screens: offset taps around the answer buttons and Next, a slow double tap on the
> last Next of a round, hold and drag off a button.
>
> Judge: can the persona find the activity and understand what to do; is the question readable and the answer
> feedback clear (right and wrong); is the difficulty right for each persona (not too easy, not hopeless); do star
> rewards feel visible; text clipping, overlap, layout on the round screen; dead ends.

The brief was not revised.

**Disclosure.** Before play I read `.claude/agents/playtester.md`, `docs/playtesting.md` and the `playtest` and
`tour` skills, plus two `grep` lines from `apps/porthole/AGENTS.md` about `--serve`. No game source, scenario file or
earlier Biscuit report was read. `ui` hit-region dumps were used only after the first-play passes, for the
supplementary audit. Tap coordinates in play came from the frames (480 px / 3 = logical px).

## Setup and paths played

Both profiles were created through the visible shell from `reset`: name keyboard, face, age, "skip" on the code
pad, then the Biscuit tile on Home.

- **Persona A (Mia, 6, pup kept as "Biscuit").** Mia first opened the other dog tile (Pets Club's parcel screen),
  went Home and opened the second dog tile, which is Biscuit. On Biscuit's home she tapped **Read** (book icon), and
  the Library showed "Little reads: level 1" as the green middle row. Rounds played:
  - R1: "The hat". Tapped Previous (no effect), then wrong answer "A lid", then a fast double tap on Next (acted
    once). On passage 2 ("Box, bag, tub") she pressed **Back mid-passage**, landing on the Library.
  - R2 (re-entry starts a new round): Mud off ✓ (offset tap near the bottom-left of the button), The hat (**Back on
    the question**, which returns to the passage; then a gap tap between answers did nothing, and an edge tap was
    accepted) ✓, Up the hill ✗. Slow double tap (350 ms) on the last Next: the summary showed once and "More reads"
    did not fire. Result: **2 of 3, 0 stars**.
  - R3: hold-and-drag off Question and off an answer (both cancelled, no action). Mud off ✓, The red cap ✓, A hot
    day ✓. Result: **3 of 3, "+1 star! Next time: longer reads!"**.
  - R4 (level 2): A thin track ✗, Two buns ✗, The hat ✓ (+1 star). Result: **1 of 3**; the Library then showed level 1
    again. Back from the answer feedback leaves to the Library.
- **Persona B (Sam, 8, pup renamed "Rex").** The Library opened at level 3.
  - R1 (L3): Snow bone ✓, Three cups ✓, The garden row ✓ (an offset tap 3 px left of Next was ignored; a corner tap
    was accepted). Result: 3/3.
  - R2 (L4): Deep snow ✓, The present ✓, The orange cat ✓. Result: 3/3.
  - R3 (L5): The fridge picture ✓, The vacuum ✓, Staying on the porch ✓. Result: 3/3, 3 stars. The Library now shows
    level 5, and a 3/3 round no longer promises longer reads.
  - R4 (L5): read "One shoe" and its question, then stopped.

Traces (raw sim scripts, replayable with `snap --fresh --script`): `biscuit-little-reads-playtest/trace-A.txt`,
`trace-B.txt` and `probe-back-from-feedback.txt`. Both final traces were replayed in a fresh process and gave the
same frames.

## Qualitative assessment

**Finding it and knowing what to do.** Both personas found the activity unaided in one tap from Biscuit's home. The
Read button has an icon, and the green "Little reads" row sits in the middle of the Library. The two dog tiles in
the launcher cost Mia one detour. The flow (passage, Question, three answers, Next) is clear and consistent.
Everything on these screens is words, though: the Library rows have no icons, and Mia has to decode "Little reads"
to choose it over "The Moon Biscuit".

**Readability and feedback.** The text is large, dark on cream, and never clipped by the bezel (A, B: 02, 21, 26).
Right answers read "Yes! <answer>. Biscuit wags his tail." (06). A wrong answer reads "It's: A hat. It says: Mia has
no hat." (04). It never says the answer was wrong, and the layout, colours and Next button are the same as for a
right answer. For a slow decoder the only signal is "Yes!" versus "It's:". After a wrong answer Mia double-tapped
Next, unsure what had happened. Biscuit never appears on any reading screen. The feedback says he wags or spins,
but there is nothing to see.

**Difficulty.**
- *A (6):* Level 1 is decodable CVC text of about 25-35 words (hat, bin, lid, pen, tub). That is the right kind of
  text, but at the top of what an impatient early reader will decode three times in a row. One perfect round moved
  Mia to level 2 (digraphs, 35 words: "A thin track"). After 1/3 she dropped back to level 1, so adaptation works in
  both directions. Two questions are hard for the wrong reason. "What did the kid not have?" (03) is a negation
  question about "the kid", when the story said "Mia". "Mia is not up. What is up?" (08) contradicts the passage's
  last lines, "Mia gets up. Yes!" (07), so Mia's answer "Mia" was a fair reading and was marked wrong.
- *B (8):* Level 3 is fine but literal ("Where was the bone?"). Levels 4-5 are genuinely good for a chapter-book
  reader: inference ("What is the kid hoping for?", "What did the dog know first?", "What does the shoe show about
  the dog?") and vocabulary ("What does 'reluctant' mean here?", 24). The stories are small and warm (the fridge
  drawing, the shoe at the door) and not babyish. B never got one wrong, so the three options are on the easy side
  at L5, but not trivially so. Two of the first four passages were snow-in-the-yard stories.

**Repetition, boredom and stopping points.** Level 1 repeats quickly. "The hat" appeared in three of Mia's four
rounds, and "Mud off" opened two rounds in a row, so by round 4 the answer was memory, not reading. Mia's natural
stopping point was the end of R2 (her first complete round: three 25-35-word passages decoded with no
picture or character on screen). The "1 more right for a star" line is the only pull to continue, and she continued for
coverage. Sam would happily do two or three rounds at level 4-5. He would stop around round 4: the screens never
change and the only reward is a number in the corner. Each round summary is a clean natural stopping point.

**Stars.** A star comes from every three right answers, counted across rounds (09: "1 more right for a star"). The
answer that completes three shows "+1 star for you and Biscuit!" (11), the counter top-right goes up, and the summary
repeats "+1 star!". The reward is visible but text-only. There is no star image or animation, and the summary says
"+1 star!" even on a 1-of-3 round. The total also shows in Biscuit's More menu ("2 stars").

**Recovery and interruptions.** Back is always there. Back on the question returns to the passage, which is good
for re-reading. Back on a passage or on feedback leaves to the Library. Coming back starts a **new** round with
different passages, so the interrupted round is lost (05). Right answers already counted toward stars are kept.
Wrong answers cannot be retried.

### Gesture and interruption matrix

| Probe | Screen | Result |
| --- | --- | --- |
| Offset tap inside the answer edge (2 px) | question | accepted |
| Tap in the 2 px gap between answers | question | ignored |
| Tap 3 px outside Next | feedback | ignored |
| Corner tap on Next | feedback | accepted |
| Fast double tap (100 ms) on Next | feedback | acted once |
| Slow double tap (350 ms) on the last Next | feedback to summary | acted once; More reads did not fire |
| Hold 700 ms, drag off, release | Question button, answer button | cancelled, no action |
| Previous on page 1 or a 1/1 passage | passage | nothing happens (no hit region) |
| Previous / Next across pages | multi-page passage (L3+) | works |
| Back mid-passage, then re-enter | passage | Library; new round |
| Back on the question | question | back to the passage |
| Back on the feedback | feedback | Library |
| Leave to the launcher mid-round, process restart, night tint, audio | n/a | **not tested (gaps)** |

## Findings (prioritized)

### Major

1. **Wrong-answer feedback never says it was wrong**
   - Screen: `biscuit_read_ask` (feedback)
   - Concern: feedback / comprehension
   - "It's: A hat. It says: Mia has no hat." has the same layout, colours and button as a right answer. There is no
     "Not this one", no mark on the chosen answer, and no Biscuit reaction. A 6-year-old who decodes slowly cannot
     tell a miss from a hit.
   - Repro: `trace-A.txt` up to `snap a18` (R1: Question, then tap 100 73 "A lid")
   - Screenshot: `docs/biscuit-little-reads-playtest/04-A-wrong-feedback.png`, compared with `06-A-right-feedback.png`
2. **A level-1 question contradicts its passage, so a correct reading is marked wrong**
   - Screen: `biscuit_read_ask`, "Up the hill"
   - Concern: challenge (unfair) / comprehension
   - The passage ends "Mia gets up. Yes!", but the question asks "Mia is not up. What is up?" with Mia as a choice.
     The key says Biscuit. "What did the kid not have?" (The hat) is a negation question about "the kid" rather than
     Mia, which is also hard for a first grader for reasons that have nothing to do with reading.
   - Repro: `trace-A.txt`, R2 passage 3 (`snap a31`, `a32`), tap 80 73 "Mia", which is marked wrong (`a33`)
   - Screenshots: `07-A-passage-up-the-hill.png`, `08-A-ambiguous-question.png`, `03-A-question-negation.png`

### Minor

3. **No pictures and no Biscuit anywhere in Little reads**
   - Concern: boredom / comprehension (A)
   - Every screen is text on cream. The feedback describes Biscuit wagging or spinning, but he is not drawn. For the
     6-year-old, reading carries the whole interaction, and the voluntary stop came after one complete round.
   - Repro / screenshots: any passage or feedback screen (`02`, `06`)
4. **Level-1 pool repeats within minutes**
   - Concern: boredom
   - "The hat" appeared in R1, R2 and R4, and "Mud off" opened R2 and R3 back to back, so answers become memorised.
   - Repro: `trace-A.txt` (`a14`, `a27`, `a54`, `a22`, `a36`)
5. **Back mid-round throws the round away**
   - Concern: recovery
   - Re-entering Little reads after Back on a passage or feedback starts a fresh round with new passages. There is no
     "passage 2 of 3" indicator either, so the kid does not know how far through a round they are.
   - Repro: `trace-A.txt` to `snap a19`, tap 51 25 (Back), tap 80 84 (Little reads) gives "Mud off", not "Box, bag,
     tub"
   - Screenshot: `05-A-after-back-new-round-leading-space.png`
6. **"Previous" is drawn as a button on the first page but does nothing**
   - Concern: feedback / consistency
   - The `ui` dump shows only Back and Next/Question as regions on page 1 and on 1/1 passages. The button is tinted
     slightly pinker, which a child will not read as "disabled". The audit cannot see this because it has no region.
   - Repro: `trace-A.txt`, R1, tap 52 132 on "The hat" (the frame does not change)
   - Screenshot: `02-A-passage-the-hat.png`
7. **Pagination breaks mid-sentence and leaves one-line last pages**
   - Concern: reading flow (B)
   - Examples: "...finds a lump. It / is only a rock"; "Deep snow" page 3 is "that part alone for him."; "The present"
     and "The orange cat" end the same way. Splitting at sentence ends would cut the page count and avoid the dangling
     clause.
   - Repro: `trace-B.txt` (`b07`, `b08`, `b24`, `b29`, `b34`)
   - Screenshots: `21-B-page-break-mid-sentence.png`, `23-B-orphan-last-page.png`
8. **Questions use generic names**
   - Concern: comprehension (A) / consistency
   - The passages use the kid's and pup's own names (Mia, Rex), but the questions say "the kid" and "the dog" ("Why
     did the kid wrap it again?"). This adds a step for an early reader and loses the personal touch.
   - Screenshots: `03`, `22`
9. **The star reward is text-only**
   - Concern: feedback / reward visibility
   - The star arrives as "+1 star for you and Biscuit!" and a small counter change. There is no star graphic or
     animation, and the summary says "+1 star!" even on a 1/3 round, after a mid-round star.
   - Screenshot: `11-A-star-earned-feedback.png`

### Nit

10. **Wrapped lines sometimes start with a space**
    - Examples: " Biscuit hops in.", " Sam grumbled about it for", apparently after a sentence ending with a double
      space.
    - Screenshots: `05-A-after-back-new-round-leading-space.png`, `26-B-leading-space-level5.png`
11. **"It's:" wording**
    - "It's: A hat." reads oddly. "The answer is a hat" or "It was the hat" would be clearer.
12. **Library rows have no icons**
    - "Little reads: level 1" has to be decoded. An icon would help a 6-year-old pick it out.
    - Screenshot: `01-library-level1.png`

## Positives

- The activity is easy to find (Read, then the first green row), and B automatically starts at level 3.
- Difficulty adapts visibly: A went 1 to 2 to 1, and B went 3 to 4 to 5 ("Next time: longer reads!").
- Level 4-5 passages and inference or vocabulary questions are interesting for an 8-year-old.
- The kid's and pup's names appear in the stories.
- Layout fits the round glass with no clipping. The answer buttons are 104x22 logical px and well spaced.
- Touch handling is forgiving and safe: gap and outside taps are ignored, drag-off cancels, double taps act once,
  and the slow double tap on the last Next is absorbed.
- Back exists on every screen, and Back on the question lets the kid re-read.
- Stars are shared with Biscuit's More menu.

## Supplementary automated QA

`python3 tools/playtest.py` (no rebuild, so it used the hashes above) ran 102 of 102 scenarios: all pass, 0 UI FAIL,
0 UI WARN, including `49_biscuit_read_tour` and `52_biscuit_reads` (the latter's file was not read). Ad hoc `ui` dumps
of the library, passage, question, feedback and summary screens showed 30x22 Back and 104x22 answers, no text box
near the bezel, and contrast `42345A` on light cream or pastel. The audit cannot see finding 6 (no region), nor
findings 1-4, 7 and 8 (content and feedback).

## Limitations and judgment

Emulator roleplay only: no real children and no physical board. Audio was not observable in headless mode. Leaving
to the launcher mid-round, process restart, and the evening and night tints were not tested. B never answered wrong,
so wrong feedback and adapt-down were checked only at levels 1-2. There was a single iteration with no game fixes yet.

**Iteration 1 judgment: needs another pass.** Two Majors are open: wrong-answer feedback for non-readers, and the contradictory "Up the
hill" question (plus the negation question). Fix them and replay Persona A's flow on the new build.

## Iteration 2 (retest after fixes)

| Item | Value |
| --- | --- |
| Revision | `ae9cb14`, clean working tree |
| Executable | `build/host/snap`, sha256 `96e7a371ca89fd7efd79443f8492efd5db0bc68bf841de1058793017e7624cc4` (not rebuilt; played from a private copy with the same hash) |
| Same agent | Yes. This is the same agent as iteration 1, so it is a replay and cannot support claims about first-time comprehension. The rules were unchanged: no source, no `dbg`, taps chosen from frames |

**Coordinator brief for this pass (summary).** Persona A, a fresh age-6 profile, plays 3-4 rounds with some wrong
answers, checks that a miss is unmistakable and that passages don't repeat early, and reads "Up the hill" if it
comes up. Persona B does a quick wrong-answer check at level 3 or above. The coordinator listed the changes: the
"Oops! The answer: X." feedback; a rewritten "Up the hill"; no repeats until the level's pool of 8 has been shown;
two shorter level-5 "why" lines; the "A hot day" title is now "So hot". Findings #3, #5, #6, #7, #9, #10 and #12 were
deferred as shared reader/UI follow-ups.

**Paths played.** The profiles were created through the shell as before (`i2-trace-A.txt`, `i2-trace-B.txt`).
- **A (Mia, 6), level 1.**
  - R1: The hat ✗, Box, bag, tub ✓, Mud off ✗. Result: 1/3.
  - R2: The ham ✓, The red cap ✗, Up the hill ✓. Result: 2/3, +1 star.
  - R3: So hot ✓, Two buns ✗, Mud off ✓. Result: 2/3.
  - R4: The red cap ✗, then a fast double tap on Next (acted once), landing on "So hot".
  - The first eight passages were all different, and the first repeat ("Mud off") came as the ninth.
- **B (Sam, 8), level 3.** Snow bone ✗, Stuck stick ✗ (B skipped the passage with one tap too many, then used Back
  on the question to read it), Up the hill ✓. Result: 1/3. The Library then showed **level 1**, and the next round
  opened with "Two buns".

**Results against the iteration 1 findings**

| # | Finding | Status |
| --- | --- | --- |
| 1 | Wrong feedback never says wrong (Major) | **Resolved as a Major; a Minor remains.** "Oops! The answer: The tan cap. It says: ..." (`i2-01`) is unmistakable in words, and "Oops!" is within a first grader's reach. The miss is still words only: same colours and layout as a hit, the chosen answer is not marked, and Biscuit is not shown. Answers now wrap awkwardly ("The tan / cap.", "By the / tree."). |
| 2 | "Up the hill" contradicts its passage (Major) | **Resolved.** The passage now says "Mia sits on a log", and the question is "Is Mia on top?" with "No, on a log" as the key (`i2-02`). The right-answer feedback reads "Yes! No, on a log." (`i2-03`), which is confusing for a 6-year-old (new Nit 15). "What did the kid not have?" still stands and is tracked under #8. |
| 3 | No pictures or Biscuit | Open (deferred), Minor |
| 4 | Level-1 pool repeats | **Resolved.** 8 distinct passages in a row, then the first repeat |
| 5 | Back discards the round; no round progress | Open (deferred), Minor |
| 6 | Inert Previous on the first page | Open (deferred), Minor |
| 7 | Mid-sentence page breaks and orphan pages | Open (deferred), Minor. Not replayed at L4-5 |
| 8 | Generic "the kid" / "the dog" in questions, plus the negation question | Open (follow-up), Minor |
| 9 | Star reward is text only | Open (deferred), Minor |
| 10 | Leading space on wrapped lines | Open (deferred), Nit. Still in "Mud off" |
| 11 | "It's:" wording | Resolved: replaced by "Oops! The answer:" |
| 12 | Library rows have no icons | Open (deferred), Nit |

**New findings**

13. **Major: one bad round drops an 8-year-old from level 3 to level 1 (babyish content)**
    - Screen: Library / `biscuit_read`
    - Concern: challenge / boredom
    - After two misses in the first level-3 round, the third passage was already the level-1 "Up the hill" (`i2-04`).
      The summary then sent Sam to "Little reads: level 1" (`i2-05`), where the next round opened with CVC text ("Rex
      has a bun. Sam has a bun."). A third grader who misreads twice is pushed two levels down into first-grade text
      and needs two perfect rounds to climb back. This is exactly the babyish content persona B quits over. In
      iteration 1, persona A's drop was one level (2 to 1). This path was untested in iteration 1 (B never missed), so
      it may predate the fixes.
    - Repro: `i2-trace-B.txt`. Fresh age-8 profile, then Read, then Little reads (level 3). Answer "Under the rock",
      then "Rex", then the right answer. Back: the Library shows level 1.
    - Screenshots: `docs/biscuit-little-reads-playtest/i2-04-B-level1-passage-in-level3-round.png`,
      `i2-05-B-library-dropped-to-level1.png`
14. **Nit: answers in the "Oops!" line wrap mid-phrase**
    - Examples: "The answer: The tan / cap.", "By the / tree.", "A bun not / big." Putting the answer on its own line
      would keep it whole.
    - Screenshot: `i2-01-A-oops-feedback.png`
15. **Nit: right-answer feedback for a "No, ..." answer reads "Yes! No, on a log."**
    - Screenshot: `i2-03-A-yes-no-on-a-log.png`

**Not verified in this pass.** I did not reach level 5, so the two shortened level-5 "why" lines were not seen. A
level-1 "So hot" passage did show its new title. Pagination at L4-5, process restart and leaving to the launcher
are still gaps.

**Automated suite on this build** (`tools/playtest.py`, no rebuild): 101 of 102 pass. `49_biscuit_read_tour` and
`52_biscuit_reads` pass, and there are 0 UI FAIL and 0 WARN. The one failure is `c0_times_cards` line 22, `expect n=5`
got `n=3`, in Times Cards, which is unrelated to Biscuit.

**Judgment after iteration 2: needs another pass.** Both original Majors are resolved, but new Major #13 (adaptation
drops an 8-year-old two levels after one round) is open. Once it is fixed, replay persona B with misses at level 3 or
above, and reach level 5 to read the shortened "why" lines.
