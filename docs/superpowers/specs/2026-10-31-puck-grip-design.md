# Snap-on tilt grip for the puck

Approved in chat 2026-10-31. Model: `hardware/grip/grip_v1.py`; lid change in `hardware/case/puck_v1.py` (v1.2).

## Why

Kids play tilt games (Racing, Starfighter, Tilt FC, Marble Kick) by tilting the whole puck. A two-handed
grip is easier to hold steady than the bare Ø84 puck. Starfighter fires on a tap and Tilt FC kicks on a
tap anywhere, so a thumb must reach the glass while both hands are on the grips.

## Design

- Shape: Yoke Pro horns (upright grips, open top) with the Racing U bottom bar, picked from
  ChatGPT concept renders. One colour; the two-tone renders were only illustrations.
- Snap: rear click ring. The lid gets a 1 mm V groove (z -14.5) and a 4 mm key notch at the
  bottom of the screen (-90 deg). Six 1.4 mm PETG fingers with 0.8 mm bumps click into the groove;
  a rib in the notch fixes the rotation, so the accelerometer axes stay square to the grips.
- Cradle stops 0.6 mm below the bezel/lid seam: screen, USB-C ports, BOOT/RESET and LED
  windows stay open. Thumb hole in the back (r 34) pushes the puck out and clears the buzzer vent.
- Grips at x = +/-78 (inner edge 26 mm from the glass), 28 mm wide, 26 mm deep with rounded fronts.
  Overall 194 x 127 x 31 mm. Prints back-down with no supports.
- Bezel and plate are unchanged; only the lid is reprinted.

## Checks in the model

- Seated puck overlaps nothing in the grip (bumps sit in the groove, the rib in the notch).
- Fits a 250 mm bed (X1 Carbon is 256).
- Finger bending strain 1.5 * t * d / L^2 = 2.8%, under 3% for repeated snaps in PETG.

## Test plan

1. Print `hardware/case/stl/v1.2/puck_lid.stl` and `hardware/grip/stl/snap_test.stl` (the cradle alone).
   Check the click, the hold when shaken, and push-out through the thumb hole.
2. Adjust BUMP / FINGER_T / FIT in `grip_v1.py` if it is too loose or too stiff, then print
   `hardware/grip/stl/grip_v1.stl`.
3. Hand it to a kid: thumb reach to the glass, grip size, weight.

## Unverified

- Thumb reach and grip size for ages 5-11 are estimates.
- 0.8 mm engagement and 2.8% strain are rules of thumb; the snap test print decides.
