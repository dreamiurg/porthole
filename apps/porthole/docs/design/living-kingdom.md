# Living Kingdom: the playable Willowbrook chapter

This implementation expands the approved Chapter One handoff into forty complete
scenes of original fan-game prose. The audience is an advanced third-grade reader.
The core characters come from Princess Pulverizer; all local characters, scenes,
choices, and dialogue here are original. This is the private, unofficial prototype
described in the source brief, not an authorized adaptation of book text or art.

The production manuscript and world rules live in
[`games/living-kingdom/content/chapter-01.json`](../../games/living-kingdom/content/chapter-01.json).
`tools/content.py` validates and compiles this file into flash-resident C++ tables.
The JSON is the editable source; there is no JSON parser or generated storytelling
on the device. Narrative and rule authoring do not require editing the renderer.

## Causality and the four routes

An earlier upstream flood damaged the bridge footing. A subsequent dry spell
reduced the water available to mill and marsh. Bram brought replacement stones;
Elric's wheel repair held. Neither the troll nor the lost washer is evidence of
sabotage. Flour exists, but not enough to fill every order: ten usable bags against
fourteen requested. Pantry oats and beans keep meals available while delivery and
employment costs remain real. No one is left unfed as a punishment for a choice.

The council requires hearing Elric, hearing Fen, and investigating the cracked
bridge. All three principal answers remain possible for every seed. The fourth
requires the tower drawing and Tilda's reservation of the shared timber and crew.
A player can postpone the council to investigate. A reserved option is not a
construction commitment; its real cost begins only when the council chooses it.

| Plan | Delayed result | Required follow-through | Ending retains |
| --- | --- | --- | --- |
| Mill first | Flour improves; nesting pools grow shallow after two world days | Prepare the nursery with two days of carrying, or borrow a mill turn and work one day. With earned Courage and Bram's trust, recruit his help at the cost of his deferred maintenance. | A smaller marsh and continuing nursery checks; optional unfinished work |
| Marsh first | Pools hold; bakery hours and flour remain limited after two days | Trade a flour parcel plus a day's work, or earn relief through two days of carrying. Earned Ingenuity offers a relay that delays Nell's next trip. | Reduced baking, varied meals, and slower delivery arrangements |
| Timed sharing | Handovers fail to account for batches and pool depth after two days | Spend a day training paid joint watchers, or test longer turns over two days with nursery carrying | Waiting, ongoing checks, and a routine people can revise |
| Second channel | Bridge work loses its resources; a storm tests the new gate after two or three days | Pay apprentice wages with supplies/work, then spend one or two days finishing safe gate repairs and a paid maintenance plan | Bridge reinforcement and school work explicitly deferred; continuing gate costs |

The channel is not a perfect ending. It involves two substantial follow-through
scenes after the delayed return, spends the bridge's resources, and leaves a
visible deferred bridge quest. The other three routes can repair the bridge
through separate inspected work. A deed is recognized only after the chosen
route's follow-through, never for preferring a particular council answer.

## Scene graph and stable identities

Scene IDs are explicit integers; their string keys remain readable authoring
identities. Choice IDs are `scene * 10 + option`. Already released IDs must not be
reused or reordered. `content/ids.json` locks registry identities; new entries
append. Original design-fixture aliases are recorded in `fixtureAliases` in the
JSON. The fixture was not a shipped save format, so its incomplete graph is
normalized here rather than treated as runtime data.

| IDs | Content |
| --- | --- |
| 1–8 | Arrival, village/Pippa, mill/wheel, Bram/Bea, Lucas's inspection |
| 9–12 | Woodland clue and seed-selected fox, parcel, or goose revisit |
| 13–15 | River inspection, Fen's account, tower drawing |
| 16–21 | Flour allocation, trade, cooking, Tilda's costs, bridge work, corrected accusation |
| 22–23 | Council, including explicit deferral, and the return commitment |
| 24–25 | Mill-first consequence and nursery restoration |
| 26–27 | Marsh-first consequence and village relief |
| 28–29 | Shared-water consequence and tested handovers |
| 30–32 | Channel consequence, paid apprentices, inspected storm repair |
| 33–34 | Optional tower evidence and shared deed recognition |
| 35–38 | Four separate persistent outcome epilogues |
| 39–40 | Seeded revisits tracing and correcting the first bridge rumor |

There are six map locations. The fixture's seventh `loc.river` is a riverside
encounter within Marsh; the council meets in Village. Those remain different
scenes and functions without requiring a seventh location target. Fixture
`return.changed` and `quest.resolution` are expanded into route-specific scenes.
`chapter.final` maps to `chapter.finale`; epilogues are four distinct nodes.

