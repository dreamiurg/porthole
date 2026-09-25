# Puck case: Porthole on battery

Status: design approved 2026-09-24 (round puck); v2 controls design built in Fusion. Owner: Dmytro (PM). Issue: #14.

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

## Parts (v2, 2026-09-24)

Nothing on the front part may protrude inward below the glass: the round 75 mm glass slides in from
the back, so every control lives in the back part, which goes on after the board.

1. **Ring**: front lip over the glass edge (outside the 2.1" active area), 5.6 mm tall.
2. **Plate**: screws to the board's 4 standoffs (M2x4 countersunk, from below); notches let the
   Cup's button nubs and slider fork pass on assembly.
3. **Slider**: separate small part captured in a chord channel inside the Cup wall; thumb knob
   outside, fork around the switch lever inside. Drops in from the seam before the Ring goes on.
4. **Cup**: wall (3.8 mm) + floor + cell pocket; flex-tongue buttons for BOOT and RESET printed in
   the wall (raised dot, nub pushes the side plunger along +X); ledge the Plate sits on.
5. **Fasteners**: 3 x M2x20 pan head from the Cup floor into the Ring. They clamp the stack
   lip - glass - board - Plate - ledge; the seam keeps a 0.2 mm gap.

Overall: 83.2 mm across, 24.8 mm thick. Source of truth: `hardware/case/puck_case_fusion.py`
(rebuilds the Fusion document, checks part/board/cell interference, both slider travel ends and
the assembly path); exports in `hardware/case/stl/`; Fusion project "Porthole".

## Openings

- USB-C (native, 5 o'clock) and USB-C (UART, 7 o'clock), recessed, sized for a standard overmold.
- Power slider (3 o'clock, below the buttons), BOOT and RESET flex buttons above it.
- Charge and power LEDs: 2 mm windows in the Cup wall (the LEDs face the back of the board; print
  translucent so the wall glows).
- Buzzer: vent holes through the Plate and the Cup floor (position from Waveshare's photo).
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
