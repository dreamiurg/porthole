# Battery on the picker and Admin 2/2: independent playtest

Played on 2026-11-28 in the headless emulator (`snap --script`). This is emulator roleplay: no real child was involved and no physical device was used.

## Build identity

| Pass | Revision | `build/host/snap` sha256 prefix |
| --- | --- | --- |
| First pass (all probes) | `ae14e66766162ec79ebff5603917e696a04a0534` (`feat/porthole-battery`; the working tree also had an unrelated `uv.lock` change) | `bd30a5a1a3b5cfd0` |
| Replay and full suite | `3efe61a` ("the battery reaches the shell through shell::System"), plus uncommitted edits to `os/battery.h`, `host/sim.cpp`, `firmware/main.cpp` and others that were in the worktree at build time | `d0673deb3dd52435` |

The implementer committed and kept editing while I was testing. I replayed every probe script (`scripts/a.txt`..`h.txt`) on the second build. All 32 frames were byte-identical to the first pass, and every screen transition matched.

## Dispatch brief (as received)

> Independent playtest of a small UI change. (1) The "Who's playing?" picker shows a small battery icon centered above the title: an outline filled to the charge, the percentage rounded to 5, a bolt while charging, nothing without a reading. (2) Admin page 2/2 (hold the picker title 5 s -> Info -> Admin -> ">") has a top line such as "Battery 76%, 3.99 V, ~5 h left", "Battery charging, 4.19 V" or "Battery 22%, 3.74 V, ~88 min left". Personas: a 6-7 year old who reads a little and has imprecise fingers, picking themselves (does the battery distract, look tappable, get tapped by mistake inside the 5 s hold area; try tapping it, pressing near it, picking a profile right after; with 4 and with 1-2 profiles); and a grown-up checking the battery (find it, understand the line, read at a glance, tell charging from not; every state, including 3720 and 4200). Also check round-glass clipping (ui-check) and that the bolt and digits are legible at native scale. Battery state is set with `sys battery MV`, `sys charging MV` and `sys nobattery`.

Brief revisions: none. Suggested improvements for next time:
- The brief gives exact screen paths, so it is not a source-free first-play brief. That fits a narrow presentation change, but the dispatcher should say so explicitly.
- Name the widest label (charging at 100%) and the empty states (0-5%) as required cases.
- Say that the UI audit logs only text, so any drawn icon has to be measured from pixels.

Disclosure: before play I read `apps/porthole/AGENTS.md`, which describes the feature, and the existing scenario `tests/playtests/c4_battery.txt`. I did not read the shell or battery source. I chose coordinates from `ui` dumps of the rendered frames.

## Setup and paths

- Fixtures: `reset` and then `profile` commands for 4 profiles (Ann, Bob, Cat, Dan), 2 profiles, and 1 profile. The launcher's orange back arrow leads to the picker. The device boots at 3990 mV, discharging.
- Picker states: 3990, 4200, 3720, 3500 and 3300 mV discharging; 3800, 4120, 4170, 4190 and 4200 mV charging; no battery. Also the evening tint (20:00) and the night tint while charging at 4200 mV.
- Child probes on the 4-profile picker:
  - a tap on the icon, a tap on the percentage, a double tap;
  - holds of 1 s, 3 s, 4.8 s and 5.5 s;
  - a drag from the icon onto a row, and from a row onto the icon;
  - a tap on the icon and then a row 150 ms later.
- Child probes on the 1-profile picker: a tap on the icon, then a pick.
- Grown-up path: hold the title for 5.5 s, then Info -> Admin -> `>` to reach Admin 2/2. I changed the state live there: 3990, 4200, 3720 and 3300 mV, charging at 4120 and 4200 mV, no battery.
- Input scripts are in `docs/battery/scripts/a.txt`..`h.txt`. Run them with `./build/host/snap --script FILE`.
- Iteration history: one pass, with no game fixes between passes. The second build came from the implementer's own refactor, not from these findings. A replay showed identical output.

## Observations

### Child (6-7, reads a little, imprecise fingers)

