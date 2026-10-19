# Admin, play limits and clock: black-box playtest

Final outcome: the affected adult tasks and both natural one-minute limit runs passed in the emulator. One layout finding was fixed and replayed. No open findings remain from this pass.

Tested the working tree based on `b76db520aea131ccd010fd9796fb50817ead2b19` on 2026-10-19. Final `build/host/snap` SHA-256:

```text
0f0efa7fd47a007b4ebf139b69cffdf9f0e27fbec0c6067058dd54d30448d056
```

## Method and setup

Used `snap --serve`, inspecting native 480×480 rendered frames before choosing the next input. Entered the grown-up menu using the README's instruction to hold “Who's playing?” for five seconds. Read the new implementation only after finishing the visible-input pass. No debug queries, state probes, `app` shortcuts, wall-clock jumps, or game progression cheats were used during play.

The declared [fixture](fixture.txt) prepares two existing profiles: Sam, age 8, PIN 1234; Kai, age 6, PIN 5678. It prepares no game progress, limits or admin code. The emulator starts with its canned saved Wi-Fi network and fixed local clock. Ordinary elapsed-time stepping and touches drive the rest of the session. `power_cycle` in the trace terminates/restarts the emulator while keeping its save directory; the simulator's fixed startup clock is not a physical RTC persistence test.

[Input trace](input-trace.jsonl) records logical 160×160 touch coordinates and elapsed milliseconds. Each entry captures the named native PNG after its inputs. The first empty capture was followed by a render tick and overwritten with the initial picker frame. All other frames represent the inspected steps below.

## Paths and observations

| Task | Observed result | Native evidence |
| --- | --- | --- |
| Admin code | Canceling repeated entry kept the menu code off. Set and confirmed 2220; leaving the menu required it again. Wrong 1111 showed feedback; 2220 admitted access. | [cancel](06-code-cancelled.png), [saved](08-code-saved.png), [rejected](10-auth-rejected.png), [admitted](11-auth-success.png) |
| Selected player PIN | Selected Kai without entering his old PIN, canceled once, then confirmed reset. Sam remained protected; Kai later entered the launcher directly. After a process restart, the picker still showed Kai without a lock. | [confirmation](13-player-confirm.png), [reset](14-player-reset.png), [login](28-recovered-player-login.png), [persisted picker](29-power-cycle-picker.png) |
| Limits draft and save | Defaults were 6 session / 25 daily minutes. Holding session + changed 6→16; daily + changed 25→26. Back discarded both edits. A dragged-off press changed nothing; a normal + and Save retained 7 / 25. Settings survived restart. | [held draft](17-limits-edited.png), [canceled](18-limits-cancelled.png), [retained](20-limits-retained.png), [restart](31-limits-after-power-cycle.png) |
| Clock offset | + drafted UTC+00:15; Back canceled it. Saving +15 visibly shifted 16:00→16:15. The saved offset survived restart. | [draft](22-clock-offset-draft.png), [cancel](23-clock-offset-cancelled.png), [save](24-clock-offset-saved.png), [restart](30-clock-after-power-cycle.png) |
| Clock request and exit | Sync now showed “Syncing with wifi...” and removed its request button. Home remained usable, and reentry still showed the pending request. Exited to the picker and entered Kai's launcher. | [waiting](26-clock-waiting.png), [reentry](27-clock-reentered-pending.png), [launcher](28-recovered-player-login.png) |
| Natural session enforcement | Saved session 1 / daily 25. Chose Kai, opened Biscuit through its visible tile, accepted the displayed name, then let ordinary time pass with a petting touch before a minute of inactivity. After about one minute of total play, the shell showed “Back in 10 min.” | [setting](32-session-one-minute.png), [entry](33-visible-game-entry.png), [playing](35-session-thirty-seconds.png), [rest](36-session-limit-enforced.png) |
| Natural daily enforcement | Saved session 3 / daily 1, then explicitly confirmed Reset all limits. Reentered the same saved Biscuit through the launcher. Ordinary ticks and petting reached “Back tomorrow” after one minute. Selecting Kai again kept the cap; OK exited to the picker. | [setting](37-daily-one-minute.png), [reset confirmation](38-reset-before-daily-run.png), [playing](40-daily-thirty-seconds.png), [daily cap](41-daily-limit-enforced.png), [reentry](42-daily-reentry-still-capped.png), [exit](43-exit-picker.png) |

