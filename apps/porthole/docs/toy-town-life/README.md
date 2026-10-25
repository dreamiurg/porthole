# Toy Town Life

A local, silent doll neighborhood for Porthole. Open **Toy Town** on the fourth
launcher page. Every profile has its own dolls, bag, outing and scrapbook.

- **Doll & wardrobe:** four materials (fashion, plush, clay, keychain), free hair,
  skin, clothes, shoes, accessories, colors and suggested in-world names.
- **Walk into town:** choose School, Cafe or Park, then pack up to three useful
  items. Missing paper can be borrowed at School.
- **School:** listen to Mina and balance her art presentation with Theo's display.
  Plans lead to different results, with a conversation and repair afterward.
- **Cafe:** a 10-coin pretend lunch budget. Compare a saved tray with another,
  revise quantities and correct an order that needs more coins.
- **Park:** ask Nori, inspect landmarks, try an idea and revise it. Deliberate new
  visits vary the notebook location together with its matching evidence.
- **Scrapbook:** eight recent memories, optional captions and stickers. A natural
  stopping screen follows each outing. No daily rewards or penalties for absence.

The top Back button opens Resume / Town / Exit. Leaving during an outing keeps
its exact page and drafts. Home allows wardrobe changes while an outing waits.
The title's Continue returns to the saved screen. The shell's family play limits
remain authoritative.

## Development

Run `make -C apps/porthole webemu`, then visit http://127.0.0.1:8765.
The same source is compiled for the ESP32-S3 firmware and host emulator.

The game's `AGENTS.md` documents persisted state and the asset pipeline. Original
concept PNGs are preserved in the design spec's `assets/reference/` directory;
those moodboards are separate from runtime graphics.

See [black-box evidence](blackbox/README.md) for tested flows and limitations.
