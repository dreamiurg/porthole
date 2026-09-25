---
name: art
description: Pets Club (apps/porthole). Use when adding, changing, or regenerating pixel art -- dog poses, size variants, or icons. Covers regenerating sprites, reviewing the sheet, and what to check before calling it done.
---

1. Edit `apps/porthole/games/pets-club/tools/art.py` only. Never hand-edit `apps/porthole/games/pets-club/sprites.h` -- its own header says it's auto-generated, and a hand-edit is silently overwritten by the next regeneration.
2. Regenerate: `make -C apps/porthole art` (runs `python3 apps/porthole/games/pets-club/tools/art.py`). This rewrites `apps/porthole/games/pets-club/sprites.h` and, if Pillow is installed, `apps/porthole/build/art/sheet.png`.
3. `python3 apps/porthole/games/pets-club/tools/art.py --check` verifies `sprites.h` matches the generator without rewriting it -- `make -C apps/porthole check` (and so pre-commit) runs it. Use it if you want to confirm you didn't forget to regenerate after a `apps/porthole/games/pets-club/tools/art.py` edit.
4. If Pillow isn't installed, `make -C apps/porthole art` still succeeds (`sprites.h` is written) but prints "Pillow missing; skipping preview." Say so rather than skipping the visual review silently -- install Pillow or ask for a human look instead.
5. Open `apps/porthole/build/art/sheet.png` and check, per pose/size row: the 1px dark-brown (`C_DKBROWN`) inner outline is continuous, the dog still faces right, the silhouette reads at the smallest size (`pup`, roughly 24x18 px), and every color used is one of the 32 in `apps/porthole/os/palette.h`.
6. If a pose's proportions changed or a pose was added, the `DOG_PARTS` anchors (`headX/Y/R`, `bodyX/Y/RX/RY`, `tailX/Y`, `eyeX/Y`) shift or gain a row. Report the new numbers explicitly -- `game.cpp` reads them directly for hit-testing and effect placement, and a silent shift breaks gameplay with no compiler error.
7. Confirm nothing else broke: `make -C apps/porthole test && make -C apps/porthole snap` (both pull the regenerated `sprites.h` in transitively).
