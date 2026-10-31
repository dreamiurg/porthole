# Independent native-size child-perspective playtest

## Completed assessment

The quieter places support readable play in this simulated age-eight run: Mini was immediately identifiable, all five places were visited, and people and landmarks stayed legible at native 480. School display, Cafe lunch and Park notebook return reached ordinary saved stopping points. Final4 Park Save & leave, visible launcher reentry and a normal restart retained the returned notebook and safe stop. This is a bounded qualitative acceptance of the observed paths, not exhaustive usability evidence.

The initial Cafe doll-on-tray occlusion was a material failure and is preserved below. Final3 direct tray interaction and Home/resume retest resolved it: Mini stayed beside the tray, food/water/counts remained visible. Final4 tray/menu edge checks also worked; a full fruit-and-water Cafe lunch was then completed to continue naturally to Park.

Remaining friction: destination thumbnails are too small to identify confidently without labels; “Town” in a paused outing leads Home; repeated Add/Done/Tray steps tire this persona. A direct tap on the already found Park notebook returns to the clue screen instead of returning it, adding another Try my idea/Search sequence. These did not block completion. The original Cafe stop was the voluntary stopping point; all later retests and Park coverage were prompted continuation, not spontaneous sustained child engagement.

## Build boundaries and continuation fixtures

| Binary / SHA256 | Observed independent scope | Starting store |
|---|---|---|
| Initial `/tmp/toytown-quiet-final-snap` — `df08c56bbfa97c19d18852639ea05dc2d8943f502e9e19f4e857a1b8cc37d5b9` | Fresh profile/launcher, Mini/wardrobe, Home/neighborhood, School display and pause/resume, Cafe mistake/correction/order/saved stop | Empty |
| Final3 `/tmp/toytown-quiet-final3-snap` — `c370de00dcc6c9dfdf64052b9f1cdab239dac44396bf4b135a939c1e56200f9c` | Direct tray, complete new Cafe lunch with missing-water mistake/recovery, Home movement/resume, natural saved stop | Root copied initial Cafe safe-stop on-disk saves unchanged |
| Final4 `/tmp/toytown-quiet-final4-snap` — `7b1c6c997fbc24b8defdad9766f765a796b7dcde7fa7aab19e65aaee01e7ffb7` | Direct tray and imprecise menu edge; complete fruit/water Cafe lunch; full Park clue/hypothesis/recovery/return; safe leave/reentry/restart; prompted mid-walk action, card-drag cancel and held/released control supplement | Root copied final3 safe-stop on-disk saves unchanged |

Final3 and final4 binary SHA256 values were verified locally before play. Both entered through visible M profile and launcher, with no debug/state shortcut. Final3 used port8768, final4 port8769, with the same ordinary endpoint protocol. The copied save stores are continuation fixtures; they are not new empty-store runs.

## Final3 Cafe continuation

Direct tray tap at (370,240) opened Tray review and left Mini beside it ([000073](final3/frames/000073-direct-tray-opens.png)). Added sandwich, reviewed without water and received an explicit drink prompt. Pause > Town opened Home with saved outing; tapping (350,248) visibly moved Mini right ([000113](final3/frames/000113-home-movement.png)). Resume my outing returned to Tray5/10 with sandwich retained and Mini back to the safe left of tray. Added water; both food and drink quantity labels remained visible ([000133](final3/frames/000133-water-visible.png)). Ordered, Mina ate, remembered and kept the memory, reaching the saved stop. No occlusion recurrence on this route.

Home pale floor taps at (180,285) did nothing, while (350,248) moved the doll; some visually open floor is outside the walking area. This limits the literal “Tap the floor to walk” promise. Walking itself was not needed to reach the action cards.

## Final4 Cafe and Park continuation

Final4 direct Cafe tray tap opened review ([000058](final4/frames/000058-direct-tray-review.png)). A touch near the Fruit bowl card's top edge (306,270) opened Fruit bowl rather than the tray ([000068](final4/frames/000068-menu-edge-fruit.png)). Added fruit and water, tapped visible tray to review, ordered lunch, skipped memory, and reached saved stop. Items and labels stayed readable. To select another destination naturally required completing the outing: unfinished Home offered Resume my outing, wardrobe and scrapbook rather than a new destination.

Park is immediately recognizable through its bench, fountain, tree canopy and winding path. Yellow magnifier markers indicate useful places more clearly than background art alone ([000143](final4/frames/000143-park-arrival.png)). Nori and blue Mini are visually distinct. The first curious tap on the fountain found the ribbon/notebook clue immediately; this was a spontaneous correct first location, not a wrong hypothesis. To cover error recovery after that, I continued Look elsewhere, directly tapped the bench, chose Try my idea and Search here. “No notebook here. Let's look somewhere else” was clear and gentle ([000168](final4/frames/000168-bench-wrong-hypothesis.png)). Change my idea switched to Fountain, then Search here found the notebook. This wrong-location exercise was deliberate continued coverage after the natural discovery, and must not be presented as the child's original guess.

