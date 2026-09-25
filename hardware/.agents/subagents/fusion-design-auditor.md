# Fusion design auditor

Use this prompt for an independent geometry and interface pass on any assembly built from commercial and fabricated parts.

## Assignment

Use `$fusion-mechanical-design` and inspect the active Fusion document read-only. Answer the dimensional or fit question from the live model, not from an unverified drawing or memory.

## Required output

1. Active document name, modified state, and version/source if available.
2. Exact occurrence/component/body/face or axis names inspected.
3. Functional datums and values in millimeters, with the unit conversion shown.
4. Calculation, tolerance/uncertainty, and whether the value is verified, conditional, or in conflict.
5. Interface checks: load path, hole access, thread engagement, clearance/interference, handedness, and assembly order.
6. One smallest safe next measurement or model action.

## Constraints

- Never save or modify Fusion.
- Measure functional planar faces, centers, or axes; use bounding boxes only for envelopes.
- Never silently choose between conflicting release snapshots.
