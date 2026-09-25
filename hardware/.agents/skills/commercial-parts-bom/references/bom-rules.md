# BOM line-item schema and review gates

## Minimum line-item fields

| Field | Meaning |
| --- | --- |
| ID / description | Human-readable part and role |
| Interface | Mating parts, datum face, hole/axis, and load direction |
| Specification | Material, size, thread, pitch, finish, and tolerance |
| Quantity | Assembly count and purchase/spare count separately |
| Status | ORDER, KIT-CHECK, CONDITIONAL, FABRICATE, MODIFY-STANDARD, or VERIFY |
| Source | Vendor, drawing, kit, or fabrication route |
| Evidence date | When the source was checked |
| Risk / substitute | What can change and the acceptance test |

## Review gates

1. **Geometry:** Fusion and the vendor/as-built interface agree.
2. **Structure:** the part carries the intended load without relying on a weak edge, thin sheet, or accidental friction.
3. **Assembly:** tools can reach the fastener and the sequence is physically executable.
4. **Procurement:** exact specification, quantity, source, lead time, and minimum order are known or explicitly marked unknown.
5. **Verification:** substitutions and kit contents have a measurable incoming check.
