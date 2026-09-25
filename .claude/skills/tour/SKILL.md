---
name: tour
description: Any game (apps/porthole). Use when you need screenshot evidence of a screen or flow, or a quick ad hoc UI audit outside the formal playtest suite. Covers writing a raw snapshot script, running it, building a contact sheet, and reviewing it.
---

1. Write a plain-text script, one command per line. This is the *raw* grammar `apps/porthole/host/sim.cpp` understands directly (see its own header comment, which is the authoritative list if this drifts):
   `tap X Y | hold X Y | down X Y | move X Y | up | wait MS | skip SEC | snap NAME | reset | profile NAME AGE [PIN] | app NAME | newgame KID PET | debug | dbg CMD | ui | screen | echo WORD | watch MS | monkey N SEED`.
   A line starting with `#` is a comment. Coordinates are logical px, 0-160, inside the circle of radius 80 around (80,80).
   - This is a different, lower-level language than the `expect`/`expect-screen`/`ui-check`/`follow-glow`/`solve-word`/`answer-book`/`repeat...end` directives used by `apps/porthole/tests/playtests/*.txt` and `make -C apps/porthole playtest` — those are compiled down into this raw grammar by `apps/porthole/tools/playtest.py`, not read by `sim.cpp` itself. Use this raw grammar for a one-off ad hoc script; use the directive language (see the `playtest` skill) for a scenario that should join the formal suite.
2. Start every script with `reset` (a fresh device: the "new profile" screen), `profile NAME AGE [PIN]` (create and select a profile: the launcher; `app pets-club` opens Pets Club, `app biscuit` opens Biscuit: its name keyboard for a new pup, where `tap 119 125` takes the typed "Biscuit") or `newgame KID PET` (a profile with an adopted pup, straight into Pets Club) so it's reproducible from a clean save.
3. Run it: `cd apps/porthole && make snap && ./build/host/snap --script path/to/yours.txt`. Every `snap NAME` line writes `apps/porthole/build/host/<NAME>.bmp`.
4. Add a `ui` line right after reaching a screen you care about. It dumps that exact frame's hit regions and text boxes (logical px) to stdout — for an indexed game (Pets Club today) each text box prints its drawn and background color index; for an RGB565 game (Biscuit) the same line prints `rgb=RRGGBB bg=RRGGBB` instead. Use this instead of eyeballing the `.bmp` for tap-target sizes or text clipping. `screen` alone just prints the current screen name; `debug` dumps the full stat line from `Game::debugPrint()`.
5. Build a contact sheet so a whole flow can be reviewed at a glance instead of opening every file:
   ```python
   from PIL import Image
   import glob

   paths = sorted(glob.glob("apps/porthole/build/host/*.bmp"))
   imgs = [Image.open(p) for p in paths]
   cols = 5
   w, h = imgs[0].size
   rows = (len(imgs) + cols - 1) // cols
   sheet = Image.new("RGB", (w * cols, h * rows), (32, 32, 32))
   for i, im in enumerate(imgs):
       sheet.paste(im, ((i % cols) * w, (i // cols) * h))
   sheet.save("apps/porthole/build/host/contact_sheet.png")
   ```
6. Review the contact sheet (or the individual `.bmp` files if Pillow isn't installed — don't skip the review) for round-edge clipping, overlapping controls, and that every screen shows the feedback it should.
