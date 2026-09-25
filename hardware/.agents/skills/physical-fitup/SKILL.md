---
name: physical-fitup
description: Compare as-built measurements with the current Fusion model for assemblies made from commercial parts, while separating datum error, skew, tolerance, and genuine design mismatch without forcing the fit.
---

# Physical fit-up and as-built verification

Treat a physical fit-up question as a controlled measurement problem, not a visual guess. Separate the measured surfaces, current Fusion source/version, fabrication basis, and assembly condition before deciding whether a part needs redesign.

## Fit-up workflow

1. State exactly what each measurement touches: inside or outside face, rail or extrusion face, cross-member end, hole center, fastener center, or other datum. Do not compare an inside measurement with an outside CAD span.
2. Identify the current Fusion basis with `$fusion-mechanical-design`. Keep an older drawing, purchase dimension, and current model as separate evidence until reconciled.
3. Remove avoidable assembly error: support heavy or flexible parts, keep mating plates at the same station, make the relevant parts parallel, and measure without pulling them together with fasteners.
4. Measure at least two locations along the joint and, where practical, at two heights or diagonals. Report the spread; it is evidence of skew, twist, or nonparallelism, not random noise to average away.
5. Record instrument resolution and repeatability. Use a tape or rough ruler for triage only; use calipers, a bar/rod, pins, or a controlled gauge for a CAD-versus-as-built decision.
6. Check the upstream structure first. A changed member length, rail span, spacer stack, or end-face datum can explain a downstream gap without any plate being wrong.
7. Compare the measured functional datum with Fusion. Report total delta, per-side or per-interface delta when meaningful, and whether the result is within the stated assembly tolerance.
8. If the result is outside tolerance, stop before ordering or final tightening. Choose explicitly among remeasurement, correcting the base, updating the model, adding a deliberately designed shim/interface, or changing a part.

## Acceptance language

- **Triage only:** rough measurement or a single point with unknown surfaces.
- **Repeatable:** multiple readings on identified datums with a stated uncertainty.
- **Skew/nonparallel:** the gap changes along the joint; do not collapse it into one average.
- **Free motion:** moving elements travel their range without tight spots after the measurement setup is restored.
- **No preload:** bolts may clamp intended joints, but must not bend plates or rails into a nominal dimension.

Use [fitup-checklist.md](references/fitup-checklist.md) for the generic measurement layout and decision record.
