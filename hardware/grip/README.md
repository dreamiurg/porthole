# Steering wheel print kits

Choose one wheel kit. The wheel and puck enclosure print separately.

| Wheel | Width | Print from this folder | Puck mount |
| --- | ---: | --- | --- |
| [V13 kid](stl/v13-kid/) | 172 mm | Two two-color PLA halves and four alignment pins | Recessed R2 twist lock in the rear half |
| [V9 kid](stl/v9-kid/) | 172 mm | Front, back, four-pin set | R2 twist lock |
| [V9 adult](stl/v9-adult/) | 215 mm | Front, back, four-pin set | R2 twist lock |
| [V8 kid](stl/v8-kid/) | 172 mm | Front, back, four-pin set, cover | Steel ring in the wheel and magnet in the puck |
| [V7 adult](stl/v7-adult/) | 215 mm | Front, back, four-pin set, cover | Steel ring in the wheel and magnet in the puck |
| [V6](stl/v6/) | 184 mm | Front and back | V1.2 snap lid |
| [V5](stl/v5/) | 184 mm | One-piece wheel | V1.2 snap lid |
| [V4](stl/v4/) | 171 mm | One-piece wheel | V1.2 snap lid |
| [V3](stl/v3/) | 215 mm | One-piece wheel | V1.2 snap lid |
| [V1](stl/v1/) | 194 mm | One-piece wheel | V1.2 snap lid |

## V13 kid wheel

Open the [complete H2D project](stl/v13-kid/bambu/V13-wheel-H2D.3mf) to print both halves and four pins on one plate. Individual plates are also available for replacements. The front contains the two upper spokes that follow the cylindrical socket wall. The rear contains the three bayonet catches, their seating pads and four loose alignment pins on the same plate. If you already have a P016 rear and pins, print only the P017 front. [P017 front plate](stl/v13-kid/bambu/P017-front.3mf) · [P016 rear and pins plate](stl/v13-kid/bambu/P016-back.3mf)

Both projects use a Bambu Lab H2D with two 0.4 mm nozzles and 0.20 mm layers. The front slices without support material. The rear uses supports from the build plate. Each half prints as one object with a Deep Midnight PLA body (`#09182C`) and two Hot Orange PLA grip accents (`#FF7900`). To slice on another printer, import the three STLs for each half together as parts of one object and keep their relative positions. The separate four-pin STL duplicates the pins on the rear plate. [Aligned components](stl/v13-kid/components/) · [Four pins](stl/v13-kid/pins-4.stl)

Remove the supports, dry-fit the halves on the four pins, then seat and twist the puck to check screen orientation and thumb reach. Glue the halves together after the fit check. The wheel accepts the R2 side-groove puck lid. Remove the puck from the wheel to plug in a USB cable. [R2 lid](../case/stl/twist-lock/r2/side-groove-lid-r2.stl)

Open the assembled wheel in a glTF viewer. For editable Blender meshes, start Blender from the repository root with `blender -P hardware/grip/assemble_v13_blender.py`. The scene uses millimetres and includes both bodies, four grip accents and four installed pins. [3D assembly](source/P017-assembled.glb) · [Blender assembly script](assemble_v13_blender.py)

## Earlier wheels

**V9.** Fit the four pins, then glue the two halves together. The catches are built into the front. Use the [R2 twist-lock lid](../case/stl/twist-lock/r2/side-groove-lid-r2.stl). The kid front fits the existing V8 kid back and pins. The adult front fits the V7 adult back and pins. [Kid preview](previews/v9-kid.png) · [Adult preview](previews/v9-adult.png)

**V7 and V8.** Glue the halves together with the four pins. For the magnetic mount, place a 56 mm OD × 46 mm ID × 0.5 mm steel ring in the front recess and fit the printed cover over it. When retrofitting the [R2 hub](../case/stl/twist-lock/r2/retrofit/wheel-hub-r2.stl) to the V8, leave that recess empty and bond the hub in place.

**V1 through V6.** These wheels use the [V1.2 snap lid](../case/stl/v1.2/puck_lid.stl). V6 needs four 7 mm lengths of 1.75 mm filament to align the halves during gluing. V5 has a rounded back and was printed with supports. Its model-only 3MF preserves the triangles from the original prototype STL. V1, V3 and V4 print as single pieces.

[Prototype gauges](stl/prototypes/) include the early snap cradle, right-hand ergonomics gauge and V6 handle sections. For a replacement V7–V9 pin, print the [single pin](source/alignment-pin.stl).
