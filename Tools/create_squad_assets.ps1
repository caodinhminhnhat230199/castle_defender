# Creates only missing T-SQD-01 editor fixtures; existing game maps/content are preserved.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')
$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Script = Join-Path $PSScriptRoot 'create_squad_assets.py'
& $Editor $ProjectFile -run=pythonscript "-script=$Script" -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
exit $LASTEXITCODE
