# Runs Tools/create_enemy_assets.py in a headless editor, then builds the navmesh of a newly created test map.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')

$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Script = Join-Path $PSScriptRoot 'create_enemy_assets.py'
$Map = Join-Path $ProjectRoot 'Content\CastleDefender\Maps\Test\FT_Enemy_AggroChase.umap'
$NewMap = -not (Test-Path -LiteralPath $Map)
& $Editor $ProjectFile -run=pythonscript "-script=$Script" -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
# Only for a map created by this run (rebuilding would resave it every run). Commandlet worlds lock navigation
# building until async loading settles, which never happens here, so that wait is disabled for this command only.
if ($NewMap) {
    & $Editor $ProjectFile -run=ResavePackages -BuildNavigationData -Map=FT_Enemy_AggroChase `
        "-ini:Engine:[/Script/NavigationSystem.NavigationSystemV1]:bWaitForAsyncLoadingBeforeBuildingNavigationAutomatically=False" `
        -unattended -nosplash -nullrhi -stdout -FullStdOutLogOutput
}
exit $LASTEXITCODE
