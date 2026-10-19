# Extensible content and state contract for Codex

This is a design contract, not mandatory C++/JSON runtime wire format. Prefer compact compiled/static data tables and indexed stable IDs suitable for ESP32-S3 firmware. The example `chapter-01-world.json` is a QA/spec fixture for implementing and validating the engine, not required to be parsed as JSON on-device.

## Requirements for narrative node

- `id`: globally stable opaque identifier; never change after shipping. `chapter`, `location`, `kind`.
- `requires`: declarative boolean predicate over facts, items, prior decisions, quest status, skills, relations, promises, day or a seeded variation. Avoid arbitrary C++ conditionals scattered throughout UI code.
- `once` / `repeat` policy: one-time / repeat with world variant / repeat with cooldown. One-off scenes must not replay as if new.
- `pages`: an array of paragraphs/text, or deterministic pages generated from one immutable full scene text. Page count/page index reflect measured font geometry. Do not persist text bytes, only node ID and cursor.
- `choices`: ID, caption, optional description, `requires`, optional preview of known stakes, one set of `effects`, `next` scene. Include choice for leaving/deferring only when story allows it. Persist chosen ID before applying next scene.
- `effects`: atomic set/delete facts, item delta, quest state transition, relation delta, in-game day advance, promise create/fulfil/break, schedule future event by relative day or world predicate, link to response scene.
- `journal`: bounded human-readable event ID and arguments, recorded once; history should remain readable after updates even if active quest changed.
- `version`: explicit content revision and save migration strategy. Added content should not invalidate old world seeds, IDs or decisions.

## Save design

At minimum: magic, version, byte length, CRC, profile slot, stable seed and PRNG state, world day/time phase, current location/node/read-page, pending-choice identity, game mode/overlay origin, objective state, fact flags/IDs, inventory IDs/quantities, skill values, relation values, promise flags, scheduled events, seen scene IDs, story log IDs and last major outcome. Keep bounded and compact, e.g. stable enum IDs / bitsets / small arrays; **do not store prose** or pointers.

Atomicity: choose once → commit effects in deterministic order → save complete snapshot. Interrupted write must load the last valid complete state, not duplicate rewards. Preserving page index on shell rest/exit is required. When chapter adds new IDs, append new fields and migrate old snapshots safely; do not reorder existing fields or rename shipped NVS namespaces.

If the persistence budget prevents saving an unbounded detailed journal, preserve summaries, dated decision IDs, facts and outcomes; the prose can be reconstructed from stable content IDs. Do not silently discard the major decisions needed to make a returning player feel remembered.

## Deterministic generator

World initialization chooses a seed per profile and from that a few variant facts:
- Which old route is usable (forest footpath / watchtower climb)
- Who is the first person to express suspicion (Elric / Nell)
- Whether the first storm lands on day 4 or 5
- Which small side encounter appears (fox / lost parcel / grumpy goose)

These variations must **never prevent access** to needed evidence, derail quest completion, contradict persistent NPC knowledge, or produce irresolvable dead ends.

Every optional encounter has a weight, cooldown / seen policy, preconditions and stable ID; selection is deterministic given state and seed. Major story beats take precedence. Maintain coverage: every world seed should permit at least one Chapter One resolution without relying on a lucky roll.

## Skills and player agency

Courage, Ingenuity and Insight begin at levels representing Serena's existing character (e.g. 3/2/2) and can improve at deliberate milestones. Scores unlock believable methods; no random binary skill checks where a high score can still fail without explanation. Lucas and Dribble offer advice because of their personalities, not simply because the player optimized their stats.

Relationships are per-NPC states, preferably stored as compact integer plus significant flags. They can create additional dialogue opportunities or help, not a visible morality meter. **No kindness score**.

## Quest graph

Chapter One tracks three core threads, their prerequisites and resolution:
- `quest.bridge`: inspect→Bram dialogue→repair or negotiate reopening; may remain temporarily deferred until resources available.
- `quest.supplies`: understand shortage→allocate/seek supplies→respond to outcome.
- `quest.river`: hear miller and Fen→discover river context→choose project→view consequence and do follow-up.
- `quest.chapter`: activated on arrival; complete after major resolution + at least one follow-up + deed recognized. Does not require every side quest or the objectively impossible satisfaction of everyone.

Status values: undiscovered, available, active, blocked, deferred, resolved. Quest objective text reflects actual state and should never claim completion before effects commit.

## Validation and testing

Create a validator to ensure:
1. No duplicate ID, broken next-node reference, unknown fact/item or unreachable location.
2. Every decision has at least one available choice in every reachable state.
3. No post-commit repeat awarding items/day or re-rolling events when the player rereads.
4. At least three distinct major outcomes are reachable from representative sequences.
5. Delayed events fire after defined days, exactly once, with consistent scenes and journal.
6. Save/load reproduces exact page cursor, pending choices, inventory, relationships and world day.
7. Two profiles with different seeds/progress remain isolated.
8. All text fits circle at supported font size, with hitboxes and full reread loop tested via simulator.
9. Chapter One can complete for all supported initial variations without traps.
10. World seed and choice trace allow replay from emulator debug commands for reproducible bugs.

Good test trace: Start → page 1 → page 2 → choice → **REREAD** → page 1 → page 2 → return choice → different option → confirm → journal update exactly once → save → reboot → revisit on day+2 → consequence shown once.

Future chapters add content and occasionally mechanics using existing abstract state/effects; no rewriting every prior scene.
