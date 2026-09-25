---
name: fusion-mechanical-design
description: Read-only Fusion MCP design and verification for assemblies built from commercially available parts, including exact interfaces, datums, clearances, fit-up, and fabrication-release checks.
---

# Commercial-parts Fusion design and verification

Use the active Fusion document as the geometry authority for the question at hand. Treat screenshots, exported drawings, BOMs, and older conversations as historical evidence until the document, design version, and dimensional basis are identified.

## Workflow

1. Define the decision: the use case, loads and constraints, purchased-part assumptions, fabricated parts, and the measurement or interface that must be trusted.
2. Read the active document with `mcp__fusion__fusion_mcp_read` (`document`, `operation=open`). Record the document name, active/modified state, and version or source identifier when exposed.
3. Use `mcp__fusion__fusion_mcp_execute` with a read-only Python script. Do not save or alter the Fusion document unless the user explicitly authorizes that action.
4. Identify exact occurrence/component/body names before measuring. Report entity paths, not naked numbers.
5. Measure functional datums directly: planar mating faces, hole centers/axes, bore axes, rail centers, and travel limits. Use bounding boxes only for envelopes and sanity checks.
6. Convert Fusion's centimeter API units to millimeters explicitly (`value * 10`) and state the axis, faces, or axes used.
7. Verify interfaces in the load path: fastener access, thread engagement, edge distance, handedness, clearance, interference, motion, assembly order, and whether a purchased part actually matches its drawing.
8. Compare competing release bases explicitly. If the live model and a drawing or as-built measurement disagree, report both and stop short of fabrication advice until the source of truth is chosen.
9. Return measured values, calculations, tolerance/uncertainty, practical consequence, and the smallest safe next measurement or model action.

## Measurement patterns

- **Inside gap:** subtract the two opposing functional datum faces, not two arbitrary outer faces.
- **Plate thickness:** measure the two faces of the same plate at a controlled location.
- **Outside span:** use outer faces only when the specification calls for an envelope dimension.
- **Purchased interface:** measure both the envelope and the functional datum (mounting face, hole pattern, bore axis, or locating shoulder).
- **Interference:** distinguish overlap, zero clearance, and a positive gap at the intended contact pair. Do not hide a mismatch by moving unrelated geometry.

## Safety and reporting

- Keep audits read-only unless a design edit or save is explicitly requested.
- Do not force a structure, rail, or plate into position to make a CAD number match a drawing. A preload-free assembly is the acceptance condition unless the design intentionally specifies preload.
- Preserve the provenance of every number: live Fusion measurement, ordered-part drawing, as-built measurement, or historical release.
- Do not call an assembly fabrication-ready without checking interfaces, handed parts, hole counts/axes, access for tools, no staging geometry, and the relevant clearance/interference invariant.

Read [measurement-patterns.md](references/measurement-patterns.md) for reusable datum and release-basis rules.
