param(
    [Parameter(Mandatory = $true)]
    [string]$BuildDir,
    [string]$Msys2Root = "C:\msys64"
)

$ErrorActionPreference = "Stop"
$toolBin = Join-Path $Msys2Root "ucrt64\bin"
$outputDirectory = Join-Path $BuildDir "bin"
$runtimeNames = @(
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
    "libunwind.dll",
    "libssp-0.dll"
)

if (-not (Test-Path $outputDirectory -PathType Container)) {
    throw "Runtime output directory was not found: $outputDirectory"
}

$copied = @()
foreach ($runtimeName in $runtimeNames) {
    $source = Join-Path $toolBin $runtimeName
    if (Test-Path $source -PathType Leaf) {
        $destination = Join-Path $outputDirectory $runtimeName
        Copy-Item -Path $source -Destination $destination -Force
        $copied += $runtimeName
    }
}

$required = @("libstdc++-6.dll", "libwinpthread-1.dll")
if (Test-Path (Join-Path $toolBin "libgcc_s_seh-1.dll") -PathType Leaf) {
    $required += "libgcc_s_seh-1.dll"
}
foreach ($runtimeName in $required) {
    if (-not (Test-Path (Join-Path $outputDirectory $runtimeName) -PathType Leaf)) {
        throw "Required MinGW runtime was not bundled: $runtimeName"
    }
}

if ($copied.Count -eq 0) {
    throw "No MinGW runtime DLLs were copied from $toolBin"
}

$objdump = Join-Path $toolBin "objdump.exe"
if (Test-Path $objdump -PathType Leaf) {
    foreach ($executable in Get-ChildItem -Path $outputDirectory -Filter "*.exe" -File) {
        $dependencyOutput = & $objdump -p $executable.FullName 2>$null
        foreach ($runtimeName in $runtimeNames) {
            if ($dependencyOutput -match [regex]::Escape("DLL Name: $runtimeName") -and
                -not (Test-Path (Join-Path $outputDirectory $runtimeName) -PathType Leaf)) {
                throw "$($executable.Name) requires $runtimeName, but it was not bundled."
            }
        }
    }
}

Write-Host "Bundled MinGW runtime beside executables: $($copied -join ', ')"
Write-Host "Runnable output directory: $outputDirectory"