The pink notebook is small but discernible on the fountain edge. After a direct approach tap, Mini stands left of it and does not cover it ([000193](final4/frames/000193-notebook-visible-after-approach.png)). That tap reopened Clue: Fountain rather than picking it up, so I followed Try my idea > Search here again. Give it to Nori then visibly removed it from the fountain and placed it in Nori's hands ([000198](final4/frames/000198-notebook-returned.png)). Keep memory (imprecise left-edge tap) reached the explicit saved stop. Save & leave returned to launcher. Reentry restored the saved Park stop and Nori holding it; ordinary restart, wait, visible profile/launcher entry restored the same outcome again ([000252](final4/frames/000252-park-after-restart.png)). Finished with Save & leave at the launcher.

## Prompted final4 gesture and motion supplement

After the report was initially completed, root requested narrow additional coverage for motion, drag cancellation and holding. This is explicitly prompted testing beyond the voluntary Cafe stopping point. Same final4 binary/server/store; no source or hidden state was inspected. Reentered through visible launcher, saved Park stop > Keep exploring > Home; no new full outing was completed. Initial brief Park bench-area touches on the saved-stop screen showed no movement. Home was used for the clear motion sample.

| Exact ordinary inputs | Native evidence / outcome |
|---|---|
| Home floor down(180,248), wait32, up(180,248), wait32 | Before: Mini near x384 in [000286](final4/frames/000286-supplement-home-before-motion.png). Mid-walk: near x374, well short of x180 target in [000294](final4/frames/000294-supplement-midwalk.png). |
| Before arrival, down(152,350) on Walk into town, wait32, up(152,350) | Pressed card green while Mini moved near x363 then x351: [000296](final4/frames/000296-supplement-action-pressed-midwalk.png), [000298](final4/frames/000298-supplement-moving-with-pressed-action.png). Release changed to Town while Mini near x342: [000300](final4/frames/000300-supplement-action-before-walk-end.png). Action worked before reaching target; doll, text and action borders remained distinct throughout these native samples. |
| At Bag, down(240,305) on unselected Pencil, wait80, move(240,210), up(240,210), wait600 | Pencil temporarily highlighted green [000321](final4/frames/000321-supplement-pencil-pressed.png); dragging outside removed highlight [000325](final4/frames/000325-supplement-drag-outside.png). After release/wait Pencil stayed unselected and bag stayed3/3 [000329](final4/frames/000329-supplement-drag-cancel-confirmed.png). No accidental pack/change. |
| At Bag, down(96,306) on selected Notebook, wait1200, up(96,306), wait600 | During hold stayed selected at3/3 [000333](final4/frames/000333-supplement-notebook-held1200.png). After release changed once to2/3, Notebook unselected [000337](final4/frames/000337-supplement-held-release-once.png). No repeat during hold. |
| Normal tap Notebook to restore; back > Save & leave | Bag restored3/3 [000342](final4/frames/000342-supplement-bag-restored.png); safely returned to launcher [000352](final4/frames/000352-supplement-safe-leave.png). |

A preliminary Town Cafe-card drag/hold attempt did not show a reliable pressed-state redraw in immediate primitive captures; it is retained in the trace without claiming a definitive hold result. A following normal Cafe tap entered Bag. The definitive drag/hold judgments above use Bag controls and explicit post-release elapsed time. Supplement `/input` and `/wait` calls were individually wrapped and captured, unlike the earlier standard 80/600ms tap wrapper; the trace records every operation. No tray-item drag, multi-touch or system interruption was tested. The card-cancel check covers the requested card-or-tray cancellation sample.

## Evidence

`trace-initial.jsonl`, `final3/trace.jsonl` and `final4/trace.jsonl` are canonical harness traces. Representative native 480 PNGs are unmodified copies in `frames/`, `final3/frames/` and `final4/frames/`; friendly names preserve the exact canonical capture ID. `SHA256SUMS` covers report, copied traces and images. Some repeated screens have identical pixel hashes, but their filenames retain their actual capture event IDs. Root-owned build manifests were not edited.

## Initial build observations (superseded Cafe interaction)

The blue Mini doll is easy to find on the pale backgrounds. Home, neighborhood, School and Cafe read as different places through bedroom furniture, street perspective, chalkboard/desks, and cafe counter/tables. School people match their action-card portraits. The background is quiet enough that people, cream text panels and purple action borders dominate.

A direct tap on the large Cafe tray instead walked Mini onto it. The doll then covered the water and its quantity label throughout lunch ordering. This is an observed concern, not a pass. Original evidence: frames 000237 (before), 000242 (on tray), 000282 (water obscured), 000317 (ready tray). Root was notified during testing. The concern was resolved in the observed final3/final4 continuations below.

Destination thumbnails are tiny cropped strips; I used the words School/Cafe/Park to choose. The larger wardrobe portraits clearly showed selected Mini and the buns hairstyle. Useful objects cannot consistently be distinguished from scenery by appearance: the cafe tray looked useful, but direct tap walked; School chalkboard tap did nothing, while direct Theo tap opened conversation. Reading action buttons was the dependable method.

## Persona, dispatch and independence

