# Runs CastleDefender automation tests headless and fails on any failed test.
# Usage: powershell -File Tools/run_tests.ps1 [-Filter "CastleDefender.+Project.Functional Tests"]
# Exit codes: 0 all passed, 1 a test failed or none ran, 2 no report written.
param(
    [string]$Filter = 'CastleDefender.+Project.Functional Tests'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')

$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Report = Join-Path $ProjectRoot 'Saved\Automation\CLI'
Remove-Item -Recurse -Force $Report -ErrorAction SilentlyContinue

# The editor's exit code does not reflect test results, so read the report instead.
& $Editor $ProjectFile "-ExecCmds=Automation RunTests $Filter;Quit" "-ReportExportPath=$Report" -unattended -nopause -nosplash -NullRHI -stdout -FullStdOutLogOutput | Out-Host

$Index = Join-Path $Report 'index.json'
if (-not (Test-Path $Index)) { Write-Host "No test report at $Index"; exit 2 }

$Result = Get-Content $Index -Raw | ConvertFrom-Json
foreach ($Test in $Result.tests) { Write-Host ("{0,-10} {1}" -f $Test.state, $Test.fullTestPath) }
$Passed = $Result.succeeded + $Result.succeededWithWarnings
Write-Host "Passed: $Passed  Failed: $($Result.failed)  Not run: $($Result.notRun)"
if ($Result.failed -gt 0 -or $Passed -eq 0) { exit 1 }
exit 0
