# Toy Town Life validation

Validated on 2026-10-25 in the Linux cloud host; no physical device was used.
Source/evidence fingerprints: [build manifest](blackbox/build-manifest.json).

| Gate | Actual result |
| --- | --- |
| `make -C apps/porthole check` | Passed: asset/content checks, warning-fatal host compilation, every host test including Toy Town and lossless Biscuit picture checks. |
| `ASAN_OPTIONS=detect_leaks=0 make -C apps/porthole ci REQUIRE_NODE=1` | Passed: check, all 73 scenarios present at launch, coverage. A subsequent full playtest includes the added profile/rest scenario. |
| Coverage | 98.7% lines (7,618 / 7,716), 99.3% functions; existing 97% threshold retained. |
| Final full playtests | 74 scenarios; see the [preserved report](playtest-report.md) for exact results and every UI audit. |
| Toy Town regressions | `95` School/wardrobe, `96` Cafe, `97` Park, `98` sanitizer monkeys, `99` actual shell rest and two-profile isolation. |
| `make complexity` | Passed with existing thresholds; no whitelist additions. |
| Native black-box session | Three complete player-led outings with mistakes/recovery, memory edit/skip, exit and exact-page restart; 39 native frames. Independent reviewer replay reproduced all frames pixel-for-pixel. |
| Firmware | `make -C apps/porthole firmware` passed, 7,922,002 / 8,323,072 flash bytes (95.2%), about 401 KB free. No partition changes. |
| Artwork | All five original concept PNG SHA-256 values match the design manifest. Runtime art is separate and reproducibly checked. |
| Review | Independent Astra review; every actionable finding fixed and re-reviewed. No remaining code findings, conditional ship pending recorded final checks. |

The sandbox's ptrace environment prevents LeakSanitizer shutdown inspection.
`detect_leaks=0` was set only for that limitation; AddressSanitizer and UBSan ran
and passed. No sanitizer reports were filtered from the runner.

The initial full run failed one existing Starfighter visible-entry scenario
because a nine-game balanced layout moved its tile. The final launcher preserves
the first eight tiles and appends Toy Town; the same unmodified Starfighter
scenario then passed. A shell test pins preservation of those original pages.

The first firmware attempt exceeded the slot. Lossless RLE for Biscuit's 96
raw discovery pictures saved about 729 KB. Every original pixel is frozen by CRC
and real-decoder comparisons; source scenes and pixel manifest hashes remain
unchanged. New Toy Town runtime artwork occupies about 312 KB.

See [observed playthrough and limits](blackbox/README.md) for qualitative findings,
resolved usability issues, input evidence, screenshots and deferred features.
