# Browser emulator verification

Verified locally on 2026-09-24. This verifies the browser implementation, not ESP32 firmware or long-term enjoyment.

## Runnable checks

`npm run check`: syntax checks plus **29 passing Node tests**. Coverage includes care/clamping, sleep, capped absence, clock rollback, timezone movement, v1 migration, malformed saves, local-day progression, all twelve sticker rewards, permanent mastery, growth thresholds, distinct trick art, reduced motion, stale-tab saves, full/blocked storage, cross-midnight save merging, concurrent daily rewards, discovery persistence, the 96-card daily rotation, text pagination, primary/secondary pointer filtering, and complete illustration coverage with integer pixels and RGB565 colors.

## Browser flows

Checked in the Codex in-app browser, including a 390 × 844 phone viewport:

- Feed and cuddle change needs; nap disables care; Wake restores controls.
- Fetch accepts five touches, gives the reward, and returns to home controls.
- Choosing fetch from a daily invitation wakes a napping Biscuit. Enter opens the menu; Escape returns from fetch to home.
- Bookshelf paging shows future books with their active-day unlocks.
- Reading advances through passages, presents both choices, shows the chosen ending, and preserves one-time book rewards on reread.
- Passage focus introduces the new text to keyboard users. All twelve rendered Moon Biscuit opening pages and five ending pages fit at 24px without text overflow. Both choices and rereading were exercised.
- Today's invitation records snack, story, and fetch; completing all three earns a permanent sticker. The pocket-word page returns correctly.
- A wrong training cue gently retries. Three successful practices master Sit, which remains mastered after reload and can be demonstrated.
- World, daily, library, tricks, scrapbook profile, and sticker views fit within the 480px logical screen without scrolling or hidden content. No horizontal overflow at the checked phone size.
- All 96 discoveries have described pixel illustrations. Visual contact sheets were inspected across science, culture and life; the eye diagram was corrected to focus light on the retina. The illustrated list, cover, prose, return-to-picture, source, Keep, and notebook flows were exercised in the browser. Covers and topic thumbnails fit the fixed 480px screen; calibrated physical preview still measures 202 CSS pixels.
- Discoveries show three daily suggestions across two list pages; topic browsing and notebook rereading work. A Moon discovery was read, its source opened in the source panel, and it survived a reload after being kept. Source links are ordinary external links.
- Training now advances through three, four, and five cues for Sit; retry and mastery were checked in the UI.
- Story, choice, reward, library, daily, word, world, training, profile, sticker, discovery, and source panels were checked for text overflow and controls extending beyond the circular boundary.
- Hardware preview uses a 160×160 Canvas at 3×, bundled fonts and the RGB565 palette. The physical-size toggle measured 202 CSS pixels at default and 240 after calibration. It is an approximation, not a physical measurement.
- Primary touch click filtering is covered by a Pointer Events regression test. Browser UI checks used pointer clicks; actual touchscreen input remains unverified.
- Sound toggle updates its pressed state. Audio output was not independently measured.
- Two tabs synchronize sleep and wake. Reloading both preserves three completed books and learned Sit. Unit tests separately cover stale-book overwrite and storage-write failure.
- No browser errors or warnings were observed during these flows.

Code review found and corrected stale-tab progress loss, unwritable-storage rollback, inconsistent cross-day saves, and mastered demonstrations not counting toward daily training. Review also corrected concurrent daily reward loss, stale discovery navigation when opening a sidebar book, and incorrect reliance on click.isPrimary. The relevant regression tests pass.

Unverified: exact board variant, device frame rate/memory use, physical touch accuracy, and players' sustained interest. The source-backed rationale and engagement assumptions are in [game-design.md](game-design.md).
