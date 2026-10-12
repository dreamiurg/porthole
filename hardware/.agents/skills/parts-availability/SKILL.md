---
name: parts-availability
description: Check current availability of required commercial parts and distinguish broadly available catalog items from specialist, fabrication-only, obsolete, or custom items with dated evidence.
---

# Commercial-parts availability

Use this skill when a design needs current purchase links, stock, lead time, regional sourcing, or a judgment about whether a part is genuinely easy to obtain. Availability is time- and region-sensitive, so do not infer it from memory or from a search-result title.

## Workflow

1. Normalize the requirement before searching: function, exact dimensions, material, strength/finish, thread/standard, quantity, region, acceptable substitutes, required-by date, and whether cut-to-length or modification is allowed.
2. Search authoritative product pages and technical drawings first. Use retailers and marketplaces as purchase channels, but verify the exact SKU/specification against the maker or a reliable drawing.
3. Record the checked date, region, seller, SKU, exact specification, listed price/currency, stock or lead-time statement, minimum order, pack quantity, shipping constraint, and evidence URL.
4. Classify each line:
   - **A - broadly available:** common hardware or standard stock sold by multiple normal retail/industrial channels;
   - **B - catalog/specialist:** a standard item available from a limited industrial distributor or specialist supplier;
   - **C - service/fabrication:** requires cutting, machining, bending, welding, plating, or a specialist service even if the raw material is common;
   - **D - constrained:** obsolete, backordered, region-locked, high-MOQ, or otherwise difficult to source;
   - **E - custom/unknown:** no verified off-the-shelf match, or evidence is insufficient.
5. Distinguish **listed**, **orderable**, **in stock**, and **deliverable by the required date**. A search result or product page without stock evidence is not proof of availability.
6. For substitutes, compare the functional specification and Fusion interface, not just nominal size. Hand off the design choice to `$custom-vs-standard` when a substitute requires modification or creates a new load path.
7. Report a concise recommendation: buy now, buy from a specialist, request a quote, redesign around a standard part, or keep the item as a custom/fabrication dependency. Do not place an order or claim a delivery date without user approval.

## Evidence and uncertainty

- Prefer manufacturer, distributor, and service-provider pages with drawings and explicit inventory or lead time.
- Keep one row per exact SKU and pack size; a 10-pack is not the same as ten individually orderable pieces.
- Mark marketplace listings with seller identity and condition. Do not treat multiple duplicate listings as independent availability.
- If live stock cannot be verified, say **availability unconfirmed** and give the next check or contact action.
- Prices, stock, shipping, and lead times expire; include the evidence date in every current-availability report.

Use [availability-schema.md](references/availability-schema.md) for the report table and confidence rules.
