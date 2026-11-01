# Pets Club playtest report

`python3 tools/playtest.py` at 79ca717, 2026-11-01 19:54 UTC: 84 of 84 scenarios pass. A scenario fails on a failed directive, a crash, a sanitizer report or any UI finding: FAIL and WARN both fail a run (warnings are errors).

| Scenario | Result | Checks | Failures | UI FAIL | UI WARN | Time |
| --- | --- | --- | --- | --- | --- | --- |
| 01_adopt | PASS | 36 | 0 | 0 | 0 | 0.1 s |
| 02_feed | PASS | 26 | 0 | 0 | 0 | 0.1 s |
| 03_fetch | PASS | 13 | 0 | 0 | 0 | 0.1 s |
| 04_words | PASS | 15 | 0 | 0 | 0 | 0.2 s |
| 05_read | PASS | 22 | 0 | 0 | 0 | 0.1 s |
| 06_trick | PASS | 18 | 0 | 0 | 0 | 0.3 s |
| 07_bath | PASS | 7 | 0 | 0 | 0 | 0.0 s |
| 08_sleep | PASS | 17 | 0 | 0 | 0 | 0.1 s |
| 09_days | PASS | 13 | 0 | 0 | 0 | 0.1 s |
| 10_profiles | PASS | 52 | 0 | 0 | 0 | 0.2 s |
| 11_stats | PASS | 14 | 0 | 0 | 0 | 0.1 s |
| 12_reset | PASS | 13 | 0 | 0 | 0 | 0.1 s |
| 13_profiles_delete | PASS | 37 | 0 | 0 | 0 | 0.1 s |
| 14_shell_edges | PASS | 50 | 0 | 0 | 0 | 1.0 s |
| 15_shell_taps | PASS | 24 | 0 | 0 | 0 | 0.1 s |
| 16_night | PASS | 17 | 0 | 0 | 0 | 0.1 s |
| 17_daily_cap | PASS | 17 | 0 | 0 | 0 | 0.2 s |
| 18_tilt_play | PASS | 9 | 0 | 0 | 0 | 0.4 s |
| 19_system_menu | PASS | 49 | 0 | 0 | 0 | 0.3 s |
| 20_monkey_home | PASS | 30 | 0 | 0 | 0 | 7.5 s |
| 21_monkey_creation | PASS | 17 | 0 | 0 | 0 | 8.7 s |
| 22_monkey_games | PASS | 30 | 0 | 0 | 0 | 7.2 s |
| 23_monkey_biscuit | PASS | 19 | 0 | 0 | 0 | 13.3 s |
| 24_brightness | PASS | 32 | 0 | 0 | 0 | 0.1 s |
| 24_wifi_setup | PASS | 75 | 0 | 0 | 0 | 0.4 s |
| 25_admin_menu | PASS | 74 | 0 | 0 | 0 | 0.4 s |
| 25_home | PASS | 35 | 0 | 0 | 0 | 0.1 s |
| 26_player_pin_reset | PASS | 57 | 0 | 0 | 0 | 0.4 s |
| 27_play_limits | PASS | 88 | 0 | 0 | 0 | 1.3 s |
| 28_clock | PASS | 86 | 0 | 0 | 0 | 0.4 s |
| 28_timezone_retry | PASS | 36 | 0 | 0 | 0 | 0.1 s |
| 29_admin_profiles | PASS | 18 | 0 | 0 | 0 | 0.1 s |
| 29_family_controls | PASS | 47 | 0 | 0 | 0 | 3.0 s |
| 30_ui_audit | PASS | 137 | 0 | 0 | 0 | 2.2 s |
| 40_biscuit_naming | PASS | 29 | 0 | 0 | 0 | 0.1 s |
| 41_biscuit_care | PASS | 44 | 0 | 0 | 0 | 0.1 s |
| 42_biscuit_fetch | PASS | 19 | 0 | 0 | 0 | 0.1 s |
| 43_biscuit_tricks | PASS | 42 | 0 | 0 | 0 | 0.3 s |
| 44_daily_cap_games | PASS | 15 | 0 | 0 | 0 | 0.0 s |
| 45_biscuit_tour | PASS | 35 | 0 | 0 | 0 | 0.4 s |
| 46_biscuit_story | PASS | 44 | 0 | 0 | 0 | 0.3 s |
| 47_biscuit_discoveries | PASS | 53 | 0 | 0 | 0 | 0.2 s |
| 48_biscuit_today | PASS | 41 | 0 | 0 | 0 | 0.2 s |
| 49_biscuit_read_tour | PASS | 132 | 0 | 0 | 0 | 0.9 s |
| 50_marble_levels | PASS | 83 | 0 | 0 | 0 | 0.6 s |
| 51_marble_edges | PASS | 39 | 0 | 0 | 0 | 0.3 s |
| 52_marble_monkey | PASS | 7 | 0 | 0 | 0 | 1.8 s |
| 53_marble_kid_taps | PASS | 23 | 0 | 0 | 0 | 0.0 s |
| 54_marble_by_hand | PASS | 12 | 0 | 0 | 0 | 0.1 s |
| 60_tiltfc_calibrate | PASS | 25 | 0 | 0 | 0 | 0.1 s |
| 61_tiltfc_goal | PASS | 10 | 0 | 0 | 0 | 0.1 s |
| 62_tiltfc_fulltime | PASS | 17 | 0 | 0 | 0 | 0.1 s |
| 63_tiltfc_monkey | PASS | 4 | 0 | 0 | 0 | 2.7 s |
| 64_tiltfc_kid_taps | PASS | 23 | 0 | 0 | 0 | 0.1 s |
| 70_pumpkin_win | PASS | 26 | 0 | 0 | 0 | 0.1 s |
| 71_pumpkin_loss | PASS | 11 | 0 | 0 | 0 | 0.1 s |
| 72_pumpkin_tilt | PASS | 12 | 0 | 0 | 0 | 0.1 s |
| 73_pumpkin_kid_taps | PASS | 12 | 0 | 0 | 0 | 0.0 s |
| 74_pumpkin_monkey | PASS | 4 | 0 | 0 | 0 | 5.7 s |
| 75_pumpkin_navigation | PASS | 8 | 0 | 0 | 0 | 0.2 s |
| 76_pumpkin_ghost | PASS | 14 | 0 | 0 | 0 | 0.1 s |
| 77_pumpkin_rustle | PASS | 14 | 0 | 0 | 0 | 0.1 s |
| 80_kingdom_reader | PASS | 76 | 0 | 0 | 0 | 0.4 s |
| 80_radar | PASS | 23 | 0 | 0 | 0 | 0.0 s |
| 81_radar_monkey | PASS | 1 | 0 | 0 | 0 | 1.4 s |
| 90_starfighter_flow | PASS | 29 | 0 | 0 | 0 | 0.1 s |
| 91_starfighter_controls | PASS | 34 | 0 | 0 | 0 | 0.1 s |
| 92_starfighter_stages | PASS | 35 | 0 | 0 | 0 | 0.2 s |
| 93_starfighter_navigation | PASS | 46 | 0 | 0 | 0 | 0.1 s |
| 94_starfighter_monkey | PASS | 12 | 0 | 0 | 0 | 10.7 s |
| 95_toytown_school | PASS | 57 | 0 | 0 | 0 | 0.3 s |
| 96_toytown_cafe | PASS | 24 | 0 | 0 | 0 | 0.1 s |
| 97_toytown_park | PASS | 65 | 0 | 0 | 0 | 0.2 s |
| 98_toytown_monkey | PASS | 25 | 0 | 0 | 0 | 6.7 s |
| 99_toytown_profiles | PASS | 95 | 0 | 0 | 0 | 0.2 s |
| a0_racing_flow | PASS | 41 | 0 | 0 | 0 | 0.5 s |
| a1_racing_monkey | PASS | 4 | 0 | 0 | 0 | 11.5 s |
| a2_launcher_pages | PASS | 9 | 0 | 0 | 0 | 0.1 s |
| a3_sound_admin | PASS | 11 | 0 | 0 | 0 | 0.1 s |
| a4_racing_board | PASS | 9 | 0 | 0 | 0 | 0.2 s |
| a5_racing_level | PASS | 31 | 0 | 0 | 0 | 0.8 s |
| a6_racing_mode_kids | PASS | 10 | 0 | 0 | 0 | 0.1 s |
| a7_racing_times | PASS | 14 | 0 | 0 | 0 | 0.2 s |
| a9_racing_times_fingers | PASS | 16 | 0 | 0 | 0 | 0.4 s |

