# Synthetic regressions for false-green automation reports; no external test dependency.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'TestReport.ps1')
$Checks = 0
function Check([string]$Name, [bool]$Expected, $Report, [int]$ExitCode = 0, [bool]$DefaultGroups = $true) {
    $Actual = Test-AutomationReport $Report $ExitCode $DefaultGroups
    if ($Actual -ne $Expected) { throw "FAIL: $Name (expected $Expected, got $Actual)" }
    $script:Checks++
    Write-Host "PASS: $Name"
}
function Report {
    return ('{"succeeded":2,"succeededWithWarnings":0,"failed":0,"notRun":0,"inProcess":0,"tests":[{"state":"Success","fullTestPath":"CastleDefender.Combat.Health"},{"state":"Success","fullTestPath":"Project.Functional Tests.Smoke"}]}' | ConvertFrom-Json)
}
Check 'Complete default suite' $true (Report)
Check 'Editor failure after successful report' $false (Report) 7
$r = Report; $r.notRun = 1; Check 'NotRun aggregate' $false $r
$r = Report; $r.inProcess = 1; Check 'InProcess aggregate' $false $r
$r = Report; $r.failed = 1; Check 'Failed aggregate' $false $r
foreach ($State in @('NotRun','InProcess','Fail','Unknown')) {
    $r = Report; $r.tests[1].state = $State; Check "Nonterminal/failed state $State" $false $r
}
$r = Report; $r.tests = @($r.tests[0]); $r.succeeded = 1
Check 'Missing functional group in default suite' $false $r
Check 'Explicit filtered suite' $true $r 0 $false
$r = Report; $r.tests[0].fullTestPath = 'Other.Test'; Check 'Missing project group' $false $r
$r = Report; $r.succeeded = 3; Check 'Truncated report' $false $r
$r = Report; $r.tests[1].fullTestPath = $r.tests[0].fullTestPath; Check 'Duplicate results' $false $r 0 $false
$r = Report; $r.PSObject.Properties.Remove('notRun'); Check 'Missing counter' $false $r
$r = Report; $r.succeeded = '2'; Check 'Invalid counter type' $false $r
$r = Report; $r.succeeded = 0; $r.tests = @(); Check 'Empty run' $false $r
$r = Report; $r.succeeded = 1; $r.succeededWithWarnings = 1; $r.tests[1].state = 'SuccessWithWarnings'
Check 'Completed report with explicit warnings' $true $r
Write-Host "$Checks report checks passed"
