---
name: commercial-parts-bom
description: Reconcile a commercial-parts bill of materials with Fusion interfaces, as-built parts, fasteners, spacers, kits, quantities, thread engagement, and fabrication or procurement status.
---

# Commercial-parts BOM and hardware reconciliation

Start from the current BOM and live Fusion interfaces, then verify each order-critical item against the purchased part or its authoritative drawing. A BOM is a release aid, not permission to ignore a newer model, measurement, or vendor revision.

## Reconciliation workflow

1. Identify the joint or subsystem: parts, mating faces, load direction, hole type, thread standard, access, and required engagement.
2. Give each line a status: **ORDER**, **KIT-CHECK**, **CONDITIONAL**, **FABRICATE**, **MODIFY-STANDARD**, or **VERIFY**. Keep the exact assembly quantity separate from purchase quantity and spares.
3. Calculate the stack: clamped material, washers, spacers, brackets, counterbores, and any unthreaded clearance. Choose screw length from usable engagement, not from nominal part labels.
4. Verify fastener type, diameter, pitch, head and washer fit, edge distance, access, and engagement in the actual mating interface.
5. For substitutions, require the same functional hole pattern, bearing face, stiffness, parallelism, load path, and usable thread. Nominal size alone is not equivalence.
6. Treat kit contents, vendor lead times, and cut-to-length services as unconfirmed until checked against the supplied packing list or current product page. Hand off current availability questions to `$parts-availability`.
7. Hand off ambiguous manufactured-versus-bought choices to `$custom-vs-standard`; show the user the alternatives rather than silently turning a purchased part into a custom part.
8. Report missing information as a verification item. Do not invent a SKU, quantity, inclusion, or compatibility claim.

## Spacer and shim rules

- Tube standoffs, precision flat plates, washers, and thin sheet are different interfaces. A stack is acceptable only when the sum, bearing faces, bore/hole pattern, parallelism, stiffness, and retention are controlled.
- A dimensional stack that is acceptable for positioning may be unsuitable under a guide, bearing, or other concentrated load.
- Do not sand or bend stock to a precision thickness when face parallelism, flatness, or preload matters; use a known stock thickness, machining, or controlled shims and record the tolerance.
- Every spacer line should state material, finished thickness, flatness/parallelism need, holes/bore, quantity, and whether it is temporary or release-grade.

## Tapping and installation

- A holder is not a tap set: confirm the correct tap geometry, drill size, depth, alignment method, cutting fluid, chip-breaking procedure, deburring, and final screw-fit test.
- Separate threadlocker and anti-seize decisions from guideway, bearing, and ball-screw surfaces. Never contaminate rolling or sliding surfaces.

Read [bom-rules.md](references/bom-rules.md) for the reusable line-item schema and review gates.
