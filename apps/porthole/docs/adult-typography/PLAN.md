# Adult typography QA plan

This plan was recorded before final-build play. The completed source-free traversal, one fixed finding, replay and subsequent automated results are documented in [README.md](README.md).

Baseline emulator SHA-256: `0f0efa7fd47a007b4ebf139b69cffdf9f0e27fbec0c6067058dd54d30448d056`. Preserved binary: `/tmp/porthole-before-font-snap`. Live baseline input and native captures are `baseline-input-trace.jsonl`, `baseline-info.png`, `baseline-admin.png`, and `baseline-clock.png`. `baseline-fixture.txt` declares the two existing profiles used to reach the menu.

The password-keyboard baseline uses a separate, explicitly canned scan completion (`baseline-keyboard-fixture.txt`) and prepares no real network connection. Its native frame is `baseline-wifi-keyboard.png`.

## Final visible-input pass

1. Start the final emulator with the declared existing-profile fixture, inspect the picker, and use the README's five-second title hold. Inspect Info at native 480×480: label/value alignment, long Wi-Fi name truncation, readable weights and contrast, all four navigation buttons, round edges and home exit.
2. Traverse Admin pages in both directions. Choose a code, cancel repeated entry, retry with mismatched confirmation, then set it. Reenter the menu, try an incorrect code and recover with the correct code. Inspect masked digits, keypad labels, feedback, and code removal/reset confirmations. Exercise selected-player PIN recovery, cancel/back and success feedback; verify the named player remains clear in the confirmation.
3. Inspect limits values/units, +/- labels, held controls, canceled drafts, save feedback, and reentry. Inspect Clock date/status/last-sync/offset and saving/canceling a draft. Start a sync and use the visible way back while it waits. Settings-only typography changes do not require another natural game run.
4. Inspect Updates labels, channel buttons, status and pending/busy wording. Inspect Wi-Fi scanning and its exit. Separately declared canned network completions will enable the network list, password keyboard and join-result states for typography inspection; these are fixture-assisted visual coverage, not real network success.
5. Traverse Home and its address keyboard using visible labels. Inspect the address field, typed key preview, editing, keyboard pages, lookup waiting, cancel/back and exit. Explicit canned lookup completion may supplement failure/success typography states.
6. Include mis-taps, a carried second tap, a dragged-off press, canceled holds, repeated entries and all relevant exits. At every step inspect native frames rather than judging contact sheets alone. Pay particular attention to narrow keys, long explanations, selected-player names, values, toast widths and smooth native text over the shell's indexed art.
7. Report prioritized findings with exact inputs and native screenshots. Ask the owner to fix actionable issues; replay affected paths on the rebuilt final emulator. Keep before/after evidence and record the final binary hash.

## Separate automated verification

Only after completing final-build visible-input play, run the affected system/admin/PIN/limits/clock/Wi-Fi/Home fixtures. Read the entire audit report and inspect contact sheets; any UI FAIL or WARN remains a finding. State clearly which network outcomes require canned completions and which were observed through ordinary visible input. Preserve a replayable trace and a concise final report in this directory.
