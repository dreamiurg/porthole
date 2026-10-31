# Reading, choices and persistence — interaction contract

## Navigation state machine

Title → campaign (new/resume) → map / location / encounter → reading pages → decision → consequence → world map. Inventory, party and journal are reachable from the map and encounter header, and return to exactly the scene/page from which they were opened. All locations and persistent world state must survive app exit, sleep, firmware upgrades and child profile switching.

### Long text

- Default **tap Back / Next** pagination; swiping is optional. Don't implement free scrolling unless measured on the physical screen and rereading is just as comfortable.
- Break a 180–250 word event across several pages (typically 3–5 at readable font size on a 2.1-inch circle). Story can contain multiple paragraphs, dialogue, humor and clues. Avoid word limits that force narrative into one-line blurbs.
- Navigation never mutates story variables. Only committing a decision does. Back on first page may return to prior scene/map without forgetting progress, as appropriate. The current scene and its page remain accessible after a temporary visit to party/inventory/journal.
- A **Read from Start** action on the choice screen must return to page 1, allow pages to be revisited, and then return to the exact same pending choice list, including the same originally rolled outcome options. No re-roll, no duplicate item award, no changed clock.
- **Choices follow reading** on a separate screen with 2–3 child-sized cards and short summaries, pagination for extra options. A small 'Why?' indicator can reveal known stakes without spoiling secrets. A high-stakes choice has a gentle confirmation that allows rereading and switching options before commit. No decision is committed just by tapping a link while paging.
- Once a choice is committed, the journal retains a concise narrative record and the world moves forward. **Past actions remain committed**, but the player may later make amends in new scenes.

## Proposed 480x480 safe zones (illustrative, measure in emulator)

- Circular physical radius 240px. At y=52 or 428, safe horizontal width is only around 300px. Restrict headers/buttons there; wider elements at center y=150–350. Text must avoid beveled edge.
- Reading layout: small centered title y≈56, scene image vignette y≈90–160 (optional and omitted on text-dense pages); cream prose panel x≈70–410, y≈115–368, using a word-wrap system that measures actual glyphs; bottom previous/next buttons around x≈145/335 at y≈395, each a true tappable region at least ~72px high/width and fully in circle. For title-less text continuation pages, allow extra prose space at top.
- Use visible **page n/m** indicator and an unambiguous way to return to the same choice screen after rereading. Provide a **Journal**/read-history affordance on the map; avoid text disappearing forever after an option is picked.
- Choice cards: up to 3 rows, near-center, each ≥72 physical px tall (rough planning: y 134–209, 218–293, 302–377), rounded but pixel-inspired, contrast sufficient under typical backlight. Backstory link should remain outside decision targets and inside circular mask; do not put five options in tiny rows.
- Tests must log bounding-box safety and tap accuracy, ensure no glyph or button cuts through circular bezel, check 480 screenshots and evaluate at **actual 2.1-inch size**.

## Initial interface screens

1. Title: colorful kingdom scene, Princess and companions, **CONTINUE** / **NEW ADVENTURE**. For new game introduce party in a few reading pages.
2. Map: river cutting the region; Willowbrook, bridge, mill, woods, marsh, tower. Routes and alerts show world status; tapping a location gives a text preview rather than instant irreversible travel.
3. Story page 1/3: picturesque village vignette plus 1–2 paragraphs of genuine narrative. Back/Next at bottom.
4. Story page 2/3 (more reading): no need for full-size picture; comfortable longer prose and speaker identifiers.
5. Decision: title **WHAT DO YOU DO?** with three distinct options and **READ AGAIN**. Some options visible only when information available.
6. Party: portraits of Princess, Lucas, Dribble; Courage/Ingenuity/Insight and companion specialties; options indicate requirements in natural language.
7. Inventory/Journal: six item cards, current quest threads, clues and promises; journal includes dated consequence notes.
8. Return to village after advancing in-game days: river/bridge illustration differs, NPC dialogue references prior decisions.
9. Save/resume: auto-save after every committed choice and on safe navigation. Resume exactly same page, including before an uncommitted choice.

## Primary UX risks

- **Reading fatigue:** illustration must not squeeze prose into a tiny unreadable box. Offer alternating illustration+text pages and text-only pages.
- **Navigation trap:** every reading page and choice screen needs a visible route back, with no accidental progress mutation. Tests must simulate 1→2→3→choice→re-read→1→2→3→choice then confirm once.
- **Session interruption:** shell rest caps can occur mid-passage; store current page, not just quest ID. Returning to game should not force rereading or skipping.
- **Tiny taps:** all interactive glyphs need larger hit regions than their decorative pixel outlines.
- **False fairness:** no all-powerful 'ask everyone / compromise' choice without trade-off. Clarify what the player could reasonably know about consequences.
- **Different age abilities:** content targets advanced third grade; parents can read with younger children. No violence, gore, timed choices or permadeath.

## Starter screen reference

[starter-screens.svg](starter-screens.svg) is a **design sketch** of six round screens. It should be a source of visual composition and mood, not treated as final shipped sprite art or proof of fit. The richer ChatGPT-generated boards remain additional inspiration and were not included as raster files.

## Chapter extension protocol

A later story package adds stable IDs, locations, prose blocks, choice rules and effects to a content registry. Replacing prose should not alter already persisted state identifiers. Existing characters and relationship changes must remain visible, not silently reset. Chapter boundaries are not save boundaries; a child retains all items, promises and history.

For new mechanics: introduce behind an explicit schema version, provide migration and tests, and ensure old Chapter One endings still replay deterministically after update. Prefer one new mechanic per new chapter only when playtest feedback warrants it.
