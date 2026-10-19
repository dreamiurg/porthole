# Family screen time and local time

## Contract

Parents manage all screen-time rules from Admin → Screen time. A new device
allows 15 minutes in one session and 30 minutes across all games per player per
local calendar day. The existing ten-minute break follows a full session. A
minute without touch or movement does not count as play. Existing saved limits
remain unchanged when updating firmware.

The optional play window defaults to 2:00 p.m. inclusive through 7:30 p.m.
exclusive. Parents can turn it off or change either endpoint, including an
overnight window. Equal endpoints are rejected. The window applies to idle
launchers and running games as well as entry; stopping a game flushes its save
before showing a gentle return-time message. Midnight resets the daily allowance
according to the selected local time zone. All screens retain a visible exit.

Reset usage requires confirmation and clears session, daily and rest counters
for every player while preserving rules, profiles, codes and game progress.
It does not bypass the play window. A separate confirmed Allow 30 min action
temporarily overrides the session limit, daily limit and play window for the
device. Play still counts. Parents can end it early; it expires after 30 elapsed
minutes and does not survive reboot. Opening Admin alone never grants extra play.
The optional admin code protects entry to these controls.

Local time defaults to Pacific (`America/Los_Angeles`) with automatic daylight
saving transitions. Named time zones replace editable UTC offsets. Network time
corrects the UTC clock, and bundled time-zone rules derive local time even when
offline. A saved manual zone takes precedence over automatic guessing. In
automatic mode, an HTTPS request to `ipapi.co/timezone/` may choose the zone from
the connection's public IP. No SSID, GPS coordinates, address or player data are
sent. The result is validated against the bundled catalog; a failed or invalid
lookup retains the last valid zone, initially Pacific. A later successful NTP
connection retries failed automatic detection, limited to once per six hours; a
lookup never retries itself from its own clock-sync result. Successful guesses
are cached. A VPN may make the guess
wrong, so manual selection remains available. Clock corrections do not add play
duration: counting and temporary overrides use elapsed milliseconds.

## Persistence and migration

The profile record and all game save layouts stay unchanged. The shipped
four-byte `sys/limits` blob remains two minute counts; only missing or invalid
values adopt 15/30. The window is a separate versioned eight-byte `sys/window`
blob with enabled, start and end fields. Settings writes require readback before
showing success. Failed writes keep the prior in-memory policy and allow retry.
The override is RAM-only. Time-zone and RTC migration are independent of profile
and game records. A verified prepared/committed journal converts legacy local RTC
state using its saved offset; a missing or corrupt legacy offset preserves the
old UTC default. After migration, an hourly verified UTC checkpoint supplies a
conservative recovery time if the RTC fails. Recovery never reinterprets mixed
historical profile timestamps as if their time-zone provenance were known.
Existing daily caps lift when the device must guess the clock.

## Verification

Rule tests cover exact window boundaries, overnight windows, defaults and old
settings, failed writes, monotonic override expiry and rollover, continued usage
under override, and normal enforcement after expiry. Clock tests cover named
zones, daylight saving boundaries, UTC/local conversion and failed lookup
fallback. Final visible-input play must cover settings save/cancel/reentry,
natural session and daily enforcement, cutoff, override/recovery, and exit.
Simulated clock/network outcomes are disclosed separately from ordinary play;
physical RTC and live network behavior need separate hardware verification.
