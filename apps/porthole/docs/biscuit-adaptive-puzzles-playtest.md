# Biscuit adaptive puzzles: black-box playtest

- **Build:** branch `feat/biscuit-adaptive-levels`, commit `c7a18a3`, `apps/porthole/build/host/snap` sha256 `e4fa08a79e242256e0031fc6e542a567edafa8a1bacbebc5d7dcdb972e2e43b7` (not rebuilt).
- **Date:** 2026-11-15. Emulator roleplay only: no real child, no physical device, no sound.
- **Source exposure:** none. No game source, `dbg`, `debug` or hidden state was used to pick answers; every tap was chosen from native 480 px frames.
- **Evidence:** screenshots and exact input traces in [`biscuit-adaptive-puzzles-playtest/`](biscuit-adaptive-puzzles-playtest/) (`age6_*`, `age8_*`, `age6_inputs.txt`, `age8_inputs.txt`). Each trace was replayed in a fresh process (`snap --fresh --script`) and reproduced the saved frames.

## Dispatch brief (as received)

> Independent child-perspective playtester. Change: in Biscuit, World → Puzzles now shows three choices instead of six: "Plus and minus", "Times" and "Words". Each picks questions at the kid's own level, and the level moves after each round of five (all five right: harder next time, and the round's end says so; two or fewer right: quietly easier). Times questions are short stories ("3 bowls, 4 biscuits in each." / "12 bones shared by 3 pups."). One answer per question out of three; wrong answers show the right one and move on.
> Personas: (1) age 6, start of first grade: reads 3-4 letter words, sounds out longer ones slowly, cannot read most sentences alone, counts on fingers, adds within 10; impatient, taps a few pixels off, double-taps, sometimes taps the wrong button first. (2) age 8, start of third grade: reads well, adds and subtracts two-digit numbers, knows the 2, 5 and 10 tables and some 3s and 4s; bored fast if too easy; likes being told they're doing well.
> Goal for each: fresh profile with matching age through the visible picker, name the pup if asked, find Puzzles, try each choice, play a few rounds of the favourite, and see whether it gets harder or easier as the persona would answer.

**Brief note:** the visible path is Biscuit home → **More** → **Puzzles**. No "World" label was seen.

## Setup and paths played

Each persona started on a fresh device (`reset`). The profile was then made through the visible screens: name keyboard, face, age button, PIN "skip". Biscuit was opened from the launcher tile.

- **Age 6 "Leo" (frog, age 6):** kept the pup name "Biscuit" (double-tapped OK). Opened Play/fetch by mistake and left it with "All done". Then More → Puzzles.
  - Rounds played: Plus and minus 4/5; Times 1/5, then Times 3/5 (both mostly guesses); Words 4/5; "Show me" (Biscuit does Sit); Plus and minus 5/5 ("Next time: harder ones!"), 4/5, then Back mid-round and re-entered.
  - Next, a bored mostly-left-button round scored 1/5. The following round was easier: addition only.
- **Age 8 "Maya" (fox, age 8):** renamed the pup "Coco" (7 backspaces, then typing).
  - Rounds played: Plus and minus 5/5, 5/5, 4/5; Times 5/5, 5/5, 4/5, 5/5, 4/5 (division stories showed in the fifth round); Words 5/5, then the start of the next Words round.

## What worked

- **Plus and minus starts at the right level for each age.**
  - Age 6 gets sums within 10 (6+2, 7+3, 1+1).
  - Age 8 gets two-digit minus one-digit (25−4, 29−8).
  - For age 6 the step up after 5/5 is gentle: subtraction within 10 appears (4−4, 8−5).
  - After a 1/5 round the next round quietly goes back to addition only (`age6_d6`, `age6_e1`, `age6_e2`).
- **"Next time: harder ones!" is clear and motivating for the 8-year-old.** In Plus and minus, round 3 does feel harder (15+80, 37−13, 78−67: `age8_q1`).
- **The Times stories read easily for the 8-year-old.** The short "N plates, M treats in each. / How many?" line is readable. Division arrives at the fourth level ("9 treats shared by 3 pups. / How many each?", `age8_x2`) and was the most engaging moment for that persona.
- **Wrong answers are kind.** "It's 7 + 3 = 10. Biscuit thinks that one was tricky." Play then moves on with one Next button.
- **Tricks are a strong payoff.** "Biscuit learned Sit: 1 of 3" leads to "Show me", and Biscuit then sits on the home screen (`age6_sm0`).
- **Imprecise input was safe.** Off-centre taps (±4 px), double-taps on an answer and on keyboard OK, and wrong-first taps caused no double answers and no skipped questions.

