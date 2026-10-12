---
name: custom-vs-standard
description: Evaluate whether an assembly should use an off-the-shelf part, a modified standard part, or a custom fabrication, using Fusion interfaces and explicit load, cost, lead-time, and risk trade-offs.
---

# Custom versus standard-part decisions

Use this skill whenever a design gap, hole mismatch, spacer, bracket, or availability problem can be solved in more than one way. The goal is not to maximize custom fabrication or to force every design onto catalog parts; it is to make the trade-off visible and let the user choose with evidence.

## Options to compare

1. **Standard as supplied:** buy and use the catalog part without geometry changes.
2. **Standard modified:** buy a verified part and perform a defined operation such as slotting, drilling, trimming, shimming, or adding a spacer.
3. **Custom fabricated:** make or commission a part whose geometry, material, and tolerance are specified for the interface.

Include a fourth option, **redesign around another standard part**, when it removes a risky modification at reasonable cost.

## Evaluation workflow

1. Capture the functional interface in Fusion: datums, hole centers/axes, load path, fastener access, edge distances, clearances, travel, and assembly sequence.
2. Define the mismatch or requirement precisely. A hole that misses a T-slot, for example, is a geometry problem; it is not automatically permission to enlarge a hole or weaken a bracket.
3. Model each viable option sufficiently to verify the interface. For a modified part, represent the actual slot/hole/shim and remaining material, not an idealized original.
4. Score each option for fit certainty, strength/stiffness, tolerance control, work and tooling, cost, lead time, availability, reversibility, inspection effort, and failure consequence.
5. Check modification-specific limits: minimum edge distance, remaining wall, fastener bearing, washer support, slot length and end radii, clamp load, fatigue/slip risk, corrosion/finish, and whether the modified part remains identifiable and inspectable.
6. Present the options in a decision table with a recommendation conditional on the stated loads and priorities. Ask the user to choose before an irreversible custom order or modification.

## Guardrails

- A slot is acceptable only when the fastener still has a positive load path and adequate bearing/retention in the intended direction. A slot must not turn a structural joint into friction-only support by accident.
- Do not modify a purchased guide, bearing, safety-critical, or highly loaded part without checking the maker's restrictions and the resulting tolerance/strength.
- A workaround that preserves existing fabricated holes can be sensible, but it must be modeled and inspected as the new design. State what is gained (avoided re-fabrication, low cost, reversibility) and what is lost (material, stiffness, adjustment, inspection, or future interchangeability).
- Custom parts are not automatically stronger; they add drawing, process, inspection, and lead-time obligations.
- Standard parts are not automatically safe; confirm the real drawing, material, load rating, and interface.

Use [choice-matrix.md](references/choice-matrix.md) for the comparison table and approval record.