The shell wording, controls and feedback stayed readable within the round display. Every visited screen had a visible exit. Biscuit's saved name and increased happiness remained after the adult reset; game progress was kept.

## Finding, fix and replay

**Minor — Admin 2/2 subtitle overlapped the previous-page arrow.** Opening Admin, then its right arrow, placed “Time and play limits” through the lower-right edge of the left arrow. [Before](issue-before-arrow-overlap.png).

The owner moved both Admin arrows upward to y=26 with 18-pixel adult targets. Replayed the final build, inspected both page directions and the complete affected limits/clock path. The subtitle now clears the arrow. [After](15-admin-page-two.png). No additional findings arose during final enforcement or exit checks.

The complete recorded input trace was replayed from the fresh declared fixture in a new emulator process. All **49 distinct recorded native frames** reproduced exactly, including both natural limit outcomes and the final picker. To repeat that verification after `make snap`, from `apps/porthole/`:

```bash
python3 docs/admin-playtest/replay.py
```

After integrating the newer upstream display/update changes at `b76db520`, reran the original trace against the freshly rebuilt emulator: all original 43 native frames were unchanged. Then performed another live, inspected menu reentry on that binary: the admin-code prompt still gated access, both Admin pages remained readable, Play limits retained 3 / 1, Clock retained UTC+00:15, and Home returned safely to the picker. Added those visible inputs and frames to the same trace, then verified all 49 distinct frames in another fresh-process replay. Evidence: [auth reentry](44-latest-auth-reentry.png), [Admin](45-latest-admin-reentry.png), [page 2](46-latest-page-two.png), [limits](47-latest-limits-reentry.png), [clock](48-latest-clock-reentry.png), [exit](49-latest-exit.png). No new findings.

The replay helper requires Pillow, uses a temporary save directory, and compares rendered pixels with the recorded PNGs. Replay is supporting reproducibility evidence; the live, stepwise inspected session supplied the black-box judgement.

## Separate automated fixtures

The canned emulator network does not complete requests by itself. The black-box session therefore demonstrates pending sync, safe navigation and exit, not an actual network success. Explicit `sys timeok`, `timefail`, `nowifi`, scan and join fixtures below exercise those additional UI states separately.

Targeted run:

```bash
python3 tools/playtest.py 25_admin_menu 25_home 26_player_pin_reset 27_play_limits 28_clock
```

All five scenarios passed: **346 checks, 0 failures, 0 UI FAIL / 0 WARN**. Read the full report and visually inspected the limits and clock contact sheets, including 1440-minute values, sync failure/success/retry, final “No wifi connection” wording and local-midnight correction.

- `25_admin_menu`: code setup, mismatch, authentication, changes/removal, resets and saved games.
- `25_home`: existing Home setup at its new Info-screen position.
- `26_player_pin_reset`: empty/full profile lists, cancellation, duplicate/drag/hold safety, selected PIN recovery and other PINs preserved.
- `27_play_limits`: draft cancellation, hold increments, settings persistence, single-player session enforcement, usage retained when lowering/raising daily caps, an existing rest retained when raising a cap, reset retaining configured settings, and 1–1440-minute bounds.
- `28_clock`: offset draft/save/cancel/negative/reboot, pending-request duplication prevention, update-radio serialization, failed/invalid reply/retry/success, no Wi-Fi and reconnection through setup, and a successful local-midnight correction lifting the previous day's cap.

Physical touch hardware, RTC durability and a real Wi-Fi/NTP exchange were not exercised. Firmware and full CI verification belong to the implementation owner's checks.
