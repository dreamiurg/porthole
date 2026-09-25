param(
  [string]$HtmlPath = 'tmp\pdfs\assembly_manual.html',
  [string]$OutputPdf = 'output\pdf\Assembly_Manual_RENDERED.pdf',
  [string]$ProfilePath = 'tmp\pdfs\edge_profile_manual'
)

$edgeCandidates = @(
  'C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe',
  'C:\Program Files\Microsoft\Edge\Application\msedge.exe'
)
$edgePath = $edgeCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $edgePath) { throw 'Microsoft Edge was not found in the standard installation paths.' }

$htmlFull = [System.IO.Path]::GetFullPath($HtmlPath)
$pdfFull = [System.IO.Path]::GetFullPath($OutputPdf)
$profileFull = [System.IO.Path]::GetFullPath($ProfilePath)
if (-not (Test-Path -LiteralPath $htmlFull)) { throw "HTML source not found: $htmlFull" }
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $pdfFull) | Out-Null
New-Item -ItemType Directory -Force -Path $profileFull | Out-Null

$fileUrl = [System.Uri]::new($htmlFull).AbsoluteUri
$edgeArgs = @(
  '--headless',
  '--disable-gpu',
  '--no-pdf-header-footer',
  '--run-all-compositor-stages-before-draw',
  "--user-data-dir=$profileFull",
  "--print-to-pdf=$pdfFull",
  $fileUrl
)
& $edgePath @edgeArgs
$edgeExit = $LASTEXITCODE
if (-not (Test-Path -LiteralPath $pdfFull)) { throw "Edge returned exit code $edgeExit and did not create the PDF: $pdfFull" }
if ($edgeExit -ne 0) { Write-Warning "Edge returned exit code $edgeExit, but the requested PDF exists; continuing with the artifact for inspection." }
Get-Item -LiteralPath $pdfFull | Select-Object FullName, Length, LastWriteTime
