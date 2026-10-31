# Adult typography: native-frame playtest

The grown-up screens are noticeably easier to read at the device's native 480×480 size. Body text, headings, keypad digits, keyboard labels and feedback now have smooth edges and clear letter shapes. One held-key preview clipping issue was found, fixed and replayed. No open findings remain from this pass.

Tested 2026-10-19 on the working tree based on `27aa687283fff4f31ecb1ced2f84715cf827e665`. Final `build/host/snap` SHA-256:

```text
369bf5d59714337723940802992d8f7910854be31ea5a5f63da53bfcf3ff5fb2
```

Native screenshots and before/after images are retained in a private validation bundle, rather than published in the source repository. Frame names below identify those local captures. The committed input trace and `frame-hashes.json` reproduce and verify the final images without publishing screenshots.

## Setup and method

All profile names/PINs, Wi-Fi network names, typed passwords, home locations and entered addresses in this evidence are **synthetic test fixtures**. No real network scan, personal location or private credential was used. The Wi-Fi list and public-place lookup result are the emulator's canned data; the entered password and address are disposable test text.

Read the README entry instructions and the playtesting procedure. Did not read the typography implementation to choose inputs. Captured the old Info (`baseline-info.png`), old Admin (`baseline-admin.png`), old Clock (`baseline-clock.png`) and a separately fixture-assisted old password keyboard (`baseline-wifi-keyboard.png`) before the change. Baseline binary SHA-256 was `0f0efa7fd47a007b4ebf139b69cffdf9f0e27fbec0c6067058dd54d30448d056`.

For the final pass, used the headless emulator with an incrementally extended input script. After each new visible input sequence, inspected the current native 480×480 frame before deciding the next action. Each extension replayed the previous inputs from a fresh declared fixture; it did not query hidden state or use source-derived solutions. This allowed deliberate typing, corrections, presses, drags, cancellation and recovery with exact reproducibility.

The [final fixture](final-fixture.txt) supplies four existing profiles and no settings or game progress. All further navigation used visible controls. The [complete input trace](final-input-trace.txt) includes every touch/wait and snapshot. Explicit `sys` network completions are marked with comments and described separately below. No debug queries, screen-name probes, game shortcuts or wall-clock jumps were used. No game run was needed for this typography-only change.

## Paths inspected at native size

| Path | Observations | Evidence |
| --- | --- | --- |
| Info and Admin | Labels/values align, the long synthetic Wi-Fi name truncates inside the glass, and all navigation text is smooth. Both page arrows clear their subtitles. | Info (`final-02-info.png`), Admin 1 (`final-03-admin.png`), Admin 2 (`final-43-admin-page-two.png`) |
| Code setup and authentication | Inspected choose/repeat prompts, two masked digits, mismatch, cancellation, saved feedback, authentication and wrong-code recovery. Digits remain distinct, including zero. | choose (`final-04-code-choose.png`), mismatch (`final-06-code-mismatch.png`), masked (`final-07-code-masked.png`), saved (`final-08-code-saved.png`), rejected (`final-10-auth-rejected.png`) |
| Reset and player PIN recovery | Explanatory lines fit with clear spacing. The full four-player list and long synthetic name remain readable. Back and dragged-off confirmation preserve the screen; explicit reset and a no-PIN tap show legible feedback. | reset confirmation (`final-11-reset-confirm.png`), players (`final-12-players.png`), long-name confirmation (`final-38-player-confirm.png`), drag canceled (`final-39-player-drag-cancel.png`), reset feedback (`final-40-player-reset-toast.png`), no PIN (`final-41-player-no-pin-toast.png`) |
| Code removal | Cancel returned safely; confirmation and successful removal feedback fit. | confirmation (`final-42-remove-confirm.png`), removed (`final-52-code-removed-toast.png`) |
| Wi-Fi entry | Inspected scanning, list, password masking, text/numeric/symbol pages, held first-row edge keys, a lower-row key, dragged selection, retry typing and result screens. Labels remain readable in the narrow keys. | scanning (`final-13-wifi-scanning.png`), list (`final-14-wifi-networks.png`), keyboard (`final-15-wifi-keyboard.png`), numbers (`final-18-wifi-numbers.png`), symbols (`final-20-wifi-symbols.png`), bottom preview (`final-23-bottom-key-preview.png`), join failure (`final-25-wifi-failed.png`), connected (`final-27-wifi-connected.png`) |
| Home address | Inspected the existing multiline place, empty input hint, typed address, correction using backspace, lookup waiting, miss/retry, found confirmation and cancellation back through the keyboard/Home/Info. Long result lines fit. | Home (`final-28-home.png`), keyboard (`final-29-address-keyboard.png`), corrected input (`final-33-address-corrected.png`), lookup miss (`final-32-address-not-found.png`), result (`final-34-address-found.png`) |
| Updates | Channel buttons, values and checking/failure feedback are smooth and readable. Back returned to Info. | idle (`final-35-updates.png`), checking (`final-36-update-checking.png`), failure (`final-37-update-failed.png`) |
| Limits | Titles, units, values, +/- and hold hint fit. Held + changed 6→16; Back discarded the draft. A new tap and Save showed clear feedback. Automated boundary coverage also shows 1440-minute values fitting. | defaults (`final-44-limits.png`), held draft (`final-45-limits-draft.png`), saved (`final-46-limits-saved.png`) |
| Clock and exit | Date, status, relative time and offset fit. Saved +15, started sync, left/reentered while pending, retried after a canned failure and inspected success. Home navigation returned to the normal picker. | Clock (`final-47-clock.png`), offset feedback (`final-48-clock-saved.png`), pending (`final-49-clock-syncing.png`), failure (`final-50-clock-failed.png`), success (`final-51-clock-synced.png`), final exit (`final-53-exit-picker.png`) |

