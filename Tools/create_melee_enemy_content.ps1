$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')
$taskEditor = Join-Path (Get-UERoot) 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$taskScript = Join-Path $PSScriptRoot 'create_melee_enemy_content.py'
& $taskEditor $ProjectFile -run=pythonscript "-script=$taskScript" -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
exit $LASTEXITCODE
