# Resolves the pinned engine (EngineAssociation in CastleDefender.uproject).
# Override with the UE_ROOT environment variable, e.g. $env:UE_ROOT = 'E:\Program Files\Epic Games\UE_5.8'.

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot 'CastleDefender.uproject'

function Get-UERoot {
    if ($env:UE_ROOT) { return $env:UE_ROOT }
    $Version = (Get-Content $ProjectFile -Raw | ConvertFrom-Json).EngineAssociation
    $Manifest = 'C:\ProgramData\Epic\UnrealEngineLauncher\LauncherInstalled.dat'
    if (Test-Path $Manifest) {
        $Install = (Get-Content $Manifest -Raw | ConvertFrom-Json).InstallationList |
            Where-Object { $_.AppName -eq "UE_$Version" } | Select-Object -First 1
        if ($Install) { return $Install.InstallLocation }
    }
    throw "UE $Version not found. Set UE_ROOT to the engine install folder."
}
