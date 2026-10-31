# Codex implementation task — Porthole Racing

Implement the racing game in `dreamiurg/porthole` using this design package. Start from `design/porthole-racing`, or from main if the design PR has already merged. Follow the repository's worktree policy; leave the shared primary checkout unchanged. This file authorizes implementation only when supplied as the next session's user task; the present PR contains design and artwork only.

## Read first

1. Root `AGENTS.md`, `apps/porthole/AGENTS.md`, and `apps/porthole/docs/playtesting.md`.
2. This folder's `README.md`, `gameplay-brief.md`, `tracks.md`, `implementation-contract.md` and `assets/README.md`.
3. Open both `assets/reference/generated-image.png` and `assets/reference/level-showcase-upload.png` with an image viewer. Use the first board's bottom-row **Style 2**, panels **2A–2D**, and the uploaded showcase's top-row track scenes. Verify both hashes using `assets/SHA256SUMS`.

The user supplied the level/HUD showcase as a clipboard PNG; its exact identity with the historical `a_clean_concept_art_game_ui_level_showcase_poster.png` is unverified. `a_clean_concept_art_game_ui_moodboard_poster_in.png` remains unavailable. Do not fabricate it or claim the three-file recovery is complete. Use the two delivered boards as the visual baseline. Shield, Magnet and Mine shown on the showcase are out of scope; only Boost, Rocket and Jump ramp apply. Its HUD variants are comparisons, not new user-approved mechanics: the written brief remains authoritative, including lap visibility and the pause/exit path.

## Required result

A playable native Porthole game with four complete selectable circuits: **Desert Canyon, Industrial Ruins, Snow Track, Night Run**. Three laps, four racers and all numeric tuning in the brief are initial defaults; adjust with evidence while preserving the user's fixed requirements:

- Actual round 480×480 output; all glyphs and full touch targets inside the glass.
- Upright device rotation as a steering wheel, calibrated comfortable grip; clockwise steers right. Automatic acceleration.
- Exactly two drawn gameplay buttons: **Boost** and **Rocket**, each showing 0–3 stored charges; explicit road pickups, no charge meter.
- Visible, discoverable hold-to-pause on the lap chip; from pause the player can resume, recenter, restart, choose tracks or exit. No inaccessible screen.
- Simple lap (`1/3`) and place (`3rd`) HUD; compact rear-view car and clear passing space.
- Ramps and gentle jumps over continuous road. Contact, hazards and rocket hits cause recoverable speed loss, never death, lives, health, shield, destruction or game over.
- Every player can finish, even after rivals finish; correct stable ranking, results and clean replay.
- Same selected rugged arcade visual identity across all four distinctive themes; no generic substitute screen, poster-as-game, oversized player car or cluttered traffic.

## Build within the current platform

Use `apps/porthole/games/racing/` and the existing `App`/RGB565 runtime. Inspect current code before editing. Baseline `fedf62c` already has nine games and `MAX_APPS=9`; racing needs a narrowly scoped tenth-game launcher/capacity extension that preserves existing tile placement. Confirm that fact on the starting revision rather than assuming it still holds.

Reuse existing runtime conventions, not another game's private code or artwork. Keep original PNG references unchanged in docs. Author proper game assets with deterministic tooling; use current compressed-asset support where it helps. Respect physical-to-logical touch scaling, fresh-touch gates, double-buffer correctness, single-pointer input, muted/quiet feedback, shell time controls and all save-compatibility rules.

Keep this first game version self-contained and stateless between runs. No unlock/economy/online/progression infrastructure. Do not rewrite existing games or broaden the platform unnecessarily. Do not raise static internal RAM (128 KiB), flash slot or OTA safety guards to accommodate new assets. Measure rendering and memory; 30 FPS is a target, not a proven claim.

## Work sequence and evidence

1. Implement a complete Desert Canyon run with steering, road projection, rivals, lap/place, pickups, both actions, jumps, results, pause/replay/exit. Verify the complete loop before expanding scenery.
2. Add all other authored tracks on the same mechanics, with the distinctive road/hazard patterns in `tracks.md`. Do not stop at a one-track prototype.
3. Test outcome invariants and edge cases listed in `implementation-contract.md`; keep tests focused at the correct layer. Run `make check`, `make ci`, and the firmware build, fixing failures and warnings.
4. Perform the required final-build black-box emulator playthrough using ordinary visible controls, through full races on all four tracks, results, replay, pause/recenter and exit. Inspect screenshots and feel/pacing; fix findings and replay affected flows. Save evidence. Debug jumps, bots and staged screenshots do not count as this pass.
5. Measure hardware frame timing/memory and wheel comfort if a board is available. If unavailable, explicitly report that gap; do not treat emulator performance as hardware proof.
6. Run repository review, resolve real findings, and open a ready-for-review implementation PR with concise behavior, verification, screenshots and any remaining gaps. Do not merge or flash a device without the session's authorization.

Report the branch/PR, what is playable, test/review results, image provenance, final-build playthrough evidence and any unvalidated device assumptions. Use the current user request and repository rules to resolve scope; do not turn this handoff into new mandatory approval rounds for routine implementation choices.
