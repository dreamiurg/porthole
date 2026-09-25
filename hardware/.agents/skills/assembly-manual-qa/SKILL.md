---
name: assembly-manual-qa
description: Create, revise, and visually verify an IKEA-style mechanical assembly manual from HTML or similar source and annotated PDFs, with executable steps, clear diagrams, and traceable comment resolution.
---

# Generic mechanical assembly-manual QA

Use this skill with the general `pdf` skill when a build manual, annotated review PDF, or rendered assembly guide must be revised. Treat the source document and the user-annotated PDF as separate inputs; never overwrite the review evidence before extracting it.

## Workflow

1. Identify the source HTML/document, input annotated PDF, intended output PDF, assets, and revision identifier. Keep all paths configurable; do not assume a particular machine, process, or part family.
2. Preserve the annotated PDF and extract every `/FreeText` comment, including page and rectangle. Separate test annotations from actionable content, drawing, sequence, and layout corrections.
3. Map each actionable comment to a source change and acceptance check before editing. Keep an inventory so no comment silently disappears.
4. Write concise, executable steps: state the view and orientation, identify handed parts and datums, name the exact hardware, state the tool/access constraint, and use one mirror/repeat note for genuinely identical work.
5. Use code-native SVG/CAD-style diagrams for dimensionally meaningful relationships. Label hidden geometry, load direction, fasteners being installed, break lines for shortened long parts, and any temporary support. Use raster generation only when a non-dimensional illustration is explicitly desired.
6. Immediately before the first PDF create/edit operation in a turn, run the PDF artifact marker exactly once with the correct operation kind and expected output count.
7. Render the source to PDF, render every page to images, and inspect the contact sheet plus each changed page at full size. Re-render after every meaningful change.
8. Verify page count, page order, headings, revision/footer, readable labels, no clipping or overlap, correct mirrored-step logic, and zero unintended annotations in the clean regenerated PDF.
9. Deliver only after the comment inventory is closed or explicitly marked unresolved. Cite the final PDF once as an output artifact; do not cite scratch renders.

## Content invariants

- Every irreversible operation states the required measurement, tool, and acceptance condition.
- A diagram must identify its view; geometric accuracy alone is not enough.
- Long parts use continuation/break notation rather than implying a shortened part.
- A translucent or hidden part never carries the only critical instruction; explain it in labels or prose.
- Assembly order must be physically executable: no fastener, bearing, cable, or captured component may be installed through an impossible obstruction.
- If two steps are truly identical, use a clear mirror/repeat note; if handedness or access differs, write the distinct step.

Read [manual-rules.md](references/manual-rules.md) for the reusable content and review checklist. The helper scripts accept project-relative paths and do not encode a particular machine.
