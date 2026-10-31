# Family controls validation

Implementation base: `10bb122` (main, including the existing Living Kingdom and
adult typography changes). Final host emulator SHA-256:
`e0b595d2a5ab6ed6a1a74cf229835baf1cafbe8e17e39705fd540a607acfdc1f`.

The [visible-input report](README.md), declared fixtures, input traces and RGB
manifest document the actual playthrough. New screenshots remain in the private
`build/family-controls/` bundle; they are not Git assets.

## Final checks

- Host lint, content, unit and render checks pass.
- All **69/69** normal playtest scenarios and **60/60** coverage scenarios pass,
  with **zero UI failures or warnings**. Coverage is **98.7% lines**, **99.3%
  functions** and **80.4% branches**; the existing threshold is unchanged.
- All static hooks pass, including secret scanning, Ruff, mypy and Semgrep.
  The complexity gate and four playtest-runner self-tests pass.
- Independent review's final verdict is **SHIP**, with no open findings.

## Clock and firmware

- An independent C++ catalog probe was compared with Python `zoneinfo` at noon
  on the first and fifteenth of every month from 2024 through 2099: **795,264
  comparisons across 436 zones, zero mismatches**. The final lossless range
  compaction was included. Rule tests separately check exact spring/fall and
  Morocco Ramadan transition boundaries, fractional offsets and leap day.
- `python3 tools/timezones.py --check` passes against the pinned public-domain
  IANA 2026b source. Clock/timezone unit tests cover provider parsing, saved-cache
  validity, delayed automatic retry, six-hour cooldown, timer rollover, corrupt
  journals and conservative UTC recovery.
- Release firmware built with the deliberately long version `99.99.99` passes
  without warnings. Program size is **8,321,434 / 8,323,072 bytes** (1,638 bytes
  remain); static RAM is **219,080 / 327,680 bytes**. Existing partitions and OTA
  display/cancellation safety remain in place. The small flash margin means
  future additions must continue to pass the size gate.

## Review

The read-only source review covered state transitions, record compatibility,
policy gates, save flushing, time arithmetic, finite timezone transitions,
cross-core handoffs and migration recovery. Findings fixed before landing:

- Preserve finite IANA transitions rather than relying only on recurring rules.
- Latch override/cooldown expiry so timer rollover cannot revive stale state.
- Invalidate stale automatic results and persisted cache validity on network
  changes and explicit redetection, before changing credentials.
- Establish UTC before the lookup's HTTPS certificate checks.
- Restore UTC checkpoints without guessing the provenance of old local profile
  timestamps.
- Retry failed automatic detection after a genuinely later successful clock
  connection, with bounded frequency and manual precedence.
- Make combined daily/rest/window prompts agree with when play can resume.

The new schedule also exposed a host-fixture startup issue: raw profile creation
could happen before the shell received its first local-clock frame. The
simulator now seeds the clock after UTC conversion without advancing elapsed
time. Existing night/game tours explicitly disable the optional play window to
keep their original scope; dedicated family-control scenarios test it enabled.

The deliberate thirty-minute override replay needs a 120-second host deadline
under unoptimized coverage instrumentation. Only `29_family_controls` receives
that allowance; ordinary scenarios keep 30 seconds and monkeys keep 120. All
assertions and sanitizer/UI gates remain strict. The runner was split into small
execution/result helpers, removing its old complexity exemption; four runner
self-tests verify deadline scope and rejection of failed/missing assertions,
sanitizer output, unknown commands and nonzero exits.

## Limits of this verification

No physical board, RTC battery interruption, touch hardware, live Wi-Fi/NTP or
live geolocation exchange was exercised. Network/date results in the emulator
are explicitly canned. Automatic approval review rejected live geolocation
probes because they disclose network metadata to external services; no bypass
or alternate probe was attempted. The implementation retains the saved zone on
failure and supports an explicit manual choice.
