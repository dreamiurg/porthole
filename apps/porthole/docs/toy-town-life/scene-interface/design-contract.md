# Toy Town: scenes before prose

The user approved [this School reference](approved-school-reference.png) on
2026-10-27 and asked to carry its interface style through the game. It is a
generated visual target, not an emulator capture. Original design references
and shipped saves remain intact. This implementation branch builds on the
independent child-playtesting instructions in PR #95.

## Player-facing contract

- Show Home, School, Cafe and Park as recognizable places, with the customized
  doll standing and walking in the world. Tap free floor to walk; characters,
  props and their actions show what is happening. No enormous prose panel.
- School contains Mina at her presentation/easel and Theo, a boy, at a partly
  hung art display. The default scene offers the five approved illustrated
  actions: Help Theo hang pictures; Plan with Theo; Ask Mina for help; Practice
  with Mina; Borrow paper. They must describe their actual consequences.
- Talking/planning opens short named speech, not a premature commitment. Mina
  explains her difficulty starting; Theo can help after his display. Borrowing
  paper supplies a plan without silently completing the presentation. The final
  plan still yields four recognizable outcomes and an opportunity to repair.
  Show hanging pictures, cue cards, waiting/practice and repaired displays.
- Cafe shows Mina, the counter, real pictured food and a visible tray. Prices
  remain 5/3/1/2 and the budget 10. Choose food directly, add/remove quantities
  explicitly, see the sum and remaining coins, compare trays, and repair invalid
  orders. Pictures support the arithmetic; no hidden answer quiz.
- Park shows Nori and visible bench/fountain/path landmarks. Clues match the
  saved variant. The notebook/ribbon is discoverable in the scene; incorrect
  ideas can be revised, and returning the notebook visibly reunites it with Nori.
- Carry the same scene/picture-button style through Home, destinations, packing,
  scrapbook, memories and stopping. Keep the shipped pictured customization.
  Five meaningful choices belong on the main School scene; focused editors may
  have fewer controls. Never invent filler actions to reach a button count.
- Use readable native text, short specific action labels, actual scene/object
  illustrations, strong outlines and pressed/selected feedback. All controls
  meet 24x22 logical pixels and circular safety; the scene remains visible around
  them. Do not restore a wall of words to fit all original copy.
- Every page has pause/back/exit and a natural stopping point. Story, appearance,
  bag, scrapbook and exact story/edit page survive interruptions and profile
  changes. No pressure mechanics, appearance ranking, purchases or lost progress.

## Engineering contract

Keep all shipped IDs/fields and V1/V2 decoding. V3 may append world position,
borrowed-paper state and a quantity-editor page; historical sizes/CRC/version
checks and frozen fixtures stay strict. Motion is elapsed-millisecond animation,
not a reward timer. Commit movement on arrival rather than writing NVS per frame.
New art is original layered runtime data, not a screenshot used as the game.
Stay within the existing OTA slot and 131072-byte static internal-RAM ceiling.
Do not change other games, shared launcher geometry, or partition sizes.

## Acceptance

Meaningful rules/migration/input tests, all existing playtests adapted to actual
new visible input, full CI, complexity and firmware build must pass. A fresh
independent agent with an age-eight persona must discover controls and naturally
complete all three outings using inspected native frames and ordinary inputs,
including wrong choices, imprecise taps, back/exit/resume and boredom assessment.
Fix findings and refine the playtester brief without coaching; replay the final
artifact after edits. Save build hashes, prompt revisions, inputs, screenshots,
findings/resolutions and truthful emulator/hardware scope in this directory.
