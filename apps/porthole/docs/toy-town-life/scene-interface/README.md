# Toy Town scene interface

The approved [School reference](approved-school-reference.png) is an Image Gen
design target, not a game screenshot. This update carries its scene-first style
through Home, destinations, packing, School, Cafe, Park, memories and the
scrapbook. It retains the shipped pictured wardrobe and every game's launcher.

Separate backgrounds, characters, poses and objects show the activities. Short
speech bubbles replace the large prose panel. Specific picture buttons remain
visible and readable within the circular display; tapping floor moves the doll,
and people and Park places can be touched directly.

## Playtest-driven changes

The [first independent run](blackbox/first-pass/README.md) used ordinary visible
input and identified weak School agency, an inert invalid lunch order and a Park
clue that proposed the wrong search location. Its evidence is explicitly
superseded, not final acceptance evidence.

- School now includes editable picture hanging: choose Flowers, Rocket or
  Rabbit, fill three frames and explicitly finish. Selection, replacement and
  unfinished arrangements persist. Distinct plans and cumulative repair remain.
  Reopening Arrange pictures preserves the creation; only a new outing resets it.
- Invalid lunch choices lead directly back to the editable pictured menu with
  an explanation. Plus/minus controls never wrap a quantity. The tray is clear
  of the doll's default position, and price/quantity arithmetic remains visible.
- An inspected Park place becomes the proposed search, with other hypotheses
  still available. The ribbon is spatial evidence; returning the notebook
  visibly gives it back to Nori.
  The final scene placement keeps the notebook/ribbon visible after the doll
  approaches. Change my idea immediately advances to a different place.
- Reviewer findings also fixed repairs undoing each other, old scrapbook entries
  borrowing the current outing's scene, and completed Cafe conversations opening
  menus whose labels no longer matched their actions.
- Native-frame inspection caught missing card pictures after label wrapping.
  Measured typography and portraits retain every main School illustration; a
  regression test now checks rendered picture space and actual illustration
  pixels, plus distinct Cafe food pictures.

The later child-perspective run also caught premature "Practice again" wording.
The first practice action now actually practices with Mina; only subsequent
practice uses "again". Reusable playtester instructions now separate voluntary
stopping from coverage-driven persistence and require an explicit gesture and
interruption matrix.

See [final observed evidence](blackbox/README.md) for exact builds, ordinary
paths, prompt revisions, findings/resolutions, screenshots and limits. Build,
coverage, firmware and review results are recorded in [validation](validation.md).
The [design contract](design-contract.md) describes the intended behavior.

## Persisted state and limits

V3 is 100 bytes. Shipped V1 (88 bytes) and V2 (92 bytes) retain strict independent
header, size, CRC and historical-range checks before migration. V3 appends
walking position, borrowed paper and the picture editor; it extends the existing
repair values additively so both friends' repairs remain visible. No namespace,
partition, old field offset or historical enum ID changes.

Scrapbook entries remain compact place/outcome/caption/sticker summaries. Their
illustrations use the remembered place and outcome; they are not saved bitmap
photographs, and do not store each past lunch order or picture arrangement.
The current display arrangement is saved for the active outing. Characters use
a small original pose library and simple walking animation suitable for the
ESP32-S3. Actual child playtesting and physical display/touch evaluation remain
separate from agent roleplay in the emulator.