## Findings

| # | Sev | Finding | Repro | Screenshot |
|---|---|---|---|---|
| 1 | Major | **Times is too hard for age 6, and "easier" is not felt.** The age-6 profile gets the same first round as age 8 (3×2, 4×2, 2×5, 3×5). There are no pictures, and the story sentence uses words a first-grader cannot read ("biscuits", "each"), so the child can only guess. After 1/5 the "easier" round still asked 5×10=50 and 4×5=20. | age6 trace: More → Puzzles → Times; play a round, then another | `age6_t01`, `age6_t13`, `age6_t09b`, compare `age8_r1` |
| 2 | Major | **The result is told only in words.** A right answer and a wrong answer look the same: same layout, colours and button ("Yes! …" or "It's …"). The round end is 3–5 lines of text, and the pup never appears in Puzzles. A non-reader cannot tell how they did. | Answer any question right, then one wrong | `age6_p02`, `age6_p05`, `age6_a6` |
| 3 | Major | **For age 8, Plus and minus is too easy, and the first "harder" step is not felt.** Round 1 includes 62−1 and 22−2. After "Next time: harder ones!", round 2 is the same kind of question (75−6, 88+1, 56+4). The real step comes only in round 3. An easily-bored 8-year-old may quit in round 2. | age8 trace: Plus and minus rounds 1–3 | `age8_n5`, `age8_n6end`, `age8_o1`, `age8_o5`, `age8_q1` |
| 4 | Major (triage) | **A 5/5 Words round has no "Next time: harder ones!" line,** while 5/5 rounds in Plus and minus and Times do. The next round looks no harder ("pen_il" after "s_mmer", "but_on"). Either the line is missing, or age 8 already starts at the Words ceiling. In both cases the persona gets no "you're doing well" step. | age8: Words, 5 right | `age8_y6end`, `age8_z1` |
| 5 | Minor | **The same item repeats inside one round.** "sun" is Q1 and Q5 (age 6), "school" is Q2 and Q5 (age 8), 3×2 comes twice in a row, and 5×2 comes twice in one round. | Words round (each persona); first Times round | `age6_w01`, `age6_w05`, `age8_y5` |
| 6 | Minor | **Some Words have a second real-word answer.** p_g with "u" makes **pug**, a dog in a dog game. s_n with "o" makes "son", and bu_ with "m" makes "bum". A 6-year-old who cannot read the clue has two "right-looking" words. | age6 Words round | `age6_w04`, `age6_w04b`, `age6_w05` |
| 7 | Minor | **The menu is text only.** Most 6-year-olds cannot read "Plus and minus". Icons (+ −, ×, ABC) would let them choose. | More → Puzzles | `age6_s11` |
| 8 | Minor | **Some distractors give the answer away.** "0" is offered for 2×7 and 2×8. | age8 Times round 3 and 4 | `age8_u5` (+ trace) |
| 9 | Minor | **Star messages confuse.** Stars come for every 5 right answers in total, not per round. So the same score of 4 reads "1 more right for a star." in one round and "+1 star!" in another, and the round end repeats "+1 star!" for a star already shown mid-round. | age6 rounds 1–2; age8 Plus and minus round 3 vs Times round 3 | `age6_p09`, `age6_t06`, `age8_q6end` |
| 10 | Minor | **A round-end line breaks mid-phrase:** "Shake a paw: 1 / of 3.", "Take a bow: 1 / of 3." | age8: Times round 2 end, Words round end | `age8_r6end`, `age8_y6end` |
| 11 | Minor | **Back in mid-round restarts the round at Puzzle 1 without telling the child.** No stars are lost. | age6: 1 answer, Next, Back, Plus and minus again | `age6_c3` (+ `age6_inputs.txt`) |

## Qualitative assessment

- **Understanding.**
  - The 8-year-old understands everything without help.
  - The 6-year-old understands the Plus and minus screen ("6 + 2 = ?" and three number buttons) and Words with 3-letter words, by sounding out sun/cup. They cannot read the story lines, the result lines or the round end, so they act by looking at numbers only (findings 1, 2, 7).
- **Difficulty at the start.**
  - Age 6: Plus and minus and Words are right; Times is too hard.
  - Age 8: Plus and minus and Times start too easy for one or two rounds; Words is about right.
- **Is the level change noticeable and fair?**
  - Going easier for age 6 felt right and was not shaming.
  - Going harder: noticeable for age 8 from the third Plus and minus round and from the division level in Times. It was not noticeable right after the first promise (finding 3), and not shown at all in Words (finding 4).
