# Pets Club "Ukrainian" puzzles: independent playtest 1 (source-free first play)

Emulator roleplay by an independent playtester agent. This is not a study with a real child and not a test on the physical device.

## Build and setup

- Revision: `38779aa` (`git -C /home/user/porthole rev-parse --short HEAD`)
- Executable: `apps/porthole/build/host/snap`, sha256 `c9dddf15b561bf37a8595626f5ea64d7bb115ff624b69f04a77110b4a6f8fb55` (`make snap`: nothing to rebuild; same hash after `make playtest`)
- Fixture (disclosed): every script starts with `newgame Mila Pup` (a kid profile with an adopted pup, straight into Pets Club), then `wait 2500` to reach the pet's home. One script uses `reboot` (a power cycle that keeps saves) to check process restart.
- Inputs: only `tap`, `hold`, `down/move/up`, `wait` and `snap`. No `dbg`, `debug`, `ui`, `app`, `skip` or hidden-state command was used to choose actions.
- Source exposure: none of the game's source, tests, scenarios, design docs or git history was read. I read the header comment of `host/sim.cpp` (and its first ~60 lines by accident, which hold only includes and the save-file stub). After first play I ran `make playtest` and read only its report summary; that run happened after every decision recorded below.
- Exact input scripts: `playtest-1/traces/sNN.txt` (each one stands alone and replays from scratch: `./build/host/snap --script FILE`). Screenshots are native 480x480 frames converted to PNG.

## Persona and brief (summary of the dispatch)