- The icon reads as "the battery", like the one on a parent's phone: a green bar in an outline, next to a number. It has no panel, no shadow and no bright color. It looks nothing like the cream name rows or the green New button, so I don't expect a child to take it for a button. The rows and the faces stay the obvious targets (`picker-4-profiles.png`, `picker-states.png`).
- Mistakes that did nothing harmful:
  - A quick tap, a double tap, a 1 s hold and a 3 s hold on the icon or the number left the picker as it was. Nothing flashed (sound not checked: the sim's buzzer was not observed).
  - Both drags opened nothing.
  - Tapping the icon and then a row 150 ms later opened that kid as normal.
  - 4 profiles and 1 profile behaved the same.
  - Evidence: `picker-mistaps.png` and the regression `tests/playtests/c5_battery_taps.txt`.
- A 4.8 s hold did nothing. A 5.5 s hold opened the grown-ups' Info page, which has an orange back arrow. That behavior existed before this change. What is new is that the icon is a curious new thing sitting inside the hold area. A child who presses it "to see" and keeps holding lands in a dark page full of words. Recovery is one tap on the arrow. Evidence: `picker-mistaps.png`, last tile.
- Distraction is low. The child does not need the icon to pick themselves, and it adds no reading. With 4 profiles, the icon, the title and the first row sit close together but do not touch.
- Evening and night tints: the icon and digits stay readable, and the green fill stays visible (`picker-tints.png`).

### Grown-up

- On the picker, the battery shows at a glance. The digits are crisp at native scale, in the same 8x8 font and navy as the title.
- Charging and discharging differ on the picker only by the bolt. The bolt is thin and navy, and at 1x it looks a little like a slanted "f" (`picker-icon-zoom.png`).
- Admin 2/2: the battery line is the first line under the title, in the adult font, gray on navy (C6C3C6 on 182852). It is easy to read (`admin-line-native.png`). "charging" is spelled out, so charging is unmistakable there. Changing the state while the page was open updated the line live.
- With no reading, Admin 2/2 falls back to "Device settings", and the picker shows no icon. The title keeps its position, so nothing jumps.

## Findings (ranked)

No Blocker or Major findings.

### Minor

1. Charging bolt at the round-glass limit
   - **Screen**: picker (`picker`).
   - **Issue**: At the widest label ("100%" plus the bolt, charging at 4.19-4.20 V), the bolt's top-right tip reaches 79.1 logical px from the center, just past the 79 px limit the repo applies to text. Measured from pixels: the farthest navy pixel is at physical (323,18), and its outer corner is 237.4 px from (240,240). The limit is 237 px. At 90% charging the tip is at 77.8 px, and at 100% discharging the outline is at 76.5 px.
   - **Concern**: round edge.
   - **Why the audit misses it**: it logs only the label (`text 70 6 30 8`), not the outline or the bolt, so `ui-check` passes.
   - **Repro**: `docs/battery/scripts/c.txt` (`sys charging 4200` on the picker), then measure the navy (1D2B53) pixels in `build/host/bat_c_chg4200.bmp`.
   - **Screenshot**: `apps/porthole/docs/battery/picker-charging-full-zoom.png`.
   - **Impact**: about 1 physical px of the bolt's tip sits in the rim zone and may be under the bezel on a real panel. It is cosmetic, but it is the exact case the 79 px rule exists for.
   - **Fix options**: move the group down 1-2 logical px, or tighten the gap before the bolt.
2. Child's hold on the icon reaches the grown-ups' Info page
   - **Screen**: picker (`picker`).
   - **Issue**: The new icon invites presses and sits inside the 5 s Admin hold area. A child who keeps pressing reaches the grown-ups' Info page.
   - **Concern**: recovery / comprehension.
   - **Repro**: `docs/battery/scripts/e.txt` E4: `down 66 10`, `wait 5500`, `up`, which lands on `sys_info`.
   - **Screenshot**: `apps/porthole/docs/battery/picker-mistaps.png` (tile 4).
   - **Impact**: one tap on the back arrow recovers. An admin code would probably close this path, but I did not test that here (I take it from `AGENTS.md`).
   - **Fix option**: limit the hold area to the title text, away from the icon. Owner decision.
3. Admin line drops the percentage while charging
   - **Screen**: Admin 2/2 (`sys_admin`).
   - **Issue**: While charging, the line drops the percentage ("Battery charging, 4.12 V"), but the picker shows "90%" with the bolt for the same reading. The grown-ups' detail page says less than the kids' picker.
   - **Concern**: consistency.
   - **Repro**: `docs/battery/scripts/f.txt`: `sys charging 4120` on Admin 2/2.
   - **Screenshot**: `apps/porthole/docs/battery/admin-states.png` (row 2, tile 2), against `picker-states.png` (row 2, tile 2).
   - **Suggestion**: "Battery 90%, charging, 4.12 V".

### Nit

- **Bolt is hard to recognize.** It is thin, the same navy as the digits, and reads like an italic "f" at native scale. A filled bolt, or one in the fill's green, would show charging without reading. See `picker-icon-zoom.png`.
- **Picker and Admin numbers differ.** The picker rounds to 5 and Admin shows the exact value: 75% against 76% at 3.99 V, and 15% against 17% at 3.72 V. That is defensible, but a grown-up comparing the two may wonder why. See `picker-states.png` and `admin-states.png`.
- **No low-charge cue at 0-5%.** At 5% and 0% the outline is empty, so 0% looks the same as 5% and there is no red. The low cue (an orange sliver at 15%) disappears exactly when it matters most. A 1-px red sliver from 1-10% would fix it. See `picker-icon-zoom.png`.
- **Odd empty-battery text.** "Battery 0%, 3.30 V, ~0 min left" is an odd string; "Battery empty, charge now" would read better. This is probably unreachable on the board, which browns out first. See `admin-states.png` (row 2, tile 1).
- **"100%" with an unfilled sliver.** Charging at 4.19 V reads "100%", but the fill leaves a light sliver. See `picker-charging-full-zoom.png` (row 2).
- **Fill looks low against the label.** At 3.99 V the fill looks about 2/3 full while the label says 75%. This is quantization over an 11 px interior. See `picker-75-zoom.png`.

## Round glass and legibility

- Text boxes are all inside 79 px:
  - Picker label: `70 6 30 8` at the widest ("100%" while charging).
  - Admin line: `22 46 116 8` at the widest (the 68-minute string).
- The icon and bolt are not in the audit. The pixel measurement is in Minor 1.

## Gesture and interruption matrix

| Gesture | 4 profiles | 1 profile | Result |
| --- | --- | --- | --- |
| Tap on the icon or the number | yes | yes | nothing |
| Double tap | yes | gap | nothing |
| Hold 1 s, 3 s or 4.8 s | yes | gap | nothing |
| Hold 5.5 s | yes | gap | grown-ups' Info (existing behavior) |
| Drag from the icon onto a row, or from a row onto the icon | yes | gap | nothing |
| Tap the icon, then a row 150 ms later | yes | yes | opens that kid |
| State change while on the screen | yes (picker and Admin) | yes | live update |
| Evening or night tint | 2 profiles | - | readable |
| Reboot / process restart | only the `c4_battery` scenario (reboot to the picker) | gap | not probed by hand |
| Sound on a tap | gap | gap | not observable in this sim run |

## Automated suite

- I ran `make playtest` on the second build: 110 of 110 scenarios pass, with 0 UI FAIL and 0 UI WARN. I read the report in full: the summary table, the empty UI findings section and the list of contact sheets.
- `c4_battery`: 11 checks.
- `c5_battery_taps` is new from this pass (child mis-taps plus a `ui-check` of the widest charging label): 9 checks. The implementer has since included it in commit `3efe61a`.
- An earlier full run on `ae14e66` passed 109 of 109 with 0 FAIL and 0 WARN, before `c5` existed.

## Qualitative assessment

- **Goal and actions.** The child can still tell what to do: tap your face or name. The battery is recognized as a battery, not an action. After any probe, nothing changed on screen, which is correct for a non-control. The grown-up finds the battery in two places and can tell charging apart: by the word on Admin, and by the small bolt on the picker.
- **Decisions, feedback and pictures.** There are no decisions here. The picture (the fill) carries the meaning for a non-reader. The digits are for the grown-up.
- **Repetition, confusion, stopping.** The change adds no wait, no reading burden for the child and no repetition. The only confusion risk is the 5 s hold into Info (Minor 2). The child would stop at the picker exactly as before: by picking themselves.
- **Recovery.** Imprecise taps, double taps, drags and holds under 5 s are harmless. A 5 s hold costs one back tap. No progress is at stake on this screen.

## Judgment

**Ready**, with three Minor items for the owner:
- the bolt at 79.1 px at full charge while charging (recommended fix: shift it 1-2 px);
- the hold area that covers the icon;
- the charging line without a percentage.

None of them blocks a child from picking themselves or a grown-up from reading the battery. The playtester fixed nothing.

## Resolutions (implementer, 2026-11-28)

- Minor 1, the bolt at the glass limit: the picker's battery group moved down 1 px (y 7).
- Minor 2, a 5 s hold on the icon opens Info: unchanged. The title's Admin hold predates the icon, and the admin code still guards it.
- Minor 3, no percentage while charging on Admin: now "Battery 98%, charging, 4.19 V".
- Nits: the fill now uses the same rounded percentage as the label, so 100% is full, and 20% or less draws an orange outline as well. The bolt's shape, "~0 min left" (a sim-only canned value) and the picker-vs-Admin rounding (by design) are unchanged.