- **Boring or confusing parts.** Every screen is plain text on cream, five times per round, with no animation and no pup. Repeats inside a round (finding 5).
- **Recovery.** Wrong answers, double taps and Back are all safe. There are no dead ends: More / More puzzles always returns to the menu.

## Where each persona stops

- **Age 6.**
  - First voluntary stop: in the first Times round, after guessing the 3×5 question ("too hard").
  - If they go back to Plus and minus, they would play 2–3 rounds and stop at the first "Show me" to watch Biscuit sit.
  - Without an adult reading aloud, they would not come back to Times.
- **Age 8.**
  - The risk point is the second Plus and minus round ("you said harder!").
  - They would settle in Times once division stories arrive, and stop after "Coco can do Twirl now!" to press Show me.

## Gesture / interruption matrix

| Case | Result |
|---|---|
| Off-centre tap on answer (±4 px) | Tested: correct answer registered |
| Double tap on answer / keyboard OK | Tested: acted once |
| Wrong button first | Tested: wrong answer counted, no recovery needed |
| Back mid-round, re-enter | Tested: round restarts at 1 (finding 11) |
| Long press, drag off, input during transition | **Gap**: not tested |
| Process restart / level kept across reboot | **Gap**: not tested |
| Evening/night tint, sound | **Gap**: not tested |

## Supplementary automated suite

`python3 tools/playtest.py` ran against the existing `snap`/`snap-asan` without a rebuild: **101 of 101 scenarios pass, UI 0 FAIL / 0 WARN**. That includes `50_biscuit_puzzles`. The hit regions on the puzzle screens are at the floor: answer buttons 33×22 and Back 30×22 logical px, from the `ui` dump. Text contrast is dark `#42345A` on pastel backgrounds.

## First-pass judgment (build c7a18a3)

**Needs another pass.** Four Major findings are open (1–4). Fixes should be retested on a new build by a fresh agent for the age-6 first-play claims.

## Second pass

### Build and exposure

- **Build:** `c7a18a3` plus uncommitted changes in `games/biscuit/content_puzzles.h`, `puzzles.h` and `screens_puzzles.cpp`. The coordinator rebuilt the sims; I did not.
  - `snap` sha256 `ddb6ae5b27aed8a6517e2f57a417cda5034d8760393b57aa0adc595e099deabd`
  - `snap-asan` sha256 `f82f5b331e77e7394991c0d778f1b9d6fd4e74815f64450df447666fec7ec2ca`
- **Exposure:** the coordinator's fix note described the new level structure (Times levels 0–5, start level by age, Words length bands). I still read no source and used no `dbg` or `debug`. This replay can confirm the fixes. It cannot certify first-time comprehension: I had played before and had seen the level design.
- **Brief revision:** replay the touched flows as the same two personas. That means one Times round at the start level, one Plus and minus round, one Words round, and one all-five-right round per persona to see the level go up.

### Setup

Each persona used the same profile-creation inputs as the first pass, on a fresh device (`reset`). The 6-year-old is Leo, age 6, with Biscuit. The 8-year-old is Maya, age 8, with the pup renamed Coco.

**Method.** For each round I first played a probe pass in a throwaway run to see the questions. I then replayed the round from the start with the persona's realistic answers. The question order did not change between the probe and the replay.

**Traces.** The final traces are `pass2_age6_inputs.txt` and `pass2_age8_inputs.txt`. Each was replayed in a fresh process before its screenshots were copied.

| Persona | Round | Result |
|---|---|---|
| Age 6 | Times, start level | 2/5 (guesses) |
| Age 6 | Words | 4/5 (missed h_n) |
| Age 6 | Plus and minus | **5/5** |
| Age 6 | Plus and minus, next round | first questions seen |
| Age 8 | Times, start level | **5/5** |
| Age 8 | Times, next level | first questions seen (side branch, not in the trace) |
| Age 8 | Plus and minus | **5/5** |
| Age 8 | Plus and minus, next round | first questions seen |
| Age 8 | Words | **5/5** |
| Age 8 | Words, next round | first questions seen |

### First-pass findings, re-checked

