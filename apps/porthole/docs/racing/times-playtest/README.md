# Rally "Times" page: independent black-box playtest

Emulator roleplay by an independent `playtester` agent (opus). This is not a study with real children and was not run on a physical device.

## Build identity

- Feature commit `dfae73f` ("Rally Times button shows every kid's best times without racing"). During the session HEAD moved to **`79ca717`** ("restore the brief symlink, Rally board name width"). Someone else was also editing Rally game files in the shared tree at the same time (rival work).
- The early runs used the shared `build/host/snap` (sha256 `e951c03a…`, then `128638fa…`, then `1e32446c…` as other agents rebuilt it). **They were superseded.** One of those binaries already gave different race times (1:55.8 instead of 1:55.7).
- **Final evidence:** every script was replayed against a snap built from a clean detached worktree at `79ca717`, with snap sha256 `eab188695305ca98e34f6adb42d4bcc35c8af7b321132c1132c54c0ec4c6fe67`. All screenshots in `shots/` and all `scripts/*.out` come from that binary. To reproduce, run `scripts/run_all.sh <pinned apps/porthole> <this dir>`.

## Dispatch brief (verbatim from the coordinator)

> Independent black-box playtest of a new Rally page (apps/porthole/games/racing, launcher name "Rally"), on branch claude/brave-einstein-ipuhqp at the current HEAD. Play only through visible controls in the emulator; don't read the game source to decide what to do. […] What's new (the claim to test): on the track cards there's now a "Times" button between Home and the Rivals chip. It opens a "Best times" page without racing: one track at a time with arrows to flip tracks, the fastest race and fastest lap across every kid on the device, then each kid's best race and best lap; "No times yet" on a track nobody has finished; Back returns to the cards. The Home button and Rivals chip got narrower to make room.
> Personas: 1. Mia, 6, an early reader with imprecise fingers, whose big brother has played. Set up two or three profiles, have Leo actually finish a race or two (natural driving; `dbg lastlap` fine if disclosed), then switch to Mia and, without racing, try to find out "who is fastest". Does she find the Times button? Can she tell what the page says? Do mis-taps on the narrower Home / Times / Rivals row hit the wrong button? Does she get back out? 2. Leo, 10, competitive: after a few races, does Times show what he expects (his best race and best lap, and his sister's)? Do the arrows feel natural? Anything confusing about "Fastest race" vs the rows? Also check the tracks page still reads well with the narrower buttons.

The brief was not revised.

## Setup, fixtures and disclosure

- `reset`, then `profile Leo 10` and `profile Mia 6` (a disclosed profile fixture). I switched profiles through the visible picker (home button, then a name).
- I found Rally on launcher page 2 (the red car) by looking at the screen.
- **Leo's races were real races, without `dbg`.** All four of his attempts used scripted, blind inputs. I did not steer by watching frames. The attempts were: hands-off (s09), boost and rocket taps (s10), hands-off on Night Run (s11), and four Desert variants in s16 (boost taps, constant left lean, constant right lean, a weave). Mia's race (s15) was also hands-off. These inputs are a crude stand-in for a child's driving. Racing quality was out of scope here.
- `dbg lastlap` was used only in the supplementary wide-name stress tests (s18, `a9`). It gives unrealistic 0:03.2 race times.
- Source exposure: I did not read the game source. After the first-play passes I read the `79ca717` commit message and file list (not its diff) to understand the binary change. I also read the existing scenario `tests/playtests/a7_racing_times.txt` after the persona passes. Both happened before the supplementary scenario was written.

## Paths played

| Script | Persona / purpose | Outcome |
| --- | --- | --- |
| s01–s05 | Leo enters via picker, launcher page 2, Rally; opens Times before any race | "No times yet" (shots/tm_s05_times_empty.png) |
| s06–s11 | Leo: how-to, calibrate, Go; hands-off Desert (1:55.7, lap 36.6); replay with boost/rocket taps (1:56.6, +0.8, slower); Night Run (2:55.8) | 4th every time; results show "Best times" |
| s12–s13 | Leo taps Home; picker -> Mia; page 2 -> Rally; Mia taps the desert picture (how-to), Back; overshoots right on the button row (Rivals Auto->Easy); taps Times | Board: "Fastest race Leo 1:55.7 / Fastest lap Leo 36.6 / 1 Leo 1:55.7 lap 36.6" |
| s14 | Mia flips arrows: > x4 (wraps), < (wraps back), taps the text, taps beside the arrow, fast double tap, hold, drag off an arrow, Back | All correct; drag off cancels; fast double tap flips twice; Back -> cards |
| s15 | Mia races Desert hands-off: 1:55.6 "Track record!", then Times | Mia row 1 (yellow = current kid); lap tie 36.6 vs 36.6 credited to Mia |
| s16–s17 | Leo returns and beats her (boost taps): 1:55.0, lap 36.1; Times shows him first; flips to Night Run, Back, Times again | Reopen remembers Night Run within the session |
| s18 / a9 | Four kids with 8-letter wide names | Fits inside the glass, no UI findings |
| s19 | Rivals chip tap and double tap; Times corner tap | Chip cycles Auto/Easy/Normal/Hard; corner (66,120) opens Times |
| s20 | Reboot while on the Times page | Back on the picker; times persist; board opens on Desert Canyon |
| mistap_probes.txt | 40+ offset, gap, double-tap, hold and drag probes in fresh processes | see below |

## Qualitative assessment

**Can the persona recognize the goal and actions and see what changed?**
Leo (10): yes, right away. "Times" next to Home reads clearly. The page title "Best times" and the arrows next to the track name are where he expects them. Flipping is instant, with wrap-around both ways (tm_s14_sheet.png). His own row is yellow, so he can find himself.
Mia (6): partly. She is drawn first to the track pictures, not the buttons. In s13 she taps the desert picture, gets "How to race", and backs out. "Home" is a sight word, "Times" is a decodable word, and she can probably sound it out. But nothing on the button says "who is fastest" to a six-year-old: there is no trophy, stopwatch or podium. Most likely she finds it by trying each of the three buttons. Once she is on the page, the answer is the first name in the rows: "1 Leo". She can recognize her brother's name and the "1". She cannot read "Desert Canyon", the clock-format times, or the difference between "Fastest race" and "Fastest lap". The page has no pictures at all: no track art (the cards have it), no kid faces (the profiles have them) and no trophy.

**Interesting decisions, consequences and feedback; do pictures help?**
The page is a glance-and-leave screen, not a game, so the only decisions are which track to look at and when to go back. Feedback is immediate. For Leo the motivating moment is real: he sees Mia's "Track record!" on her results (tm_s15_mia_result.png), opens Times, and the next race puts him back on top (tm_s16_a.png, tm_s17_leo_board_desert.png). Reading carries the whole interaction for Mia. Pictures do not help her here.

**Where does it become repetitive, confusing or frustrating? Where would the persona stop?**
Mia would stop at her first look at the populated board. She says "Leo is first", taps Back, and taps a track picture to race (about 1–2 flips at most). That is a satisfying natural stopping point. Her friction:
- the three near-identical lines on a one-kid track (header lines and row 1 say the same thing, tm_s13_3_board.png);
- she is missing from the list until she finishes a race, so she sees no "Mia" at all.

Leo would flip through all four tracks once. He would stop on empty tracks ("No times yet"), and then want to go race. His friction is the lap tie. Both laps show 36.6 but "Fastest lap Mia 36.6" names only her (tm_s15_mia_board.png), and a competitive 10-year-old will dispute that. He reads the header as "the records" and the rows as "everyone". Since the rows already show both values for each kid, the header is redundant whenever one kid holds both records, which was every case I produced.

**Recovery from imprecise input, wrong guesses and interruptions:**
Good.
- Every hit region matches its drawn button. The 2 px gaps between Home, Times and Rivals are dead: x=60 and 61 do nothing, and so do 94–97 on the Times/Rivals side.
- Taps just above or below the row do nothing.
- Dragging off Times, or off an arrow, cancels.
- A fast double tap (100–300 ms) acts once.
- A wrong tap on Home exits to launcher page 2 with the Rally tile visible, so she is one tap from coming back.
- An overshoot onto Rivals changes the setting (Easy/Normal/Hard). The label shows the new value, but Mia would not notice or understand it.
- Times persist through a reboot (tm_s20_board_after_reboot.png).

The tracks page still reads well with narrower buttons. "Home" and "Times" fit their boxes with margin, Rivals plus its value fits, and the row sits inside the glass (tm_s04_rally_b.png).

## Gesture / interruption matrix

| Case | Tracks row (Times) | Board arrows | Board Back |
| --- | --- | --- | --- |
| centered tap | opens board | flips | cards |
| offset 2–4 px inside edge | acts | acts | acts |
| tap in gap / just outside | nothing | nothing | nothing |
| fast double tap (<=300 ms) | once | flips twice (expected) | once |
| slow repeat (>=500 ms) | **bounces back via Back** (Nit 6) | flips twice | **bounces back via Times** |
| hold 1 s / long press 2 s | opens on release | flips once | not tested |
| drag off before release | cancelled | cancelled | not tested |
| ordinary leave/resume (Back, Home, reopen) | track remembered within session | | |
| process restart (`reboot` on board) | picker; times kept; board resets to Desert | | |
| rest timer / daily cap while on board | GAP | | |
| physical touch panel | GAP | | |

## Findings (no Blocker, no Major)

1. **Minor, comprehension.** The Best times page is text only. It has no track picture, no kid faces and no trophy/podium cue, so an early reader cannot tell which track she is looking at. Screenshots: shots/tm_s13_3_board.png, shots/tm_s14_sheet.png. Repro: s13 then s14. Suggest the card's track picture beside or under the name, and each kid's profile face on their row.
2. **Minor, comprehension.** The "Times" button has a word only, with no icon. Its meaning ("who is fastest") is not visible to a 6-year-old, so she finds it by trial. Screenshot: shots/tm_s04_rally_b.png. Suggest a stopwatch or trophy glyph.
3. **Minor, comprehension.** The "Fastest race NAME / Fastest lap NAME" lines repeat row 1 whenever one kid holds both records. On a one-kid track the same numbers appear three times. Screenshots: shots/tm_s13_3_board.png, shots/tm_s17_leo_board_reopen.png. Suggest either a single header line, or marking record values in the rows (for example a star) and dropping the header.
4. **Minor, fairness/feedback.** A lap tie at the displayed precision is credited to one kid: "Fastest lap Mia 36.6" while Leo also shows 36.6. Screenshot: shots/tm_s15_mia_board.png. Repro: s15. Suggest naming both kids or showing hundredths when the tenths match.
5. **Minor, recovery/feedback.** Overshooting Times to the right by about 4 px or more changes Rivals (Auto -> Easy -> Normal -> Hard). The change persists and only the small yellow value changes. The chip is grey-labelled and borderless, so it does not read as a button to a child, and its consequence is invisible to Mia. This is pre-existing behaviour, but the narrower row brings it closer to Times. Screenshot: shots/tm_s19_rivals_once.png, shots/tm_s19_rivals_double.png. Repro: s19 (`tap 100 129`).
6. **Nit, input.** Times (x62–94, y117–141) and the board's Back (x56–104, y120–144) are stacked in the same place. A deliberate second tap 500 ms or more later bounces from the board back to the cards, and from the cards back to the board. Repro: mistap_probes.txt ("double-tap Times gap 500ms -> race_tracks"). A child repeat-tapping after the screen changes would see it flicker back.
7. **Nit, visual.** The arrow buttons use small "<" ">" text glyphs (about 4 px wide in a 26x24 box), weaker than the launcher's big triangles. Screenshot: shots/tm_s05_times_empty.png.
8. **Nit, comprehension.** A kid with no time on a track is not listed. Mia, looking for herself, sees nothing (shots/tm_s13_3_board.png). Optional: a dim "Mia: no time yet" row.

Out of scope, noted only: "How to race" shows again every time Rally opens, even for a returning kid (s16 debugging). All 4th-place finishes came from crude blind inputs, so they say nothing about rival difficulty.

## Automated suite

`make playtest` at `79ca717` (pinned worktree, plus the new `a9`): **84 of 84 pass, 0 UI FAIL, 0 UI WARN.** The report is copied to `automated_report_79ca717.md`. The earlier run on the shared tree (83 of 83, 0/0) is superseded because that tree had concurrent uncommitted Rally edits.

New scenario: `tests/playtests/a9_racing_times_fingers.txt` (16 checks). It covers:
- gap taps;
- a tap just above the row;
- drag-off on Times and on an arrow;
- a fast double tap on Times;
- a tap beside an arrow;
- four wide-named kids on the board, with `ui-check`.

## Judgment

**Ready for the audience at the level of "no Blocker/Major".** Leo is fully served. Mia can find the page by trial, get the answer from name recognition, and get out safely.

The comprehension Minors (1–3) are what stand between "usable" and "good for a 6-year-old". I recommend fixing 1 and 2 (pictures and faces, and a button icon) and 4 (ties) before calling the page child-friendly. If they are fixed, a fresh agent should retest Mia's first-time discovery.

Suggested brief change for a retest: provide a setup where one kid holds the fastest lap and another the fastest race. My blind inputs never produced that split, so the header-versus-rows confusion in that case is untested. Also add a rest-timer or daily-cap interruption while the board is open.

## Resolutions (implementer, 2026-11-01)

- Screenshots: `apps/porthole/.gitignore` drops `*.png`, so the cited shots are committed as `.jpg` beside their
  names (`shots/tm_key_frames.jpg`, `shots/tm_s14_sheet.jpg`, and the seven the findings cite).
- No Blocker or Major: the page lands as is in dreamiurg/porthole#128 at the user's request to land.
- Findings 1 (track picture and faces on the page), 2 (an icon on Times), 3 (record lines repeating row 1), 4 (lap
  ties at the shown precision), 5 (the Rivals chip reading as a label) and 6 (Times and Back in one spot): open,
  offered to the user as a follow-up; 1 or 2 needs a fresh first-play retest with Mia.
- Rival changes in `a7e6ee9` (after the tested `79ca717`) do not touch the Times page or the track cards.
