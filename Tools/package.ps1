# Packages a Win64 build into Saved/Packaged (not committed).
# Usage: powershell -File Tools/package.ps1 [-Configuration Development|Shipping]
param(
    [string]$Configuration = 'Development'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')

$UAT = Join-Path (Get-UERoot) 'Engine\Build\BatchFiles\RunUAT.bat'
$Archive = Join-Path $ProjectRoot 'Saved\Packaged'
& $UAT BuildCookRun "-project=$ProjectFile" -platform=Win64 "-clientconfig=$Configuration" -build -cook -stage -pak -archive "-archivedirectory=$Archive" -noP4 -utf8output -unattended
exit $LASTEXITCODE
