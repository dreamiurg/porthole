# Family controls: black-box playtest

The consolidated Screen time and timezone tasks passed the observed emulator flow. Natural play reached a one-minute session break, the configured play-window cutoff, a one-minute daily cap and a real thirty-minute override expiry. Fresh-player usage accumulated during the override. Manual/automatic timezone selection, saved fallback and Pacific daylight-saving transitions behaved as expected. No open findings remain from this pass.

Final working-tree base: `10bb122dabdcb1523269a7321c541e801d00a4e2`, tested 2026-10-19. Immutable tested emulator, kept privately at `build/family-controls/final-snap`:

```text
SHA-256 e0b595d2a5ab6ed6a1a74cf229835baf1cafbe8e17e39705fd540a607acfdc1f
```

## Evidence and privacy

No new PNG is included in this public evidence directory, Git publication or report. Native 480×480 screenshots remain in ignored `build/family-controls/`; final replay captures are in its `replay/` subdirectory. The binary also remains private and ignored. Public evidence consists of this report, declared text fixtures, input traces, a **list-format** [RGB hash manifest](frame-manifest.json) and [replay_family_controls.py](replay_family_controls.py). The manifest contains 74 uniquely named frames, their dimensions, RGB SHA-256 hashes and final build hash.

Every profile name, PIN, pet, Wi-Fi name, detected location and network response used here is synthetic. No real IP lookup, network scan, private credential or personal location was used in QA.

## Method

Read the README and required playtesting instructions before the pass. Did not inspect changed implementation source to choose actions. Used a live `snap --serve` process, inspected each native frame before choosing the next visible input, and recorded ordinary touch/down/move/up and elapsed-time ticks. No hidden-state queries, `dbg`, `app` shortcuts, state-aware bots or wall-clock jumps were used in the ordinary pass.

The declared [ordinary fixture](fixture.txt) supplies Sam with an adopted pet solely for game reachability, plus a second synthetic player Kai with PIN 5678. Gameplay was entered through the visible picker and launcher. All settings and grants were changed through their displayed controls. Regular touches during ordinary elapsed-time stepping kept play active; no time jump substituted for the session, daily, window or override outcomes.

The [ordinary trace](visible-input-trace.jsonl) records 54 inspected frames. A separate [clock fixture](clock-fixture.txt) and [supplemental trace](clock-input-trace.jsonl) record 20 additional inspected frames using explicitly annotated canned network results and synthetic UTC dates. Those supplemental clock corrections are not claimed as natural play or live network success.

## Ordinary visible paths and outcomes

Frame names below refer to private `build/family-controls/replay/<frame>.png` files. Their public hashes are in the manifest.

| Task | Observed outcome | Frame names |
| --- | --- | --- |
| Entry and defaults | Picker-title hold → Info → Admin → Screen time. All controls are together: session 15 min, daily total 30 min, enabled 2 PM–7:30 PM window. Text and touch controls fit the round display. | `01-picker`, `04-screen-time` |
| Session edit/cancel/save | Holding + drafted 25 from 15; Cancel kept the original value. Then saved 1 minute through the editor. | `05-session-editor`, `06-session-draft`, `08-session-one-saved` |
| Natural session enforcement | Opened the existing pet through the launcher, played with ordinary elapsed time and touches, and reached “Back in 10 min” after about one minute. | `09-launcher-one-minute`, `11-game-active`, `12-session-limit-enforced` |
| Reset confirmation and preservation | Canceled once, dragged off the confirmation once, then explicitly reset usage. Screen time retained session 1, daily 30 and the configured window. Existing player PIN/game progress remained usable. | `13-reset-usage-confirm`, `14-reset-drag-cancel`, `15-reset-preserves-settings` |
| Window editing and cutoff | Canceled a disabled-window draft, observed fifteen-minute time steps, then saved 2 PM–4:15 PM and session 15. Continued an already running game with regular touches across 4:15 PM. The shell closed it and denied reentry until the next opening. Neither session nor daily cap had preempted this cutoff. | `16-play-window-editor`, `18-cutoff-settings-saved`, `20-game-five-minutes`, `21-game-ten-minutes`, `22-running-game-window-cutoff`, `23-window-denies-reentry` |
| Natural daily enforcement | Temporarily disabled the window, saved session 3/daily 1 and explicitly reset usage. Ordinary play reached “Back tomorrow” after one minute. | `24-daily-one-window-off`, `25-natural-daily-game`, `26-daily-limit-enforced` |
| Confirmed global allowance | Reenabled the already closed window. Canceled Allow 30 min once, then confirmed it. Sam entered despite his existing daily cap and the closed window, and kept playing beyond the three-minute session limit. The same grant admitted Kai after his normal PIN. | `27-allow-thirty-confirm`, `28-override-active`, `29-override-bypasses-daily-and-window`, `30-override-past-session-cap`, `31-second-player-pin`, `33-override-global-second-player` |
| New usage during allowance | Kai had played zero before the grant. He played Biscuit for over two minutes while his daily allowance was one minute. After expiry, disabling only the window and entering his PIN produced “Back tomorrow.” This distinguishes newly counted override play from Sam's preexisting cap. | `34-new-usage-under-override`, `40-window-off-proves-new-daily-usage`, `41-override-counted-fresh-player-usage` |
| Actual expiry, early end and restart | Kept active play running through the thirty-minute deadline; normal restrictions returned. Started/ended another grant through End 30 min, then started another and restarted the emulator. The restarted player was capped, while Screen time retained session/daily/window settings and showed Allow 30 min again. | `36-override-still-playing`, `37-override-near-expiry`, `38-override-expired`, `42-override-ended-early`, `43-override-before-restart`, `44-restart-clears-override`, `45-restart-keeps-settings-no-override` |
| Named zone/manual mode | Clock initially showed Pacific/Automatic on. Chose Mountain: displayed time advanced one hour and automatic mode became off. Searched for Tokyo using the visible keyboard and selected its result: the displayed date/time changed to the next morning. Inspected both common-zone pages, restored Pacific, enabled automatic mode, started Detect and exited safely while it waited. | `46-clock-default-zone`, `47-zone-picker`, `48-manual-mountain`, `49-zone-search-keyboard`, `50-zone-search-results`, `51-manual-tokyo`, `52-common-zone-page-two`, `53-automatic-detect-pending`, `54-ordinary-pass-exit` |

