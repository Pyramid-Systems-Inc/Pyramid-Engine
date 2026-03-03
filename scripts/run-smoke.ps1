param(
    [string]$BuildDir = "build-clean",
    [string]$Config = "Debug",
    [int]$DurationSeconds = 5
)

$ErrorActionPreference = "Stop"

function Resolve-Executable {
    param(
        [string]$Name,
        [string[]]$Candidates
    )

    foreach ($candidate in $Candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw "Could not locate executable for '$Name'. Checked: $($Candidates -join ', ')"
}

function Invoke-SmokeRun {
    param(
        [string]$Name,
        [string]$ExecutablePath,
        [int]$TimeoutSeconds
    )

    Write-Host "[INFO] Launching ${Name}: $ExecutablePath"
    $proc = Start-Process -FilePath $ExecutablePath -PassThru -WorkingDirectory (Resolve-Path ".").Path

    Start-Sleep -Seconds $TimeoutSeconds

    if (-not $proc.HasExited) {
        Stop-Process -Id $proc.Id -Force
        Write-Host "[PASS] $Name remained running for $TimeoutSeconds seconds."
        return $true
    }

    if ($proc.ExitCode -eq 0) {
        Write-Host "[PASS] $Name exited with code 0 before timeout."
        return $true
    }

    Write-Host "[FAIL] $Name exited early with code $($proc.ExitCode)."
    return $false
}

$basicGame = Resolve-Executable -Name "BasicGame" -Candidates @(
    (Join-Path $BuildDir "bin/$Config/BasicGamed.exe"),
    (Join-Path $BuildDir "bin/$Config/BasicGame.exe")
)

$basicRendering = Resolve-Executable -Name "BasicRenderingExample" -Candidates @(
    (Join-Path $BuildDir "Examples/BasicRendering/$Config/BasicRenderingExample.exe"),
    (Join-Path $BuildDir "bin/$Config/BasicRenderingExample.exe")
)

$ok = $true
$ok = $ok -and (Invoke-SmokeRun -Name "BasicGame" -ExecutablePath $basicGame -TimeoutSeconds $DurationSeconds)
$ok = $ok -and (Invoke-SmokeRun -Name "BasicRenderingExample" -ExecutablePath $basicRendering -TimeoutSeconds $DurationSeconds)

$logPath = Join-Path (Resolve-Path ".").Path "pyramid_game.log"
if (Test-Path $logPath) {
    $logContent = Get-Content $logPath -Raw
    if ($logContent -match "initialized successfully|starting up") {
        Write-Host "[PASS] BasicGame log file contains startup markers."
    }
    else {
        Write-Host "[WARN] BasicGame log file exists but startup markers were not found."
    }
}
else {
    Write-Host "[WARN] BasicGame log file was not found at $logPath."
}

if (-not $ok) {
    Write-Host "[FAIL] Smoke test failed."
    exit 1
}

Write-Host "[PASS] Smoke test completed successfully."
exit 0
