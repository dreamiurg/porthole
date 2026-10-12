# Commercial-parts Fusion assembly harness

These recipes are project-neutral. They coordinate the generic skills; they do not replace a live model, a vendor drawing, or an as-built measurement.

## Session and Beads

```powershell
bd prime
bd ready
bd search "keyword"
bd show <id>
bd update <id> --claim
bd close <id> --reason="Completed and verified"
```

Create an issue before changing a CAD release, BOM, or delivered artifact. For review-only questions, report evidence first and keep the source unmodified.

## Fusion read-only audit

Use `mcp__fusion__fusion_mcp_read` with `queryType="document", operation="open"` first. Then use `mcp__fusion__fusion_mcp_execute` with a Python `run(_context)` script to inspect occurrence/body names, bounding boxes, planar faces, cylinders, hole axes, and interference/clearance. Convert Fusion centimeter API units to millimeters with `* 10`. Do not call Fusion save unless explicitly authorized.

For an opposing planar interface, measure the two functional faces rather than relying only on occurrence bounding boxes:

```text
datum_a = functional face/axis on component A
datum_b = functional face/axis on component B
interface_value = signed distance(datum_a, datum_b)
```

Record entity paths, axis direction, release/version, calculation, and uncertainty.

## Physical measurement check

```powershell
& '.agents\skills\physical-fitup\scripts\compare_measurement.ps1' `
  -Label 'inside interface gap' -MeasuredMm 740 -ModelMm 738.32 -ToleranceMm 0.50
```

The script reports the signed delta and a stop/recheck recommendation. Use multiple physical points and `$physical-fitup` when the joint may be skewed; never use bolts to bend parts into agreement.

## BOM and sourcing

Use `$commercial-parts-bom` to reconcile each interface and quantity, `$parts-availability` for current regional stock/lead-time evidence, and `$custom-vs-standard` when a catalog part may be modified or replaced with a fabricated part. Keep exact SKU, pack size, evidence date, and user approval separate.

## Annotated manual extraction

```powershell
python '.agents\skills\assembly-manual-qa\scripts\extract_free_text_annotations.py' 'output\pdf\annotated.pdf'
```

Preserve the annotated source. Map comments to page and rectangle before editing; decode UTF-16 FreeText values when exposed by the PDF library.

## PDF authoring and rendering

Immediately before the first PDF create/edit command in a turn, run the marker exactly once:

```powershell
node 'C:\Users\mitya\.codex\plugins\cache\openai-primary-runtime\pdf\26.819.11345\skills\pdf\container_tools\mark_artifact_operation_started.mjs' --operation-kind edit --expected-output-count 1 --output-format pdf
```

Render the project source with configurable paths:

```powershell
& '.agents\skills\assembly-manual-qa\scripts\render_manual.ps1' `
  -HtmlPath 'tmp\pdfs\assembly_manual.html' `
  -OutputPdf 'output\pdf\Assembly_Manual_ASSEMBLY-YYYY-MM-DD.pdf'
```

Render all pages with Poppler (`pdftoppm`) or the project's equivalent helper. Inspect a contact sheet and every changed page at full resolution. Confirm page count, footer/revision, no clipping/overlap, and no unintended `/FreeText` annotations in a clean PDF.

## Release checks

```powershell
Get-FileHash '<BOM-or-release-file>' -Algorithm SHA256
git status --short
bd show <id> --json
```

Do not commit, push, save Fusion, place orders, or sync Dolt unless the user explicitly authorizes it.
