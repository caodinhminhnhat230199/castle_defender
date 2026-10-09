# Runs Tools/create_hero_combat_tests.py in a headless editor.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')

$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Script = Join-Path $PSScriptRoot 'create_hero_combat_tests.py'
& $Editor $ProjectFile -run=pythonscript "-script=$Script" -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
exit $LASTEXITCODE
