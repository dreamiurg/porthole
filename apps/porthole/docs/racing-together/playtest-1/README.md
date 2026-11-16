# Rally "Together" playtest (emulator roleplay, round 1)

Build: commit 40b39f0, `build/host/snap` sha256 32a802a3068696b9e13e29d0ef23df3cd6231b3e166dd29c7d6f93d780f6b50c.
Two `snap --serve --fresh` processes paired over UDP 127.0.0.1, driven in interleaved 66 ms ticks by `inputs/duo.py`;
each `inputs/sNN.py` replays one probe from a fresh start. Not a real-child, real-wifi or physical-tilt test. No sound.

Persona: siblings ~6 (non-reader) and ~8 (reads short words), imprecise taps, wobbling tilt, impatient.
Fixture: `profile Mia 7` on device A, `profile Kai 7` + `profile Leo 7` on B (different faces). Rally opened from the
launcher (All games, page 2, red car), not `app`. Disclosure: before play I read `tools/together.py` (button centres)
and `host/sim.cpp` (commands). `debug` and `dbg lastlap` were used only in supplementary probes (s15, s20).

## Findings

Blocker
1. Both kids tap Together: both sit on "Looking for a friend... Ask them to open Rally" forever (32 s observed). Only
   recovery is one pressing Back, which then shows the other's invite. s7.py simul; key_1_both_together_deadlock.png.
   Same deadlock after Not now then guest taps Together: s7.py notnow_then_both, 11_deadlock_30s_*.png.

Major
2. Not now is silent: host keeps "Looking for a friend..." indefinitely. s7.py notnow_long; key_2_notnow_silent.png.
3. Together button is 1 px above the top track cards: `make playtest` spacing WARN fails a0/a5/a6/a7 (99/103 pass),
   and a tap at (70,33) just under Together starts a solo Desert Canyon race. s23.py; key_3_together_spacing.png.
4. Reading carries the flow for the 6-year-old: Together, Join / Not now (identical text buttons), "II Hold",
   "+2 stars" are words only; a wrong guess on the invite leads into finding 2. 04_rally_open_A.png, 05_invite_B.png.

Minor
5. Guest lobby: "Mia picks the track" touches "Canyon"; lone Back button. 12_lobby_B.png.
6. "Leo went home" when Leo only pressed Back (lobby) or Tracks (results). 14_guest_back_A.png, 34_b_tracks_A.png.
7. Back on the grip screen ends the session for both (to track cards), not back to the lobby. 18_b_back_while_a_waits_*.png.
8. Pause: a tap on "II Hold" does nothing; with no holds left the label only greys and a hold gives no feedback; the
   pause menu never shows holds left. 24_hold3_A.png, 23_tap_pause_A.png.
9. Host leaving via pause > Tracks shows the guest "Lost Mia, keep racing!" and "Connection lost", same as a drop. 28_*.png.
10. Both kids' cars are the same red; the friend is only the floating face. 20_count_2_*.png.
11. Tied at the start both show 4th with the "behind" arrow. 22_same_place_*.png.
12. Invite never shows while the guest is in a solo grip/race and the host is not told. 36_*.png.
13. Together stays bright above the invite but does nothing. 05_invite_B.png.

## Worked well
Invite with face and name; live track mirroring to the guest; "Waiting for Leo..."; grip tolerant of +/-25 deg wobble;
friend's face flag, up/down place marker, visible tailwind streaks for the trailing kid; "Mia paused" / "Ready" banners;
results with both faces and times, "Leo racing..." while waiting, "+2 stars" for both; "Mia wants again!" then both in
the lobby; partner Home turns Again into a solo Replay; a killed device gives "Lost Leo, keep racing!" and
"Connection lost: no winner" (key_4_finish_flow.png). Natural 3-lap race ~110 s; second kid finished 12 s later.

## Gaps
Process restart of the dropped device, rockets between kids, simultaneous Go taps, real latency/packet loss, and real
steering (my tilt was crude: both kids finished 4th/5th behind all rivals, so challenge is not judged).

Acceptance: not met (1 Blocker, 3 Major, automated suite red). Needs fixes and a fresh replay.
