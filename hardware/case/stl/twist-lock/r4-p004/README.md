# R4 handheld case with the P004 twist lock

**Upgrading a P014 case?** Print one [P019 lid for Bambu Studio](bambu/P019-lid.3mf) or use the [lid STL](P019-lid.stl). Move the three original lid screws to the new part. The bezel, board plate, slider and wheel carry over.

For a complete case, print one of each item below.

| Part | Print file |
| --- | --- |
| P019 lower lid | [Single-color H2D project](bambu/P019-lid.3mf) or [STL](P019-lid.stl) |
| P014 pinstripe bezel | [Two-color H2D project](bambu/P014-Pinstripe.3mf), or import the [bezel body](components/P014-bezel-body.stl) and [pinstripe](components/P014-bezel-pinstripe.stl) together as one multipart object |
| Board plate | [puck_plate.stl](puck_plate.stl) |
| Power slider | [puck_slider.stl](puck_slider.stl) |

The P019 project puts the flat exterior on the bed and the battery pocket upward. It uses a Bambu Lab H2D with a 0.4 mm nozzle, Bambu PLA Basic, 0.20 mm layers, two walls, 15% infill and no supports. Bambu Studio estimates **35 minutes and 18.3 g** for the lid. The recessed P019 mark is inside beside the battery fence.

To mount the puck with the display upright, start with the top of the display **40° counterclockwise from upright**, lower the three guide mouths over the wheel catches and turn clockwise to the stop. Turn it back 40° to lift it out. The three entry positions are spaced 120° apart. P019 uses the P004 trial’s tapered entry, retaining shelf and single detent. It fits the V13 kid wheel and the R2 hub geometry.

Regenerate the lid STL from the repository root with `cad run hardware/case/puck_twist_p019.py`. The [generator](../../../puck_twist_p019.py) uses the production battery fence, buzzer vents, and asymmetric screw bosses.
