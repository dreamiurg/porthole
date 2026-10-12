param(
  [Parameter(Mandatory = $true)][double]$MeasuredMm,
  [Parameter(Mandatory = $true)][double]$ModelMm,
  [double]$ToleranceMm = 0.50,
  [string]$Label = 'datum'
)

if ($ToleranceMm -lt 0) { throw 'ToleranceMm must be non-negative.' }
$delta = $MeasuredMm - $ModelMm
$within = [Math]::Abs($delta) -le $ToleranceMm
$recommendation = if ($within) {
  'Within the stated tolerance; continue with the interface checks and record the measurement basis.'
} else {
  'Outside the stated tolerance; remeasure identified datums and inspect upstream spans or skew before tightening, ordering, or redesigning.'
}

[pscustomobject]@{
  label = $Label
  measured_mm = $MeasuredMm
  model_mm = $ModelMm
  delta_mm = [Math]::Round($delta, 6)
  tolerance_mm = $ToleranceMm
  within_tolerance = $within
  recommendation = $recommendation
} | ConvertTo-Json -Compress
