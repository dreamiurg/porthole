# Battery: charge on the picker, details in Admin, drain in PostHog

Status: written 2026-11-28 while the user was away. The parts marked *(user)* are the user's decisions. The parts
marked *(Claude's call)* are open for review.

## Why

The board now has a battery connected permanently. A grown-up wants to see at a glance how much charge is left. They
also want to learn how fast the battery drains in sleep and in play, and to look at a charge and discharge graph later.

## What the board can know

From the Waveshare schematic and demo (`local/waveshare/`):

- **The only reading is the battery voltage.** `BAT_ADC` is GPIO4, behind a 200K/100K divider (1:3) on the `BAT` net.
  That net is the charger's (ETA6098) battery side and the connector J1, before the power switch SW1. The vendor demo
  reads `analogReadMilliVolts(4) * 3 / 0.992857`.
- **No charge-status line or USB-power line reaches the chip.** The charger's STAT pin drives only the CHG LED, and
  VBUS goes to no GPIO. There is no current sensor.
- So "charging" and "time left" can only be inferred from how the voltage moves:
  - plugging in lifts the voltage by about 0.1-0.2 V, then it climbs toward 4.2 V;
  - unplugging drops it;
  - while discharging it falls slowly.
- **A board with no battery** reads whatever the charger leaves on `BAT`. On USB that is likely close to 4.2 V, which
  looks like "full, charging". Not measured; such a board may show a battery it does not have.
- The ADC reads the voltage under load, which sits below the battery's open-circuit voltage. The percentage therefore
  reads a little low while the screen is lit. It is an estimate: the picker rounds it to 5 %, and Admin marks the
  time left with "~".

## Goals

- **Picker** *(user)*:
  - only a battery icon filled in proportion to the charge, and an approximate percentage;
  - a bolt while charging *(user)*;
  - nothing more.
- **Admin** *(user)*:
  - the precise reading (volts);
  - a better estimate (time left).
- **Drain data** *(user: PostHog)*:
  - samples sent with telemetry, so PostHog graphs charge over time;
  - drain in sleep and drain in use, separately.

Non-goals:

- no low-battery warnings, sounds or popups for kids;
- no shutdown at low charge (the charger and cell protection handle that);
- no history kept on the device (NVS has 8 spare entries; the SD card is unusable after the first sleep).

## Approaches considered

1. **Voltage only, inferred states** *(user picked A)*. Everything above comes from GPIO4. It costs nothing, but
   charging and time left are guesses.
2. **A fuel-gauge chip**. Accurate, but it needs new hardware. Out of scope.
3. **Coulomb counting from a load model** (backlight, radio and sleep currents measured once). More precise time left,
   but it needs calibration we don't have. Rejected for now: PostHog data can feed it later.

## Design

### `os/battery.h`: pure logic, host-tested *(Claude's call)*

- `battery::percent(mv)`: a typical Li-ion open-circuit curve (4.20 V = 100 %, 3.73 V = 20 %, 3.27 V = 0 %), linear
  between its points.
- `battery::Gauge`: fed a smoothed reading every 10 s with the UTC time. It keeps one point a minute for the last hour
  in RAM. It answers `Reading { mv, pct, charging, minutesLeft }`.
- **Charging needs positive evidence.** The thresholds are named constants with a `shortcut:` note, to tune from the
  PostHog data:
  - It goes on after a jump of ≥ 60 mV between two samples under a minute apart (plug in), or a rise of ≥ 20 mV over
    10 minutes.
  - It goes off after a drop of ≥ 60 mV between such samples (unplug), or a fall of ≥ 10 mV over 10 minutes.
  - A reading at 4.17 V or more counts as charging when nothing says otherwise: the charger holds the cell there.
  - Backlight steps change the load by tens of mV, not 60.
  - A gap longer than a minute (sleep) never counts as a jump.
- **Time left only after a steady fall.** It needs at least 20 minutes of minute points since charging last ended. The
  slope comes from a least-squares fit of percent over time, then `minutesLeft = pct / rate`. Otherwise it is unknown
  (-1), and the screen shows nothing rather than a wrong number. Sleep inside the window counts too. The result is the
  board's real mix of use and sleep, which is what "time left" should mean.

### Firmware *(Claude's call)*

- `board::batteryMv()`: 16 reads of GPIO4 averaged, times 3 / 0.992857 (the vendor's calibration).
- `main.cpp`:
  - samples every 10 s into the gauge;
  - hands the reading to the shell through `shell::System::battery()`, as the clock and telemetry status go;
  - sends the telemetry below;
  - adds `battery_mv= battery_pct= charging= left_min=` to `S` and to the `sleep` / `woke` serial lines.

### Picker *(Claude's call on layout)*

- **Placement.** It sits centered above "Who's playing?", at y 6-13 (the glass is 60 px wide there):
  - a 12x7 battery outline with a nub, filled in proportion to the charge;
  - then the percentage rounded to 5 ("80%", 8x8 font);
  - then a 5x8 bolt while charging.
- **Fill color:** green above 20 %, orange at 20 % or less.
- Shell palette only, no sound.
- Display only. It lies inside the title's 5 s Admin hold box, which is unchanged.
- No reading (sim without one, or under 3.0 V): no icon.

### Admin *(Claude's call on placement)*

- Admin 2/2 ("Device settings") is the device page. The battery line replaces its subtitle at y 46, one line:
  - "Battery 78%, 3.92 V, ~5 h left";
  - "Battery charging, 4.18 V";
  - or "Battery 78%, 3.92 V" while time left is unknown.
- Info is full: eight rows would collide with its buttons.

### Telemetry *(user: PostHog; event shape is Claude's call)*

One event, `battery` {mv, pct, charging, backlight, at, slept_s}:

| `at` | When | Extra |
| --- | --- | --- |
| `awake` | every 5 min while the screen is lit (dimmed included) | `backlight` (0-100) |
| `sleep` | right before light sleep, backlight 0, radio idle | |
| `wake` | right after the wake, same load as `sleep` | `slept_s` (RTC seconds) |

- **Graphs.**
  - Drain in sleep is `sleep` to the next `wake`, over `slept_s`.
  - Drain in use is the `awake` series, which also lines up with `game_heartbeat`.
  - Charge over time is all of them.
- **Delivery.** The queue lives in RAM and survives light sleep. Wifi is off in sleep, so a `sleep` event goes out after
  the wake.
- **Level.** Personal level only: it is not in the anonymous list. Nothing about the kid goes with it.
- **Docs.** `docs/telemetry-events.md` and root rule 4 say so.

### Sim and tests

- The sim's `System` returns a canned reading: 3990 mV, discharging, 4 minutes left per percent; none in the web
  player. The script commands `sys battery MV`, `sys charging MV` and `sys nobattery` set it for screenshots.
- Host test `host/test_battery.cpp` covers:
  - the percent curve's ends and monotonicity;
  - plug in, unplug and slow-charge detection;
  - no flip on backlight-size steps or across a sleep gap;
  - time left appearing only after 20 minutes of fall.

## Left to verify on the board

- What a full battery on USB reads, a no-battery board, and real discharge slopes. The morning ask is to unplug for an
  hour, then read PostHog.
- Sleep behavior itself is PR 142's open item.
