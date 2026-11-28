# Puck case print kits

Download the parts from a single **Kit** folder. Print one of each part listed in its row. In the v1 through v1.4 rows, bezel, plate and lid refer to `puck_bezel.stl`, `puck_plate.stl` and `puck_lid.stl`. A listed slider is one extra printed part.

| Kit | Parts for one puck | Switch and mounting |
| --- | --- | --- |
| [R3 Pinstripe · P014](twist-lock/r3-pinstripe/) | Two-color bezel, board plate, straight power slider and R2 lid | 84 mm handheld with a Hot Orange face ring. Locks into the V13 wheel. |
| [v1](v1/) | Bezel, plate, lid | Original 79.6 mm case with an opening for the board switch. |
| [v1.1](v1.1/) | Bezel, plate, lid | 84 mm case with a thicker wall and revised screw pilots. Board switch opening. |
| [v1.2](v1.2/) | Bezel, plate, lid | Snap-groove lid for the v3 grip. Includes the matching v1.1 bezel and plate. Board switch opening. |
| [v1.3 original](v1.3-original/) | Bezel, plate, lid | First magnetic lid and flat screw seats. Original bezel has a board switch opening. |
| [v1.3 with slider](v1.3/) | Bezel, plate, lid, `puck_slider.stl` | Revised bezel and curved SW1 slider. Magnetic lid. |
| [v1.4](v1.4/) | Bezel, plate, lid, `puck_slider.stl` | Straight SW1 slider and magnetic lid. One screw post moved from 350° to 335°, so use this kit's matching plate and lid. |
| [Twist-lock R2](twist-lock/r2/) | `puck_bezel.stl`, `puck_plate.stl`, `puck_slider.stl`, `side-groove-lid-r2.stl` | Straight SW1 slider. Magnet-free lid fits the integrated v9 wheel or a converted v8 kid wheel. |
| [Fusion v2](v2/) | `puck_ring.stl`, `puck_plate.stl`, `puck_cup.stl`, `puck_slider.stl` | Historical ring-and-cup enclosure with captured slider and flex buttons. |
| [Fusion v3](v3/) | `puck_ring.stl`, `puck_plate.stl`, `puck_cup.stl`, `puck_slider.stl` | Revised ring, plate, cup and matching slider. |

**P014 Pinstripe.** The Bambu Studio project prints the Deep Midnight body (`#09182C`) and 1.35 mm Hot Orange ring (`#FF7900`) together as one bezel. Print the other three STL files once each: the board plate and grooved lid in Deep Midnight, and the power slider in Hot Orange. For an existing R2 case, the bezel alone replaces the original single-color bezel. The two component STLs share one lip-down print position. Import them as parts of one object when using another slicer. [Two-color bezel · 3MF](twist-lock/r3-pinstripe/bambu/P014-Pinstripe.3mf) · [Complete kit](twist-lock/r3-pinstripe/) · [Blender source](../pinstripe_blender.py)

The v1.3, v1.4 and R2 kits each include a single ready-to-print **one-dimple slider**. It is the first body from that version's original four-slider trial, with a 0.25 mm lever gap per side and 1.0 mm travel in each direction. In each kit, `fit-test/slider_test.stl` prints all four alternatives at once. Fit **one** slider around the board's SW1 lever. The test sliders have one through four dimples. The corresponding (gap, travel) pairs in millimetres are (0.25, 1.0), (0.25, 1.5), (0.45, 1.0), (0.45, 1.5). Use the matching `fit-test/switch_section.stl` to try them against your board.

**Wheel hardware for R2.** The v9 wheel front already includes its catches. Only a v8 kid wheel being converted needs the separate [glue-on hub](twist-lock/r2/retrofit/wheel-hub-r2.stl). The [R2 fit gauges](twist-lock/r2/fit-test/) are optional trial prints. The two rings test all three catches together; the single-catch file prints a track and lug for a smaller trial.

**Historical slider prototype.** The [v4 switch fixture](v4-slider-prototype/) is a bench model. Print one `v4_block.stl`, one `v4_lid.stl` and one `v4_slider.stl`. The symmetric slider in `variants/` can replace the standard one. The [v3 test pieces](v3/fit-test/) offer a choice of wall travel and slider gap.

The original v1.3 parts come from [the magnetic-case change](https://github.com/dreamiurg/porthole/commit/9a622cd). The Fusion v2 kit comes from [the original ring-and-cup revision](https://github.com/dreamiurg/porthole/commit/12736bb). Its dimensions and slider differ from Fusion v3.
