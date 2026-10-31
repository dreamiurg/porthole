# Family controls QA plan

This plan was recorded before the final UI-ready signal. The completed source-free ordinary pass, separate canned coverage and final immutable replay are documented in [README.md](README.md).

All new native screenshots remain private under ignored `build/family-controls/`. Public evidence will contain this report/plan, declared fixtures, ordinary-input traces, a list of RGB frame hashes, and a uniquely named `replay_family_controls.py`. No new PNG will be published. All names, codes, network data and addresses are synthetic emulator fixtures.

## Visible-input pass

- Enter through the README's picker-title hold and observed navigation. Inspect the consolidated Screen time home, session/daily defaults of 15/30 minutes, enabled 14:00–19:30 play window, clear labels and visible exits.
- Edit limits/window through the UI, cancel and retry, save and reenter. Exercise mis-taps, duplicate/carried taps, dragged-off confirmations and canceled holds. Reset usage only after explicit confirmation, then verify that configured settings remain.
- Configure a one-minute limit through visible controls, open a game through its launcher tile, and play with ordinary elapsed time and touches to observe enforcement. Use a documented existing-pet fixture only for reachability. Exercise denial/recovery on launcher/profile/settings paths and during an already running game.
- Set a nearby play-window cutoff through visible controls and cross it using ordinary elapsed time. Confirm that the running game closes to a clear blocked state. Do not jump the wall clock or force gameplay state during this pass.
- Cancel the global 30-minute override confirmation once, then confirm it. Verify that a previously blocked player can play and another profile also benefits. Keep playing/activity so usage continues. Cross expiry with ordinary elapsed time and verify the normal gates return. Start another override, restart the emulator while preserving saves, and verify that override access is cleared.
- Inspect Clock/timezone defaults and saved Pacific selection, named zones, manual selection and displayed offset. Check save/cancel/reentry/restart and the visible effect on window eligibility. Network-driven IP guess success/failure and recovery use separate explicitly declared canned completions; real network discovery is not claimed.
- Inspect native 480×480 frames step by step. Report actionable layout, wording, feedback, safety-of-confirmation or navigation findings; ask the owner to fix them and replay affected paths on the final build.

## Separate supplemental verification

After the actual final-build pass, use explicit synthetic UTC/date and IP-result fixtures for Pacific winter/summer DST boundaries, named/manual/automatic precedence, fallback after a failed guess, clock correction and all-screen gates where exhaustive natural traversal is impractical. Keep these distinct from ordinary-input evidence. Then run the affected automated UI fixtures, read every failure/audit result and inspect relevant contact sheets. Root owns full CI, firmware validation and code review.

Record final build revision/hash, fixtures, findings/fixes, native private paths, ordinary inputs, replay results and remaining emulator/physical-device limits. The public RGB manifest will be a list of records with unique frame names, dimensions and RGB SHA-256 hashes, allowing reproduction without publishing image files.
