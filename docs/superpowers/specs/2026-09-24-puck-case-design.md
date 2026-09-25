# Puck case: Porthole on battery

Status: design approved in conversation 2026-09-24 (round puck chosen over a taller egg). Owner: Dmytro (PM). Issue: #14.

## Goal

A printed handheld case that makes Porthole cordless, sized for 5-11 year olds' hands, printable on a
Bambu printer, assembled with a screwdriver and no soldering. It must not block a future charging dock
(#17).

## Decisions

- **Safety over cost and thickness.** The onboard charger (ETA6098, R7 = 82K) fast-charges at up to
  2 A and cannot be changed without soldering. The cell is therefore a **protected 3.7 V LiPo of at
  least 2000 mAh** (1C or less), about 8 x 50 x 50 mm, with a 2-pin 1.25 mm plug
  (MX1.25 / PH1.25). J1 pin 1 is BAT+; many cells ship reversed, so check polarity with a meter.
- **No soldering or board changes**, now or for the dock.
- **Power off = the board's slide switch** (SW1 cuts the battery through Q4). The case exposes it.
  Firmware power saving is separate work (#15, #16).
- **Shape: a round puck with a flat back**, 80 mm across, about 22 mm thick, rounded front and back
  edges. Thickness = board stack (9.5 mm) + cell (8 mm) + walls. A flat back sits still on a table
  while a kid taps, and seats cleanly in a dock. Try to save ~1 mm by holding the board with four lugs
  at the standoffs instead of a full internal plate.
- **CAD is code.** build123d (Python, OpenCascade) in `hardware/case/`, run with `uv`. It imports
  Waveshare's board STEP for fit checks and exports 3MF (Bambu Studio) and STEP (Fusion, if anyone wants
  to edit by hand). No Fusion 360 dependency.
- **Material: PETG** (drop-tough, does not soften in a hot car like PLA).

## Board facts (from Waveshare's schematic and STEP, 2026-09-24)

| Item | Value |
| --- | --- |
| Outline | 75.0 mm round; stack 9.5 mm (glass top z=+3.4 to standoff bottom z=-6.1) |
| Mounting | 4 SMT M2 standoffs on the back, centers about (+-29.5, 13.6) and (+-18.0, -27.1) |
| Side buttons | BOOT and RESET, facing out of the edge near 3 o'clock (x=30.3..34, y=-1.8..10.5) |
| Battery plug | J1, 1.25 mm 2-pin, back side near 3 o'clock (x=17.4..25, y=6..11.2), top at z=-5.4 |
| Battery sense | GPIO4, 200K/100K divider (Vbat / 3) |
| Charge LED | LED1 on STAT, lit while charging |
| RTC backup | J4, 1.0 mm 2-pin for a backup cell |

USB-C and SW1 positions are nested in the STEP sub-assemblies; the model extracts them before any
opening is placed. Coordinates are Waveshare's STEP frame (mm, origin at the board center).

## Parts

1. **Top shell**: front lip over the glass edge (outside the 2.1" active area), outer wall, and an
   internal plate at the standoff plane. The board drops in face down; 4 M2 screws go from behind
   through the plate into the standoffs.
2. **Back cover**: flat, with a pocket that holds the cell against the plate. Fixed to the top
   shell by 2-3 M2 screws into bosses placed outside the cell footprint.
3. Bought: 2000 mAh protected cell, M2 screws (lengths set by the model), optional foam pad.

The top shell carries the board screws because the 50 x 50 mm cell overlaps the standoff circle, so
posts cannot run from the back cover to the standoffs.

## Openings

- USB-C at the bottom edge, with space for a recessed magnetic tip (the dock plan, #17).
- Slide switch: a slot a finger can work, recessed so a bag doesn't flip it.
- BOOT and RESET: pinholes (reachable with a paperclip, not by accident).
- Charge LED: a thin printed light pipe or a window.
- Buzzer: a small vent grid.
- microSD: covered (no app uses it).

## Proportions (decided 2026-09-24)

Rendered from the real board and cell envelopes next to an 8-year-old's palm (~65 mm):

| Option | Size | Result |
| --- | --- | --- |
| Pebble back, 10x40x50 cell | 80 x 24 mm (rim 13.5) | Rejected: the cell's corners reach r = 33.5 of 40 mm, so the dome only curves in the outer few mm and the puck ends up thicker. |
| **Flat back, 8x50x50 cell** | **80 x 22 mm** | **Chosen.** |
| Egg, cell in a lobe below the screen | 80 x 122 x 13.5 mm | Rejected by Dmytro: must be a round puck. |

Constraint to keep in mind: the cell's diagonal must stay under ~70 mm to fit inside the wall
(a 50 x 60 cell, 78 mm diagonal, does not fit).

## Verification

- The model asserts clearances against the board STEP and the cell envelope (no intersections, minimum
  gaps) and fails the build if they break.
- One test print of the top shell ring to tune fit (glass lip, screw seats), then the full case.
- On the device: board seats flat, touch works across the whole glass (the lip must not press it),
  switch, USB-C, LED visible, buzzer audible, the cell does not move when shaken.

## Out of scope

Dock (#17), sleep mode (#15), battery icon (#16), lanyard loop (add if asked), microSD access.
