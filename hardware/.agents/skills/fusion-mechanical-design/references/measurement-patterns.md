# Reusable Fusion measurement patterns

## Record the basis before the number

Every reported dimension should carry:

| Field | Example form |
| --- | --- |
| Source | active Fusion document, vendor drawing, or as-built measurement |
| Version | document revision, vendor revision, or measurement date |
| Entities | component/body/face or physical surfaces |
| Coordinate | axis and sign convention |
| Calculation | expression and units |
| Uncertainty | instrument, fit, or modelling tolerance |

If two values have different bases, keep them as separate rows. Never average them into a new “design” value.

## Functional rather than envelope dimensions

Use the face, axis, or center that carries the interface:

- guide or rail spacing: opposing running/mounting datums;
- plate thickness: the two faces participating in the stack;
- bolt pattern: hole centers and axis direction;
- bearing or shaft location: bore axis and locating shoulder;
- clearance: the nearest surfaces along the actual motion or load direction.

Bounding boxes are useful for overall size, but can include fillets, chamfers, curvature, or construction geometry.

## Decision language

- **Verified:** the source and functional datums agree within the stated tolerance.
- **Conditionally verified:** the geometry is sound, but a vendor dimension, as-built measurement, or kit inclusion remains unconfirmed.
- **Conflict:** sources disagree; identify the smallest measurement or model decision that resolves the conflict.
- **Not checked:** do not imply an interface was validated merely because the model opens.