Fictional roleplay: age eight, moderately fluent reader, curious about people and pictures, imprecise taps, limited patience. Profile created through visible UI: short name M, cat face, age 8, PIN skipped. This is an agent simulation, not physical-display testing or evidence from a real child.

Dispatched goal: choose a doll; visit neighborhood/Home/School/Cafe/Park; explore activities; leave safely. Focus: immediate doll finding, NPC/object/scenery distinction, recognizable landmarks, actions while walking, native 480 size. Required coverage also includes wardrobe/destination thumbnails, School display and pause/resume, Cafe mistake/correction, Park hypothesis/recovery and notebook discovery/return.

I did not implement the game and did not read source, tests, previous reports/screenshots, approved mockups, walkthroughs, debug output or state queries. Decisions came from actual native 480 PNG frames. Root supplied protocol/build confirmation and requested pausing before Park to revise the observed tray issue; no gameplay solution was supplied.

## Setup and evidence integrity

Initial frozen binary: `/tmp/toytown-quiet-final-snap`, SHA256 `df08c56bbfa97c19d18852639ea05dc2d8943f502e9e19f4e857a1b8cc37d5b9`. Fresh empty store, local emulator HTTP at port 8766. Only GET `/frame.png`, POST `/input`, POST `/wait` were used on this initial build. Each tap: down, 80 ms ordinary wait, up, 600 ms ordinary wait, then inspected actual frame. Initial blank circle became profile UI through visible home control. Launcher pagination used visible arrows until Toy Town appeared.

The first profile-setup actions mistakenly saved helper copies/log lines into the server's evidence directory, producing doubled numbering. Root verified one server and one player, correct frozen binary. Existing records were preserved. Harness records carry `seq`/`event`; helper duplicates do not. Subsequent helper downloads are in `/tmp/quiet-child-view`, leaving canonical server trace/frames exclusively to the harness. Representative images are unmodified native PNG copies; canonical original IDs are retained in friendly filenames.

## Observed route and comprehension

Chose Mini, changed hair to buns. Selection/check feedback was obvious. Back from doll choice opened a Home pause menu; Resume returned to doll choice. Town resolved navigation, but the detour felt less direct than expected.

Home: visible bed, wardrobe and soft room light; Mini stands clearly on the pale floor. Town: sunny neighborhood road and houses, matching the introductory street. School: windows, green chalkboard and desks; Theo carries pictures, Mina is visibly separate. Tapped scenery chalkboard without result, then Theo directly, which moved Mini to him and opened choices. Chose Hang pictures, selected flowers/rocket/rabbit and tapped three frames. Pause after the first picture, then Resume retained it. Finishing placed pictures visibly on the classroom's right board. Kept memory. The explicit saved stopping screen made a reasonable School break.

Cafe: counter, chair/table and conspicuous tray make the setting readable. Tapped tray before menu; Mini walked onto it and remained there. Ordered a cookie as the plausible but incorrect lunch hypothesis. Tray said water was needed, so added water; next prompt said food was needed, explicitly sandwich or fruit. A cookie not counting as food was mildly surprising. Added fruit, then ordered at 6/10 coins. Mina visibly ate. Remember this, Keep memory produced “Your town is saved. Stop here, or keep exploring.” Correction was understandable through text, without external hints.

Voluntary stopping: after Cafe's saved screen, this persona would stop. The repeated item > Add > Done > Tray loops were tiring even though understandable. Any later Park testing is continued coverage beyond that voluntary stopping point, not evidence of sustained spontaneous child engagement.

## Gesture and interruption coverage

| Gesture/interruption | Observed result |
|---|---|
| Normal taps on wardrobe, cards and pictures | Reliable selection; clear green selected feedback |
| Direct School chalkboard scenery tap | No visible action |
| Direct Theo tap | Walked Mini toward Theo; conversation opened |
| Direct Cafe tray tap | Walked Mini onto useful tray art; persistent item occlusion |
| School pause during partial art display | Pause menu appeared; Resume preserved first picture |
| School Town from pause | Returned to Home with Resume my outing; this resumed School |
| Save/leave, launcher reentry, restart | Final4: Park saved stop and Nori holding notebook survived both reentry and restart |
| Imprecise edge taps | Final4: near top of Fruit bowl (306,270) selected Fruit bowl; near left edge of Keep memory (80,350) worked |
| Card drag canceled outside | Final4 supplement: Pencil pressed, dragged outside and released; bag remained3/3, Pencil unselected |
| Held/released control | Final4 supplement: Notebook held1200ms stayed selected; release toggled once to2/3, no repeat |
| Visible action during motion | Final4 supplement: Walk into town changed scene before doll reached floor destination |

## Limitations

Qualitative findings are this single simulated reader's judgments; there are no timing, contrast-ratio, physical touch accuracy, real-child, audio or accessibility certification claims. Multi-touch and system interruptions were not tested. The final4 prompted supplement sampled native frames during one Home walk, activated one visible action before arrival, canceled one card drag and held/released one bag control. This is a bounded sample, not exhaustive gesture coverage or continuous animation-quality measurement. School was played on the initial binary only. Root reports unchanged School pixels, but that source-aware comparison is not independent acceptance play. The exact build scopes above must remain distinguishable.
