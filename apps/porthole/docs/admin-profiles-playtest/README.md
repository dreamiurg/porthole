# Admin code before New and delete: black-box playtest

Final outcome: with an admin code set, the picker's New and long-press delete both asked for it; back and a wrong
code changed nothing; the right code led to a completed new profile and a completed delete. No open findings.

Tested commit `f09d7ed` (branch `claude/jovial-lichterman-182598`) on 2026-10-19 in the emulator only, not on a
board. `build/host/snap` SHA-256:

```text
53dcda4e5d07cbace02d3ed79411531658bc60b9699522f16e6bb3cd6ed6d108
```

Re-checked after merging main on 2026-10-31: the replay reproduced every frame of the New/delete gate unchanged.
Four frames were re-captured because main changed screens outside this flow: `05-admin`, `08-saved` and
`08b-saved-settled` (the admin menu's "Reset all limits" became "Screen time") and `21-launcher` (picture tiles, #99).

## Method and setup

`snap --serve`, inspecting native 480x480 frames before choosing each next input. Entered the grown-up menu by
holding "Who's playing?" for five seconds. No `dbg`, state queries, `app` shortcuts or clock jumps during play.

The [fixture](fixture.txt) prepares two existing profiles: Sam, age 8, kid code 1234; Kai, age 6, no code. No admin
code, no game progress. The [input trace](input-trace.jsonl) holds the logical touches and elapsed ticks; each entry ends
on the named frame. `python3 ../admin-playtest/replay.py ../admin-profiles-playtest` (needs Pillow) replays it in a fresh process and
checks every saved frame.

## Paths and observations

| Task | Observed result | Native evidence |
| --- | --- | --- |
| New, no admin code | New opened the name keyboard directly, as before; Home returned to the picker. | [keyboard](02-new-no-code.png) |
| Set admin code | Info, Admin, Set admin code, 2468 twice: "Menu code: on". | [saved](08b-saved-settled.png) |
| New with the code | New showed "Admin code?". Home went back to the picker with nothing changed. 1111 showed "Wrong code. Try again." and stayed. 2468 opened the name keyboard. | [asks](11-new-asks-code.png), [wrong](15-wrong-settled.png), [open](16-new-open.png) |
| Complete New | Typed Mia, picked the bunny, age 7, skipped the code: Mia's launcher. | [launcher](21-launcher.png) |
| Delete, back | Long press on Kai showed "Admin code?"; Home kept all three profiles. | [asks](23-hold-kai-asks.png), [kept](24-back-kept.png) |
| Delete a coded kid | Long press on Sam (who has a kid code) asked for the admin code, not Sam's; 2468 opened "Delete Sam?" with the finger's side on "No, keep!". No kept Sam. | [delete](26-delete-sam.png), [kept](27-no-keep.png) |
| Complete delete | Long press on Mia, 2468, held "Hold to delete": Mia gone, New back. | [confirm](29-delete-mia.png), [deleted](30-mia-deleted.png) |
| Menu still gated | Holding the title still asked for the code; 2468 opened Info. | [asks](31-menu-asks.png), [info](32-menu-open.png) |

## Findings

- Accepted, not changed: the admin code screen reached from New or delete has the same "Admin code?" prompt as the
  menu, with no line saying what it unlocks. A grown-up who set the code knows why; adding words would change a
  screen the rest of the admin flow already uses.

## Related change not covered here

The same branch stops the first saved UTC offset from shifting the clock (network time now waits for a saved
offset). The earlier [admin playtest](../admin-playtest/README.md)'s "Saving +15 visibly shifted 16:00 to 16:15"
row describes the old behavior: saving a first offset now leaves the shown time alone. `tests/playtests/28_clock.txt`
covers the new rule.