## UI findings

0 FAIL (hard rule) and 0 WARN (soft rule) findings. Logical px: 1 px = 0.33 mm, the glass is the circle of radius 80 around (80,80).

| Severity | Rule | Screen | Count | Findings | Seen in |
| --- | --- | --- | --- | --- | --- |

## Contact sheets

- build/playtest/01_adopt-adoption.png
- build/playtest/02_feed-feed.png
- build/playtest/03_fetch-fetch.png
- build/playtest/04_words-words.png
- build/playtest/05_read-read.png
- build/playtest/06_trick-trick.png
- build/playtest/07_bath-bath.png
- build/playtest/08_sleep-sleep.png
- build/playtest/09_days-days.png
- build/playtest/10_profiles-profiles.png
- build/playtest/11_stats-stats.png
- build/playtest/12_reset-reset.png
- build/playtest/13_profiles_delete-delete.png
- build/playtest/14_shell_edges-create.png
- build/playtest/14_shell_edges-delete.png
- build/playtest/15_shell_taps-solo_rest_reset.png
- build/playtest/16_night-night.png
- build/playtest/17_daily_cap-daily_cap.png
- build/playtest/19_system_menu-update.png
- build/playtest/19_system_menu-states.png
- build/playtest/19_system_menu-dev.png
- build/playtest/24_brightness-brightness.png
- build/playtest/24_wifi_setup-wifi.png
- build/playtest/25_admin_menu-optional_admin_and_usage_reset.png
- build/playtest/26_player_pin_reset-recovery.png
- build/playtest/26_player_pin_reset-persistence.png
- build/playtest/27_play_limits-settings_and_caps.png
- build/playtest/27_play_limits-persistence_and_bounds.png
- build/playtest/28_clock-timezones_and_clock.png
- build/playtest/28_clock-offline_fallback.png
- build/playtest/29_admin_profiles-admin_profiles.png
- build/playtest/29_family_controls-play_window_and_override.png
- build/playtest/30_ui_audit-adoption.png
- build/playtest/30_ui_audit-play.png
- build/playtest/30_ui_audit-read_tricks.png
- build/playtest/30_ui_audit-care.png
- build/playtest/30_ui_audit-days_street.png
- build/playtest/30_ui_audit-grown.png
- build/playtest/30_ui_audit-bath.png
- build/playtest/40_biscuit_naming-naming.png
- build/playtest/41_biscuit_care-care.png
- build/playtest/42_biscuit_fetch-fetch.png
- build/playtest/43_biscuit_tricks-tricks.png
- build/playtest/44_daily_cap_games-daily_cap_games.png
- build/playtest/45_biscuit_tour-tour.png
- build/playtest/46_biscuit_story-story.png
- build/playtest/47_biscuit_discoveries-discoveries.png
- build/playtest/48_biscuit_today-today.png
- build/playtest/49_biscuit_read_tour-screens.png
- build/playtest/50_marble_levels-levels.png
- build/playtest/51_marble_edges-edges.png
- build/playtest/53_marble_kid_taps-kid_taps.png
- build/playtest/54_marble_by_hand-by_hand.png
- build/playtest/60_tiltfc_calibrate-calibrate.png
- build/playtest/61_tiltfc_goal-goal.png
- build/playtest/62_tiltfc_fulltime-fulltime.png
- build/playtest/64_tiltfc_kid_taps-kid_taps.png
- build/playtest/70_pumpkin_win-win.png
- build/playtest/71_pumpkin_loss-loss.png
- build/playtest/72_pumpkin_tilt-tilt.png
- build/playtest/76_pumpkin_ghost-ghost.png
- build/playtest/77_pumpkin_rustle-rustle.png
- build/playtest/80_kingdom_reader-kingdom.png
- build/playtest/90_starfighter_flow-flow.png
- build/playtest/91_starfighter_controls-controls.png
- build/playtest/92_starfighter_stages-stages.png
- build/playtest/93_starfighter_navigation-exit.png
- build/playtest/97_toytown_park-97_toytown_park.png
- build/playtest/99_toytown_profiles-99_toytown_profiles.png
- build/playtest/a0_racing_flow-flow.png
- build/playtest/a2_launcher_pages-launcher.png
- build/playtest/a3_sound_admin-sound.png
- build/playtest/a4_racing_board-board.png
- build/playtest/a7_racing_times-times.png
- build/playtest/a9_racing_times_fingers-fingers.png
