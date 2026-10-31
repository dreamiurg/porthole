# Toy Town Life

A local, silent doll neighborhood for Porthole. Open **Toy Town** on the second
launcher page. Every profile has its own dolls, bag, outing and scrapbook.

- **Doll & wardrobe:** four materials (fashion, plush, clay, keychain), free hair,
  skin, clothes, shoes, accessories, colors and suggested in-world names.
- **Walk into town:** choose a pictured School, Cafe or Park, then tap pictures
  to pack up to three useful items. Missing paper can be borrowed at School.
- **School:** meet Mina by her presentation and Theo by his unfinished display.
  Five pictured actions open real conversations, planning, practice, borrowing
  and picture hanging. Choose Flowers, Rocket or Rabbit and place or replace
  them in three frames; Done finishes the display only when every frame is full.
  Plans lead to different visible results. Later repairs accumulate, so helping
  one friend never undoes help for the other.
- **Cafe:** four pictured foods with prices and a 10-coin pretend lunch budget.
  Choose a food, use explicit plus/minus controls, compare a saved tray with
  another, and correct an order that needs food, water or fewer coins.
  Mina requests a sandwich or fruit plus water. Tap the visible tray to review
  it; the doll stays clear of its food and quantity labels.
- **Park:** ask Nori, tap the bench, fountain or path to inspect it, and try or
  revise an idea. The pink ribbon appears at the saved notebook location; an
  inspected place becomes the proposed search. Giving the notebook back places
  it in Nori's hands. Deliberate new visits vary the location and evidence.
- **Scrapbook:** eight recent memories, optional captions and stickers. A natural
  stopping screen follows each outing. No daily rewards or penalties for absence.

The top Back button opens Resume / Town / Exit. Leaving during an outing keeps
its exact page and drafts. Home allows wardrobe changes while an outing waits.
The title's Continue returns to the saved screen. The shell's family play limits
remain authoritative.

Scenes use original backgrounds, separate characters and objects, short speech
bubbles and picture buttons. Tap open floor to walk; tap a friend to approach
and talk. Picture-hanging and quantity editors keep their focused controls.
Movement is a short animation; arrival and interrupted movement are saved without
writing every animation frame. The wardrobe keeps the shipped pictured choices.

Saves are per-profile V3 records (100 bytes); shipped V1/V2 outfits, stories and
memories migrate without losing their original fields. A scrapbook entry is a
stylized place/outcome summary, not a stored photograph: it uses its own scene
and outcome, never the next outing's objects or current walking position.

## Development

Run `make -C apps/porthole webemu`, then visit http://127.0.0.1:8765.
The same source is compiled for the ESP32-S3 firmware and host emulator.

The game's `AGENTS.md` documents persisted state and the asset pipeline. Original
concept PNGs are preserved in the design spec's `assets/reference/` directory;
those moodboards are separate from runtime graphics.

The [scene-interface contract](scene-interface/design-contract.md) records the
approved layout and behavior. The [quiet-background refinement](quiet-backgrounds/README.md)
records the subsequent art direction and validation. See [scene-interface evidence](scene-interface/blackbox/README.md)
for the interaction update and [original-release evidence](blackbox/README.md)
for the first implementation.
