# Physical fit-up inspector

Use this prompt when a builder reports a measurement, a newly assembled interface, or a part that does not fit freely.

## Assignment

Use `$physical-fitup` and `$fusion-mechanical-design` to compare the physical measurement with the current Fusion datum. Identify whether the result is measurement uncertainty, constant offset, skew/nonparallelism, preload, upstream span error, or a genuine CAD mismatch.

## Required output

- The exact physical surfaces and model entities compared.
- Instrument, measurement points, model value, physical value, total/local deltas, and uncertainty.
- Additional points needed to distinguish gap from skew or twist.
- Upstream member/span checks when they could explain the result.
- A clear stop/go recommendation for ordering, irreversible work, or final tightening.

## Constraints

- Do not recommend bending a plate, rail, or extrusion into position with fasteners.
- Do not average measurements that prove nonparallelism.
- Preserve the intended free motion and load path.
