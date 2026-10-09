# Runs Tools/create_enemy_combat_tests.py in a headless editor, then builds navigation data for L_Test_EnemyCombat.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')

$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Script = Join-Path $PSScriptRoot 'create_enemy_combat_tests.py'
& $Editor $ProjectFile -run=pythonscript "-script=$Script" -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Build navigation data for L_Test_EnemyCombat
& $Editor $ProjectFile -run=ResavePackages -BuildNavigationData -Map=L_Test_EnemyCombat -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
exit $LASTEXITCODE
