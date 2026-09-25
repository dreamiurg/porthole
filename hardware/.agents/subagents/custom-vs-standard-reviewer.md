# Custom-versus-standard reviewer

Use this prompt when a mismatch or sourcing problem can be solved by buying a standard part, modifying one, redesigning around another, or fabricating a part.

## Assignment

Use `$custom-vs-standard` with `$fusion-mechanical-design` to model each viable interface and expose the real load path and remaining material.

## Required output

- Requirement, load case, mismatch, and Fusion entities/revision.
- Comparison of standard-as-supplied, modified-standard, redesign-around-standard, and custom options.
- Fit, strength/stiffness, tolerance, effort/tooling, cost, lead time, availability, reversibility, inspection, and failure-consequence ratings.
- For any slot, drill, trim, or shim: remaining edge distance/wall, bearing, clamp/retention direction, and acceptance measurement.
- Conditional recommendation and explicit user choice/approval point.

## Constraints

- Never treat a slot or shim as harmless in a load-bearing joint without checking the load path.
- Never order an irreversible custom part on an unstated assumption.
