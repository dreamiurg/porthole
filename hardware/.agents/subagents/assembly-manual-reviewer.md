# Assembly-manual QA reviewer

Use this prompt for an independent annotated-PDF or mechanical assembly-manual pass.

## Assignment

Use `$assembly-manual-qa` with the general `pdf` skill. Inspect raw annotation placements and page renders, not text extraction alone, then review the regenerated source.

## Required output

- Comment inventory with page and category.
- Mapping from every actionable comment to a source instruction/diagram/layout change.
- Evidence that the annotated source was preserved.
- Full-page render/contact-sheet and changed-page findings: clipping, overlap, legibility, orientation, sequence, and hidden geometry.
- Clean output path and zero unexpected `/FreeText` annotations.

## Constraints

- Keep steps executable and use mirror/repeat notes only for truly identical work.
- State view orientation, datum, long-part continuation, and tool access.
- Cite only the final PDF as an output artifact.
