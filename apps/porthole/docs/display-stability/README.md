# Shared display stability: final-build emulator pass

Tested `b5a623d87aa27dff945bb0f7a3c0cd425df76ea3` on 2026-10-19 with the
current native `build/host/snap --fresh --serve` build. This is a narrow
presentation check: controls, progression and artwork were not changed.

## Setup and evidence

Fresh device, profile A, first face, age 8, no PIN. Created entirely through the
visible setup screens; no save fixture. Entered both games through the launcher.
Inputs were ordinary `down`/`move`/`up`, `tick` and `tilt` only. No game source,
hidden state, debug command, app shortcut, stage jump or state-aware bot was used
to choose actions. `tick` advanced simulated time normally; no clock jumps.

[input-trace.txt](input-trace.txt) records every input and observation in order.
Coordinates are logical 160x160 pixels; screenshots are native 480x480 RGB.
`snapshot NAME` means capture the visible frame to `NAME.png`; it does not alter
the game. The trace documents this observed session; a fresh-process replay was
not separately performed. Screenshots were inspected as captured, not staged.

## Paths and outcomes

- Pumpkin: launcher page two → title/instruction → comfortable-and-still
  calibration → play. [Title](07-pumpkin-intro.png),
  [calibration](08-pumpkin-instructions.png), [play](09-pumpkin-play.png).
  Ordinary tilt visibly moved the pumpkin right and left while furniture and
  background stayed aligned: [right](10-pumpkin-roll-right.png),
  [left](11-pumpkin-roll-left.png). Waiting naturally produced Grandma's
  peekaboo and [loss](13-pumpkin-loss.png). Try Again returned to
  [play](14-pumpkin-replay.png). Exercised a mis-tap, drag off Hide, held Hide,
  release, and more tilt. Visible [Find Cover](19-pumpkin-hide-at-furniture.png),
  [Quick! Hide!](21-pumpkin-outcome.png), [Found You](22-pumpkin-hidden-or-loss.png)
  and another [natural loss](23-pumpkin-after.png) remained legible.
  Top back returned to the [launcher](24-exit-pumpkin.png).
- Marble: previous launcher page → [title](25-marble-intro.png) → visible Play
  control → [play](26-marble-ready.png). Inspected the ball and fixed field
  geometry after successive ordinary tilts: [right](27-marble-tilt-right.png),
  [left](28-marble-tilt-left.png), [play](29-marble-play.png). A drag off Home
  kept play active. Holding Home returned to [launcher](32-marble-held-home.png).
  Reentered through the launcher, inspected title again, and exited with a
  centered Home press to [launcher](36-marble-exit-retry.png).

Text, scene scale, colors, round-screen composition, moving sprites and recovery
screens showed no new presentation defect in these observations. No crash,
corrupted frame, clipped new text or dead end was observed. This narrow pass did
not complete a Pumpkin win or Marble level.

## Findings

No actionable presentation finding or dead end.

**Resolved observation — Marble Home gestures.** During the source-free pass,
a 66ms press at `(80,146)` stayed in play
([31-marble-home.png](31-marble-home.png)); a 1200ms hold at the same point exited
play. On reentry, a 1200ms hold on the title/calibration screen stayed there
([35-marble-exit.png](35-marble-exit.png)), while a 300ms press at `(80,143)` exited
([36-marble-exit-retry.png](36-marble-exit-retry.png)). Exact attempts remain in
the unchanged input trace after screenshots 30 and 34.

Post-playthrough source triage by the implementer and independent reviewer
confirmed expected behavior: Home requires a hold during play, and a tap on
calibration. A hold of at least 600ms suppresses the tap event. Both tested
coordinates lie inside the same Home hit circle, so this is not evidence of an
edge defect. This triage happened after the black-box session and did not guide
its inputs. The reviewer approved the evidence; no game code change was needed.

## Limits and automated checks

The emulator cannot exercise ESP32 RGB DMA, PSRAM contention, interrupt timing or
the firmware-only SRAM update scanout. No physical board was connected. Physical
left/right scanout stability and OTA percentage/Cancel behavior remain
unverified; a clean emulator frame is not evidence those hardware symptoms have
disappeared. Still frames and stepped observations do not measure frame pacing.

Supplementary report read in full: `build/playtest/report.md`, generated at
`b594d4f-dirty` before this final rebased pass: **54/54 scenarios passed,
0 UI FAIL, 0 UI WARN**. The parent is rerunning the final CI/playtest suite for
the release. That automated report is separate from the black-box pass above.
