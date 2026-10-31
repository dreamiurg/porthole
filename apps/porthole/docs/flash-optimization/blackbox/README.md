# Observed rendering check and pixel comparison

2026-10-25, optimized source based on `5653dfd`. No physical hardware used.
The coordinator operated an isolated `tools/webemu.py` / `snap --serve` instance
through its normal touch endpoint, inspected rendered 480x480 frames and chose
ordinary visible controls. This is a non-player-visible, lossless optimization;
no game rules, content, control layout or saves were changed.

The setup fixture created only a fresh shell profile, Sam, age 8. No `app`, `dbg`,
hidden-state query, source-derived solution, stage jump or bot chose actions in
the observed session. One off-target tap on Kingdom's title was retained in the
trace; its visible Begin button was then used normally.

Observed paths:

- Picker → launcher → Biscuit naming; accepted the existing suggested name.
- Biscuit home animation and Feed; the pup's response and needs updated normally.
- Read → Discoveries → thunder/lightning cover, reading pages and wonder page →
  Keep → notebook/library → home. Both cover illustrations and full scenes were
  inspected, with no corrupt rows, colors, stale backgrounds or visible delay.
- Launcher page three → Kingdom Begin. Read all eight opening pages, chose the
  mill visit, read the two-page result and arrived at the mill scene.
- Exited Kingdom normally and reentered through Continue. The pending mill page
  returned. No visual corruption or unexpected navigation was observed.

Evidence:

- [input.jsonl](input.jsonl): live touch decisions and screenshot markers.
- [playthrough.txt](playthrough.txt): profile fixture plus the observed inputs,
  converted to deterministic simulator timing for reproducible comparisons.
- PNGs `01`–`29`: native live captures. Real-time animation/touch polling differs
  slightly from deterministic replay timing; these remain the observed captures.
- [frame-comparison.json](frame-comparison.json): raw RGB SHA-256 for each replay
  frame, before and after optimization. **All 29 frames match exactly.**
- [comparison.png](comparison.png): representative before/after replay frames.

For the comparison, the original app source at `5653dfd` was archived and compiled
separately. Both binaries ran the same trace in separate fresh directories,
without sharing save files. Each directory contains an empty `build/host/` before
running `snap --fresh --script <absolute trace path>`. The resulting BMPs were
converted to RGB for comparison at their full 480x480 dimensions.

This representative pass supplements the exhaustive immutable 600-reference
Biscuit pixel oracle, Kingdom source-image and malformed-stream tests, alternating
framebuffer tests, and the full 74-scenario playtest suite. It does not claim a
new complete Kingdom chapter playthrough or physical ESP32 latency measurement.

No actionable visual or gameplay regression was found. Remaining uncertainty is
physical decoder timing; the repeatable host benchmark is recorded separately in
[the optimization report](../README.md).
