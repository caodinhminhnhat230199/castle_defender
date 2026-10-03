# Builds a CastleDefender target from the command line.
# Usage: powershell -File Tools/build.ps1 [-Target CastleDefenderEditor|CastleDefender] [-Configuration Development]
param(
    [string]$Target = 'CastleDefenderEditor',
    [string]$Configuration = 'Development'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')

$Build = Join-Path (Get-UERoot) 'Engine\Build\BatchFiles\Build.bat'
& $Build $Target Win64 $Configuration "-Project=$ProjectFile" -WaitMutex
exit $LASTEXITCODE
