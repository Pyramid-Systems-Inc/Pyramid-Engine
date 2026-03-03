param(
    [ValidateSet("vs2022-debug", "vs2022-debug-tests")]
    [string]$Preset = "vs2022-debug"
)

$ErrorActionPreference = "Stop"

Write-Host "Removing existing build directory..."
$buildDir = "build"
$cleanSucceeded = $true
if (Test-Path $buildDir) {
    try {
        Remove-Item -Path $buildDir -Recurse -Force
    }
    catch {
        Write-Warning "Could not fully remove '$buildDir' (likely locked by another process). Falling back to 'build-clean'."
        $cleanSucceeded = $false
        $buildDir = "build-clean"
    }
}

if ($cleanSucceeded) {
    Write-Host "Configuring with preset '$Preset'..."
    cmake --preset $Preset
} else {
    Write-Host "Configuring fallback directory '$buildDir'..."
    $buildTests = if ($Preset -eq "vs2022-debug-tests") { "ON" } else { "OFF" }
    cmake -S . -B $buildDir -G "Visual Studio 17 2022" -A x64 `
        -DPYRAMID_BUILD_EXAMPLES=ON `
        -DPYRAMID_BUILD_TESTS=$buildTests `
        -DPYRAMID_BUILD_TOOLS=OFF
}

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Configuration completed."