## Systems expressed through authored choices

Inventory has six types and six slots: bread, rope, timber, thyme, flour, and the
channel map. Essential evidence is a journal fact even when its object is not
carried. No ending requires a rare item: labor alternatives always remain open.
Thyme replaces time in a cooking task; flour replaces part of delivery or wage
work; rope improves supervised bridge or gate work and can be left as equipment.
Timber is a world reservation for this chapter rather than a randomly found key.

Courage, Ingenuity, and Insight record milestones without a moral total. Practical
bridge work increases Courage; observing repairs or learning a kitchen method
can increase Ingenuity/Insight. Courage plus Bram's trust unlocks a different
nursery work arrangement. Ingenuity unlocks a relief relay with its own cost.
Nell's trust offers a shared delivery shift and useful spare equipment; Bram's
trust permits borrowing rope. Main progress always has an ungated method.

Promises retain their original dates. Each completed river route fulfills the
party's optional commitments to return to Elric and help Fen. These effects do
not invent promises the player never made. The separate supply promise requires
an actual delivery arrangement; apprentice and gate promises require channel
follow-through. A finale does not silently resolve unfinished bridge or supply
quests. Quest objectives distinguish active, deferred, blocked, and resolved
work in ordinary language. Evidence names and descriptions never require a
reader to interpret internal keys.

Four campaign variants are selected once and preserved: the low woodland path
or a longer safe hillside route; Elric or Nell as the initial rumor source; a
channel storm inspection after two or three days; and a fox, parcel, or goose
woodland encounter. Route and storm variants affect world time, rumor variants
change whom the player can help correct the account, and woodland encounters
have different practical opportunities. They do not remove a principal ending.

## Reading, revisits, and safety

Each scene contains 150–350 words, split into natural paragraphs. Choice results
provide a separate account of what the committed action accomplished. Reading,
paging backward, opening an overlay, and rereading do not advance days or apply
effects. A saved result cannot award the same item or deed twice. The council
uses a separate confirmation and has an explicit investigation option.

Meaningful travel and explicit work/rest advance world days. Real absence does
nothing. Results have no injury, death, real-time deadline, hidden virtue score,
or forced loss. Children do not move threatened eggs or work over rushing water;
Fen and the skilled crew do the specialist work while the party helps safely.

The four epilogues describe lasting differences, not a generic victory followed
by a dead end. They remain available as cooldown revisits. The royal highland
inspection note is a lead for another chapter, clearly marked as an unanswered
question. Local exploration and optional work remain accessible after the deed.

## Current visual direction and evidence scope

The user selected
[08 — Chapter One: Polished UX](../../../../docs/superpowers/specs/living-kingdom/concept-art/08-chapter-one-polished-ux.png)
as the visual target. The original PNG package is committed unchanged in the
design branch. The SVG is a schematic, not the artwork source. The game uses
actual concept-art crops, with GPT-generated additions for missing illustrations:
rich pixel-fantasy scenes and expressive companions, near-black indigo framing,
textured parchment, gold actions and violet secondary cards. Message text uses
the exact PR #80 Atkinson Hyperlegible font for readability, with decorative
serif headings. The rejected drawn artwork and condensed font are superseded.

The reading design gives the opening page an illustrated setting and a lower
paper area, then gives continuation pages more paper space. Party and inventory
need visible overviews of all three companions and all six slots, with readable
details on selection. A clearly labeled Menu provides Save & quit while the
footer retains reading navigation. These are presentation requirements; they
do not change council choices, resource costs, promises, or save semantics.

The [final emulator reports](../living-kingdom/blackbox/README.md) identify the
frozen build and actual ordinary-input campaigns, exact page recovery,
consequences, endings and post-ending exploration. Their earlier rejected
presentations remain explicitly archived as historical functional evidence.
[Original versus final comparisons](../living-kingdom/visual-comparison/README.md)
show the selected artwork beside actual native screens and document deliberate
layout changes and source-resolution limits. No physical-device test is claimed.

## Validation boundaries

The content compiler checks keys, references, glyphs, explicit IDs, predicates,
and generated-table freshness. Engine tests exercise all four routes across
representative seeds, item-free follow-through, event timing and atomicity,
quest/promise persistence, and save/resume. Runtime reader tests measure the full
manuscript with the actual font and round-screen layout. These checks complement
ordinary-input emulator playthroughs and screenshot inspection; they do not
claim a physical-device test or replace reading the story for causal coherence.
