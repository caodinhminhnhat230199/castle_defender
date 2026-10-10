param([string]$ScriptPath)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')
$FullPath = (Resolve-Path $ScriptPath).Path
$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
& $Editor $ProjectFile -run=pythonscript "-script=$FullPath" -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
exit $LASTEXITCODE