| # | Status | Evidence |
|---|---|---|
| 1 Times too hard for age 6 | **Partly fixed, still Major.** The numbers now fit: 3×4, 3×2, 2×2, 2×4, 2×3, all with products of 12 or less. But the question is still only the sentence "3 bowls, 4 biscuits in each." with nothing drawn to count, so a 6-year-old playing alone still guesses (2/5). | `pass2_age6_T61`, `pass2_age6_T6_sheet` |
| 2 Right and wrong look the same | **Fixed.** A right answer turns the whole glass pale green; a miss stays cream. This can be read without reading. New Nit: on the green screen the Next button has the same fill as the background, so only its outline shows. | `pass2_age6_T61b`, `pass2_age6_T62b` |
| 3 Age 8 Plus and minus too easy | **Fixed.** Round 1 is two-digit ± one-digit with carrying (87−9, 15+6, 84−9). After 5/5 and "Next time: harder ones!" the next round is two-digit ± two-digit (69−45, 78−67, 40+46), a clear step. Nit: trivial items still turn up (38+1). | `pass2_age8_P81`, `pass2_age8_P8E`, `pass2_age8_Q81` |
| 4 Words 5/5 with no "harder" line | **Fixed.** The 5/5 Words round says "Next time: harder ones!". The next round uses long words (elephant, dolphin, rainbow). | `pass2_age8_W8E`, `pass2_age8_X82`, `pass2_age8_X83` |
| 5 Repeats within a round | **Fixed within a round:** no repeated word or story in any second-pass round. **New Minor:** "jacket" (same clue) is the last-round Q1 and also the first question of the new, harder round, which takes the edge off "harder". | `pass2_age8_W8E`, `pass2_age8_X81` |
| 6 Wrong letter makes a real word | **Mostly fixed.** pi_ (g/s/f), c_t, _og, h_n and _ox have no real-word decoys. **Minor remains:** _umber offers "l", which makes **lumber** (age 8, Words level 2). The clue tells them apart. | `pass2_age8_W83` |
| 7 Text-only menu | Not changed (follow-up). | — |
| 8 Distractor "0" | **Fixed in every round played:** no wrong "0". A 0 appears only where it is the right answer (2−2, 8−8). | `pass2_age6_Q61`, `pass2_age6_Q63` |
| 9–11 | Not changed (follow-ups). | — |

### What the level change now feels like

- **Age 6.**
  - Plus and minus starts with sums within 10 (0+3, 9+1, 4+3).
  - After 5/5 it moves straight to subtraction within 10 (8−8, 7−4, 2−2, 4−1). That is noticeable, and still inside this child's range.
  - Words is right at 3 letters (pig, cat, dog, hen, box).
- **Age 8.**
  - Times starts on the 2s, 5s and 10s (3×5, 4×10, 3×2, 5×2, 5×5), and 5/5 leads to the 3s and 4s (4×3, 2×3, 5×3). That is fair and felt.
  - Plus and minus and Words also step up clearly after 5/5.
  - The 8-year-old now gets "Next time: harder ones!" in all three choices, which is exactly what this persona wants.

### Where each persona stops now

- **Age 6:** still stops in Times, because they can't read the story. Plus and minus and Words now hold them for 2–3 rounds, until a trick is learned and "Show me" pulls them to the pup.
- **Age 8:** no early boredom point found. They would go round Times and Plus and minus while the levels climb, and stop at a learned trick.

### Automated suite

`python3 tools/playtest.py` on the second-pass binaries, without a rebuild: **101/101 pass, UI 0 FAIL / 0 WARN**. That includes `50_biscuit_puzzles` (41 checks). The `ui` dump of the green right-answer screen shows text `#42345A` on `#E7EBD6`: dark on light, fine.

### Second-pass verdict

**Ready for age 8; age 6 needs one more fix in Times only.**

- Findings 2, 3, 4 and 8 are fixed.
- Findings 5 and 6 are fixed, apart from the new Minors above (cross-round repeat, "lumber").
- One Major remains, **#1**: Times at level 0 has the right numbers but carries the whole question in a sentence a 6-year-old can't read, with nothing on screen to count. Suggested fix: draw the groups, for example 3 bowls with 4 dots each.
- Deferred Minors 7, 9, 10 and 11 still stand as the coordinator noted.
- A fresh agent would still be needed for any first-time comprehension claim.

## After the second pass (developer notes)

- **#1 Times for the 6-year-old:** the first Times level now draws its groups as bowls of biscuits to count, in place
  of the story sentence (`tests/playtests/52_biscuit_puzzles_age6.txt`).
- **Next blends into the green screen:** Next is purple now.
- **`_umber` offers "l":** "lumber" joined the words a wrong letter must not spell.
- Left as follow-ups: a word from the last round can open the next one; a trivial item such as 38 + 1 now and then.
