param(
    [ValidateSet("BasicGame", "BasicRendering")]
    [string]$Example = "BasicGame",
    [ValidateSet("gcc", "clang")]
    [string]$Compiler = "gcc",
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",
    [switch]$SkipBuild,
    [string]$Msys2Root = "C:\msys64"
)

$ErrorActionPreference = "Stop"
$repositoryRoot = Split-Path $PSScriptRoot -Parent

if (-not $SkipBuild) {
    & (Join-Path $PSScriptRoot "build-mingw.ps1") `
        -Compiler $Compiler `
        -Configuration $Configuration `
        -Msys2Root $Msys2Root
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$configurationName = $Configuration.ToLowerInvariant()
$buildName = "$Compiler-$configurationName-tests"
$buildDirectory = Join-Path $repositoryRoot ("build\" + $buildName)
& (Join-Path $PSScriptRoot "bundle-mingw-runtime.ps1") `
    -BuildDir $buildDirectory `
    -Msys2Root $Msys2Root
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$executableName = if ($Example -eq "BasicGame") {
    "BasicGame.exe"
} else {
    "BasicRenderingExample.exe"
}
$workingDirectory = Join-Path $buildDirectory "bin"
$executable = Join-Path $workingDirectory $executableName
if (-not (Test-Path $executable -PathType Leaf)) {
    throw "Example executable was not found: $executable"
}

Write-Host "Launching $executableName from $workingDirectory"
Start-Process -FilePath $executable -WorkingDirectory $workingDirectory