Compared native frames rather than relying on reduced contact sheets. The new letters are materially clearer than the old compact bitmap font. Text keeps useful contrast, and toast panels contain their complete messages. The ordinary child picker remains in its existing pixel style.

## Finding, fix and replay

**Minor — held Q/P preview clipped by the round bezel.** On the secured-network keyboard, pressed the leftmost Q at `(17,57)` and the rightmost P at `(143,57)` for 200 ms. The preview followed the key center into the edge of the circle, cutting both panel and glyph. Evidence: Q before (`issue-before-q-preview.png`), P before (`issue-before-p-preview.png`).

The owner clamped preview centers inward while preserving the source key's hit target and highlight. Repeated both presses on the final build: the complete bubbles and glyphs now fit, with the intended key still highlighted. Inspected lower-row and numeric previews as well. Evidence: Q fixed (`final-21-q-preview-fixed.png`), P fixed (`final-22-p-preview-fixed.png`), lower-row preview (`final-23-bottom-key-preview.png`). A focused Q/P hold audit was added by the implementation owner to `24_wifi_setup`.

After completing the observed final traversal, replayed the entire input trace in a fresh process against the final binary. All **51 final native frames** reproduced exactly. The two preserved pre-fix issue images are historical evidence and are excluded from final pixel comparison. The raw trace retains their input positions; replay renders those inputs using the fix without overwriting the historical PNGs.

To repeat the pixel comparison after building the same emulator, from `apps/porthole/`:

```bash
python3 docs/adult-typography/replay_typography.py
```

The helper requires Pillow and uses a temporary save directory. It checks the SHA-256 of every rendered RGB frame against `frame-hashes.json`, so the private screenshots are not required to run it. This reproducibility check supplements the stepwise inspected pass.

## Separate network fixtures and automated verification

The emulator does not perform real network operations. Ordinary visible touches covered waiting states and navigation. The explicitly marked canned `sys scan`, `joinfail`, `joinok`, `notfound`, `found`, `failed`, `timefail` and `timeok` commands supplied network outcomes for additional visual coverage; they do not establish real Wi-Fi, geocoding, update or NTP behavior.

Only after the actual final-build inspection, ran:

```bash
python3 tools/playtest.py 19_system_menu 24_wifi_setup 25_home 25_admin_menu 26_player_pin_reset 27_play_limits 28_clock
```

All **7 scenarios passed: 470 checks, 0 failures, 0 UI FAIL / 0 WARN**. Read the entire audit report and inspected the relevant contact sheets, including the new Q/P hold regression, maximum minute values, empty/full player lists, feedback and no-Wi-Fi recovery.

Physical panel appearance and touch accuracy were not tested. Full CI and firmware verification belong to the implementation owner's checks.
