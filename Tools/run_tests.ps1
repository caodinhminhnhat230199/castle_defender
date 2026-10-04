# Runs CastleDefender automation tests headless and fails on any failed test.
# Usage: Tools/run_tests.bat [-Filter "CastleDefender.Combat"]
# Exit codes: 0 complete pass, 1 failed/incomplete run or editor failure, 2 missing/invalid report.
param(
    [string]$Filter = 'CastleDefender.+Project.Functional Tests'
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'UERoot.ps1')
. (Join-Path $PSScriptRoot 'TestReport.ps1')

$Editor = Join-Path (Get-UERoot) 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Report = Join-Path $ProjectRoot 'Saved\Automation\CLI'
$Report = [IO.Path]::GetFullPath($Report)
$ExpectedReport = [IO.Path]::GetFullPath((Join-Path $ProjectRoot 'Saved\Automation\CLI'))
if ($Report -ne $ExpectedReport -or -not $Report.StartsWith([IO.Path]::GetFullPath($ProjectRoot).TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Refusing to remove a report directory outside the project'
}
if (Test-Path -LiteralPath $Report) { Remove-Item -LiteralPath $Report -Recurse -Force }

# Check both the editor process and the exported results; neither is sufficient alone.
& $Editor $ProjectFile "-ExecCmds=Automation RunTests $Filter;Quit" "-ReportExportPath=$Report" -unattended -nopause -nosplash -NullRHI -stdout -FullStdOutLogOutput | Out-Host
$EditorExitCode = $LASTEXITCODE

$Index = Join-Path $Report 'index.json'
if (-not (Test-Path $Index)) { Write-Host "No test report at $Index"; exit 2 }

try { $Result = Get-Content -LiteralPath $Index -Raw | ConvertFrom-Json }
catch { Write-Host "Invalid test report: $_"; exit 2 }
foreach ($Test in $Result.tests) { Write-Host ("{0,-10} {1}" -f $Test.state, $Test.fullTestPath) }
$Passed = $Result.succeeded + $Result.succeededWithWarnings
Write-Host "Passed: $Passed  Failed: $($Result.failed)  Not run: $($Result.notRun)"
Write-Host "Editor exit code: $EditorExitCode"
if (-not (Test-AutomationReport $Result $EditorExitCode ($Filter -eq 'CastleDefender.+Project.Functional Tests'))) { exit 1 }
exit 0