The natural pass covered picker/login, launcher and running-game gates, plus adult recovery and exit. It did not separately force every child sub-screen to cross a cutoff; exhaustive sub-screen checks belong to the implementation's regression suite.

## Explicitly canned timezone/network coverage

These cases were inspected after the ordinary pass, with every canned operation annotated in the supplemental trace:

- Detect pending → failed IP result retained Pacific and displayed saved-zone fallback. A later successful canned NTP connection started the automatic retry; a canned London result applied and survived restart (`clock-01` through `clock-07`).
- Changed the synthetic Wi-Fi network while Detect was pending. The old-network Tokyo result was discarded; the UI stayed on London with a new request pending. The new-network New York result then applied (`clock-08-old-network-result-discarded`, `clock-09-new-network-result-applies`).
- Started Detect, selected Pacific manually, then delivered a late Tokyo result. Pacific stayed selected and automatic mode remained off (`clock-10-manual-choice-wins`).
- Set synthetic UTC dates while manually selected Pacific remained saved. Native displayed results matched the expected DST rules:

| UTC fixture | Expected and observed Pacific display | Private frame |
| --- | --- | --- |
| 2026-01-15 12:00 | 4:00 AM, winter | `clock-11-pacific-winter` |
| 2026-07-15 12:00 | 5:00 AM, summer | `clock-12-pacific-summer` |
| 2026-03-08 09:59:59 | 1:59 AM | `clock-13-before-spring-forward` |
| 2026-03-08 10:00 | 3:00 AM | `clock-14-after-spring-forward` |
| 2026-11-01 08:59:59 | 1:59 AM | `clock-15-before-fall-back` |
| 2026-11-01 09:00 | 1:00 AM | `clock-16-after-fall-back` |

Also edited/saved a 10 PM–6 AM overnight window through the UI. At the already declared synthetic 1 AM time, the player entered the launcher; the overnight interval wrapped correctly (`clock-18-overnight-window-draft`, `clock-19-overnight-window-allows`). Exited to the picker afterwards.

## Final replay and findings

No new actionable layout or flow finding arose during the independent pass. Implementation review corrected combined rest/window release wording during the run. Final expiry/reentry frames now say **“Back tomorrow”** when daily usage is exhausted, rather than promising an earlier window opening. Both corrected frames (`38`, `39`) were inspected on the immutable final replay.

Replayed both complete traces from fresh isolated fixtures against the frozen final binary. Nine Pets Club images changed only in puppy/heart animation phase after the simulator's fixture-clock initialization correction; inspected those regenerated native frames as well. All adult screens, policies and outcomes remained correct. Recorded the final manifest, then independently replayed both phases again: **all 74 native RGB hashes matched**.

From `apps/porthole/`, after building the corresponding revision:

```bash
python3 docs/family-controls/replay_family_controls.py --snap build/host/snap
```

The helper uses temporary save directories and writes captures only under ignored `build/family-controls/replay/`. If a private frozen binary is present, it is the default. Pillow is required. The JSON manifest is a list, with unique frame names across phases.

Cached Ruff checks/format and whole-repository mypy passed for the evidence helper. Automated UI checks were not run before the ordinary inspection; root owns their results, full CI and firmware verification. Physical RTC durability, physical touch accuracy, real IP geolocation and real network exchanges remain untested by this emulator report.
