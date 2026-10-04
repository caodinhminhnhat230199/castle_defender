# Pure report evaluation shared by the CLI runner and its regression checks.
function Test-AutomationReport {
    param($Result, [int]$EditorExitCode, [bool]$RequireDefaultGroups = $false)
    if ($EditorExitCode -ne 0) { return $false }
    foreach ($Counter in @('succeeded', 'succeededWithWarnings', 'failed', 'notRun', 'inProcess')) {
        if ($null -eq $Result.$Counter -or $Result.$Counter -isnot [ValueType] -or
            $Result.$Counter -lt 0 -or $Result.$Counter -ne [Math]::Floor($Result.$Counter)) { return $false }
    }
    $Tests = @($Result.tests)
    $Passed = $Result.succeeded + $Result.succeededWithWarnings
    if ($Result.failed -ne 0 -or $Result.notRun -ne 0 -or $Result.inProcess -ne 0 -or
        $Passed -eq 0 -or $Tests.Count -ne $Passed) { return $false }
    foreach ($Test in $Tests) {
        if ($Test.state -notin @('Success', 'SuccessWithWarnings') -or
            [string]::IsNullOrWhiteSpace($Test.fullTestPath)) { return $false }
    }
    if (@($Tests | Where-Object state -eq 'Success').Count -ne $Result.succeeded -or
        @($Tests | Where-Object state -eq 'SuccessWithWarnings').Count -ne $Result.succeededWithWarnings) { return $false }
    if (@($Tests.fullTestPath | Select-Object -Unique).Count -ne $Tests.Count) { return $false }
    if ($RequireDefaultGroups -and (
        @($Tests | Where-Object fullTestPath -Like 'CastleDefender.*').Count -eq 0 -or
        @($Tests | Where-Object fullTestPath -Like 'Project.Functional Tests.*').Count -eq 0)) { return $false }
    return $true
}
