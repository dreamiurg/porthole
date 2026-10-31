# Frame rate: board measurements and Biscuit Home playthrough

2026-10-19. What changed: indexed frames are expanded only where a logical row changed (`gfx565::Expanded`), scaled
blits and RLE scenes are drawn a row at a time, Biscuit Home skips frames its buffer already holds, and the loop
neither renders nor presents while the screen stays dark.

## Board numbers (serial `M`, 64-frame averages)

Same board, same saves, flashed back to back: main (10bb122 plus two non-perf fixes) against this branch.

| Screen | main | this branch |
| --- | --- | --- |
| Picker (indexed) | 34.2 ms, 29.0 fps; present 32.2 ms (expansion plus wait) | 17.1 ms, 58.5 fps; render 2.9 ms (expansion included), present 13.1 ms (wait) |
| Admin code pad `sys_pin` (RGB565, not changed here) | 51.3 ms, 19.5 fps; render 35.0 ms | the same |
| Dark (backlight 0) | rendered and presented every frame | render 0, 21 ms loop (20 ms delay); a tap wakes it into a fresh frame at 58.5 fps |

Pets Club and Biscuit could not be opened on the board: every profile there has a secret code. Biscuit Home was timed
on the host only (`snap --script`, `perf`): 0.147 ms a frame on main, 0.03-0.05 ms on this branch. Host times say
nothing absolute about the board.

Why the list's other items were not done, from a throwaway experiment on the board (not committed):

- (a) panel on core 0: the LCD ISR takes ~8% of core 1 (a fixed spin: 45.7 ms on core 1, 42.0 ms on core 0), and one
  full-frame PSRAM write takes 23.6 ms on core 1 against 20.7 ms on core 0. A full redraw stays over one 17 ms period
  either way, and `update_scanout` only works from the core that made the panel, so moving it would break updates.
- (e) `-O2` on app sources: the firmware then overflows its OTA slot by 35 KB (main has 35 KB left of 8,323,072), and
  the hot paths are PSRAM-bound, not CPU-bound.

## Emulator playthrough (Biscuit Home)

Build: this branch, `build/host/snap`, with the sim checking every row-skipped indexed frame against a whole expansion.
Fixture: `profile Sam 8` (one profile, no PIN), as the Biscuit playtests use; then visible taps only. Played in
`make webemu` and replayed in a fresh process with [input-trace.txt](input-trace.txt) (`snap --fresh --script`).

Home ([01](fr01_home.png)), Feed ([02](fr02_fed.png)), petting ([03](fr03_petted.png)), fetch from the first ball to
five catches ([04](fr04_fetch.png), [05](fr05_fetch4.png), [06](fr06_fetched.png)), a nap from the window and waking
([07](fr07_nap.png), [08](fr08_woken.png)), World and back ([09](fr09_world.png), [10](fr10_back_home.png)), and out
to the launcher ([11](fr11_launcher.png)); then Biscuit again from the launcher. Every frame was whole: needs, speech,
the ball moving between catches, the night room and the room after World showed no leftover pixels. No findings.

Not covered: a physical-board playthrough of Pets Club or Biscuit (secret codes, above).
