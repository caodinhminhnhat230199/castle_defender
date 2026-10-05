# Runs Tools/import_hero_model.py in a headless editor.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')

$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Script = Join-Path $PSScriptRoot 'import_hero_model.py'
& $Editor $ProjectFile -run=pythonscript "-script=$Script" -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
exit $LASTEXITCODE