A 7-year-old from a bilingual Ukrainian-English family living abroad. They speak some Ukrainian at home and are learning to read it: Cyrillic slowly, syllable by syllable. English reading is a little better, but they skip long sentences. Impatient with text and waiting. Taps fast and imprecisely and tries a second answer straight away after a wrong one. Knows the pet game (it's their pup) but has never seen the "Ukrainian" puzzles.

Goal: from the pet's home, find the puzzles, play the "Ukrainian" words puzzle for at least two full rounds, then go back to the pet. Coverage: every question kind; deliberate wrong answers and second wrong answers; right answers; the end of a round; imprecise fingers (taps just off the edges, double taps, a tap right after a wrong answer, a tap during the right-answer moment, a long hold, dragging off before release); and leaving mid-question or right after the last answer, then coming back.

## Paths played

1. Finding the game (s01-s05). From home the child guessed the **book** (storybook, s02) and the **paw** (tricks, s03) before trying the **ball**. The ball opens a menu: Fetch! / Word Fetch / **Puzzles** / Little reads (s04). Puzzles shows "0 stars" and Plus and minus / Times / Words / **Ukrainian** (s05).
2. Round 1, as a wrong-first, impatient child (s06-s14). Five questions, three kinds:
   - **Kind A, English to Ukrainian, 3 choices** ("Which word means / mountain": нора / гора / кора). The child thought: "mountain, that's гора", but tapped нора first on purpose, then кора.
   - **Kind B, build the word from syllable tiles** ("Build the word for / your": empty slots, tiles лька / ва / ша). The child thought: "tap the pieces to make the word". Tapped ша first (right syllable, wrong order), then the distractor лька, then ва, ша.
   - **Kind C, Ukrainian to English, 3 choices** ("What does it mean? / два": two / side / smoke). Tapped smoke, then two 150 ms later.
   - Q4 "tooth" (дуб / суп / зуб, kind A with no prompt line): суп wrong, then зуб. Q5 "mom" (ма / ва / ма, kind B with no prompt): ва wrong, then ма ма.
   - End: five orange dots and a green ▶ button, with nothing else (s13_g6). ▶ starts round 2 at once.
3. Round 2, all right on the first try (s15-s22): сир→cheese, water→вода, winter→зи+ма, сова→owl, son→син. End: five green dots and **9 stars** (s22_n4). Home goes to the Puzzles menu, which now shows "9 stars" (s23_x1).
4. A mixed round (s25-s33): mountain right, dog (со+ба+ка, 4 tiles, 3 slots) with five mistakes, вода→water, nose→ніс, eight→ві+сім. 4 of 5 on the first try ended with **2 stars** (s33_mx6).
5. Back to the pet (s25 z4): home on a question → Puzzles → Play menu → the pet's home, three taps.
6. Interruptions: home mid-round and re-enter (s25); home 250 ms after the last right answer (s24); power cycle after a round (s28-s30).

## What each question kind asks, and how a wrong answer feels

| Kind | What the child thought it asked | Wrong answer | Second wrong | Right answer |
| --- | --- | --- | --- | --- |
| A: English word → pick the Ukrainian word | "Find гора" | Pink flash; the button turns maroon with a red ✕; the round's dot turns orange (s07). Obvious and gentle; you can tap again at once. | Only the right button is left un-crossed (s07_w2c). Not an outright reveal, but it gives the answer away by elimination. | Mint flash, green button with ✓, sparkles, the other choices turn grey; the next question comes about 1 s later without a tap (s08_c1a → c1c). |
| B: build the word from syllables | "Tap the bits in order" | A distractor flashes pink and stays maroon (no ✕). A right syllable in the wrong order flashes pink and goes back to normal (s09_b1, b2). | Any number of tries; distractors stay marked, no hint and no reveal (s26_v1: five mistakes on "dog"). | The slots fill, mint flash, then the next question about 1 s later (s10_d2 → d4). |
| C: Ukrainian word → pick the English meaning | "What is два?" | As in kind A (s11_e1). | As in kind A. | As in kind A. |

## Gesture and interruption matrix

| Probe | Result | Evidence |
| --- | --- | --- |
| Tap just outside an answer's left/right edge (34,113), (127,137) | Ignored | s19 o1, o2 |
| Tap in the gap between two answers (80,101) | Ignored | s19 o3 |
| Tap at the bottom-left corner of the lowest answer, near the bezel (42,143) | Counts as that answer (inside its padding) | s36 be1 |
| Tap again 150 ms after a wrong answer | Registers normally | s11 e1 |
| Re-tap an already-crossed answer | Nothing happens | s08 r1 |
| Taps during the right-answer moment (~1 s) | Swallowed; they don't carry into the next question | s21 (tap at +300 ms ignored), s11 e3 |
| Double tap on the last right syllable | Acts once | s27 u1 |
| Double tap on the right answer during the celebration | Swallowed | s11 e3 |
| Hold too long (1 s) on a tile | Counts as a tap on release (here a wrong one) | s17 m3 |
| Hold 2.3 s on an answer, release | Counts on release; no pressed look while held | s34 ct1-ct3 |
| Drag off a tile before release | Cancelled, nothing happens | s17 m1, m2 |
| Drag from one answer onto another, release | Cancelled, nothing chosen | s35 dr1 |
| Tap a filled slot (to undo) | Nothing happens (none needed: only right syllables in order are placed) | s17 m5 |
| Home mid-question, come back | Back to Puzzles; the round restarts with new questions; the answered question gave nothing (0 stars) | s25 z2, z3 |
| Home 250 ms after the last right answer | Lands on Puzzles; the stars were still added (9) but the child never saw the star screen | s24 y1 |
| ▶ on the end screen | The next round starts at once | s14 h1 |
| Power cycle after a round | Profile picker → launcher → Pets Club → Puzzles still shows 9 stars | s28-s30 rb1-rb3 |
| Power cycle mid-question | **Gap** (not tested) | |
| Letters ї, є, ґ in tiles/answers | **Gap** (none came up; і rendered fine) | |
| Evening/night tint on these screens | **Gap** | |
| A wrong answer on the very first question of round 2 (no prompt) | **Gap** | |
| Physical device, real child | Out of scope (emulator roleplay only) | |

## Findings (prioritized)

### Major

**M1. The end of a round doesn't say what the child got. A round with every question eventually right can end on a blank screen.**
- Screen: Ukrainian round end.
- Concern: feedback / comprehension / meaningful consequence.
- What happened: the persona's natural play (try one, then try again straight away) earned an orange dot on every question. The round then ended on five orange dots, a puff of confetti and a ▶ button, with no stars, words or picture (s13_g6). The child's reading: "Did it break? Did I get nothing?" A perfect round gave 9 stars (s22_n4); 4 of 5 on the first try gave only 2 (s33_mx6). Nothing on screen links the dots to the stars, so the child can't tell why one more green dot was worth 7 more stars. It feels arbitrary and doesn't teach "answer carefully the first time". For this persona the empty screen is the most likely outcome of round 1.
- Repro: `traces/s13.txt` (all-orange round, snap g6); `traces/s22.txt` (perfect round, n4); `traces/s33.txt` (4/5, mx6).
- Screenshots: `playtest-1/s13_g6.png`, `playtest-1/s22_n4.png`, `playtest-1/s33_mx6.png`.
- Suggestion for the owner: never show an empty result. Show at least one gentle reward or the dog cheering for finishing, show stars per green dot (or explain them visually), and keep the star curve readable.

### Minor

**m1. The Ukrainian puzzles are hard to find from the pet's home.**
- Screen: Pets Club home → Play menu.
- Concern: comprehension / discoverability.
- The child looking for "puzzles" tried the book (stories) and the paw (tricks) first. Puzzles sits third inside the **ball** menu, which reads as "play with the ball" (s04_play). The Ukrainian button is fourth on the next screen, near the bottom bezel (s05_puz).
- Repro: `traces/s02.txt`, `s03.txt`, `s04.txt`, `s05.txt`. Screenshots: `playtest-1/s04_play.png`, `playtest-1/s05_puz.png`.

**m2. The instruction line appears only the first time a kind shows up, so round 2 opens with no instruction at all.**
- Screen: kinds A/B/C after their first appearance.
- Concern: comprehension.
- Round 1's Q4 is just "tooth" over three Ukrainian words, and Q5 just "mom" over empty slots (s12_f3). Round 2 opens on a bare "сир" with cheese / bread / rain (s14_h1). A child coming back after a break, or a sibling taking over, has to work out the kind from the layout. Kind A and kind C look alike (a word over three buttons); only the font and script tell them apart. Dropping text suits an impatient reader, but a tiny pictogram or icon per kind would keep the cue without the reading.
- Repro: `traces/s12.txt` (f3), `traces/s14.txt` (h1). Screenshots: `playtest-1/s12_f3.png`, `playtest-1/s14_h1.png`.

**m3. Guessing is free and fast, so a child can finish by mashing without reading.**
- Screen: kinds A/C.
- Concern: challenge / meaningful choice / pacing.
- A wrong tap never locks anything, and the next tap counts 150 ms later (s11_e1). With three choices, tapping top to bottom always finishes the question in three taps or fewer, and the second wrong answer leaves only the answer (s07_w2c). The only cost is a lost green dot, which this child doesn't connect to anything (see M1). In kind B, mashing tiles also gets through, just more slowly. The persona tries a second answer straight away, so this is how they will play once the words get hard. A short pause or a gentle "look again" beat after a wrong answer, or the dog showing a hint picture, would make reading the better strategy.
- Repro: `traces/s07.txt`, `traces/s11.txt`. Screenshots: `playtest-1/s07_w2c.png`.

**m4. An ambiguous English prompt: "your" → ваша.**
- Screen: kind B, "Build the word for your".
- Concern: comprehension / content.
- A Ukrainian-speaking 7-year-old would first think твій / твоя (what a parent says to them), or ваш. "ваша" is only one of several right translations, and the tiles force it. Other words seen (mountain, dog, two, tooth, mom, cheese, water, winter, owl, son, nose, eight, children) were clear.
- Repro: `traces/s08.txt` (c1c). Screenshot: `playtest-1/s08_c1c.png`.

**m5. Leaving mid-round throws the round away; leaving right after the last answer skips the reward screen.**
- Screen: any question → home.
- Concern: recovery / feedback.
- Home on Q2 after a first-try right Q1, then Ukrainian again: a brand-new round with different words ("children"), and nothing kept from Q1 (Puzzles still says 0 stars) (s25_z3). Nothing is punished, which is the house rule, but a child who stepped out for a moment loses their streak of green dots. Home within the ~1 s after the last right answer does add the stars (9), but the child lands on Puzzles and never sees them awarded (s23/s24).
- Repro: `traces/s25.txt`, `traces/s24.txt`. Screenshots: `playtest-1/s25_z3.png`, `playtest-1/s23_x1.png`.

**m6. The home button goes up one level, not home.**
- Screen: question / Puzzles / Play menu.
- Concern: consistency.
- A house icon suggests "back to my pup". From a question it takes three taps (question → Puzzles → Play menu → pet). The child pressed it once to "go home" and landed on Puzzles instead (s23_x1). This is probably the shell's usual pattern, but the icon promises more than one step.
- Repro: `traces/s25.txt` (z4). Screenshot: `playtest-1/s23_x1.png`.

### Nit

**n1. The two wrong-answer marks differ in kind B.** A distractor tile turns maroon without the ✕ that kinds A/C use. A right syllable tapped out of order flashes pink and goes back, with no hint that order matters (s09_b1). The child understood it after one try, so this is a nit.
Repro: `traces/s09.txt`. Screenshot: `playtest-1/s09_b1.png`.

**n2. The dot colours (blue = now, green = first try, orange = needed another try) are never explained.** The child worked them out by round 2.
Screenshots: `playtest-1/s08_c1a.png`, `playtest-1/s10_d4.png`.

**n3. No pressed look while a finger is held on an answer** (s34 ct1/ct2). Release works correctly.

**n4. Outside this feature: button text runs past its button edges** on "Plus and minus" (Puzzles menu, s05_puz) and "Read to me!" (storybook, s02).

**n5. The menu button says "Ukrainian" in English.** A kid learning Cyrillic might enjoy seeing "Українська" (or both).

## Where this child would stop first, and why

At the **end of round 1** (s13_g6). After five questions of "tap, wrong, tap again", the round ended on an almost empty screen: no stars, no "well done", no dog. The persona's reaction: "I got nothing", and the walk back to the puppy. I kept playing only for coverage. Round 2, played carefully, was satisfying (9 stars), and a 7-year-old would happily do one more round if the first one had paid off. The puzzle itself is short (5 questions, about 30-60 s), and that is the right length.

## Qualitative assessment

1. **Can the child recognize the goal and actions and see what changed?** Mostly yes. Big coloured buttons and slots read at a glance. "Which word means / mountain" and "Build the word for / your" were clear on the first meeting, and the dot row shows progress through the 5 questions. Every tap gets visible feedback (a pink or mint flash, ✕ or ✓), and right answers move on by themselves. Friction: after a kind's first appearance the instruction disappears (m2), and the end of a round doesn't communicate a result (M1).
2. **Interesting decisions, consequences, feedback; do pictures help, or does reading carry everything?** Choosing among нора / гора / кора, дуб / суп / зуб and ніс / ніч / ніж is good practice for a syllable-by-syllable reader: the near-identical words force reading one letter. Building ві+сім for "eight" and со+ба+ка for "dog" was the most enjoyable part, a small "aha". But reading carries the whole interaction: there are no pictures for the English or Ukrainian words. A picture of a tooth or an owl would let an English-weak child play and would reward guessing less. Challenge is fair. Consequences are weak, because guessing costs almost nothing visible (m3) and the stars are unexplained (M1).
3. **Where does it get repetitive, passive, confusing or frustrating?** The waits are short (about 1 s celebration, no waiting between rounds), so nothing felt passive. Confusing: the blank end screen, the 2-versus-9 star gap, round 2's lack of instructions, and "your" → ваша. Repetition was mild: сир showed up as an answer and then again as a distractor in the same round. Natural stopping point: the end screen. With a real reward it works as one. As it stands, the all-orange version of it is where this child quits.
4. **Can imprecise input, a wrong guess or an interruption be recovered without losing progress or adult help?** Yes for input: off-edge taps, drags, double taps, long holds and taps during the transition all behaved safely (see the matrix). Wrong guesses are gentle: you can always try again, nothing is lost and no red "fail" screen appears. Interruptions: stars survive a power cycle. Leaving mid-round silently drops that round (m5), which is gentle but a little disappointing. No adult was needed anywhere.

## Supplementary automated QA (after first play)

`make playtest` at `38779aa`: 104 of 104 scenarios PASS, 0 UI FAIL and 0 WARN, including `b4_pets_ukrainian` (41 checks). The executable hash was unchanged afterwards. This doesn't change any finding above: the audit can't judge reward clarity, prompt removal or content ambiguity.

## Iteration history and judgment

- Iteration 1 (this report): no game fixes yet.
- Brief notes for the next pass: add "play round 1 as a guesser and judge the end screen", "leave and resume mid-round with a power cycle", and "look for ї/є/ґ glyphs" (both untested here). Give the next pass a way to reach a later session without `skip`, so it can check whether prompts come back.
- Judgment: **needs another pass.** M1 should be fixed and replayed by an independent agent before this is called done. m1-m6 should be fixed or explicitly deferred with the player impact stated above.
