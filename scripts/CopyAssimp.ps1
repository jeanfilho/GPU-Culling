# Post-build script to copy assimp DLLs based on build configuration
param(
    [string]$Configuration,
    [string]$OutDir,
    [string]$SolutionDir
)

$dllName = if ($Configuration -eq "Debug") {
    "assimp-vc143-mtd.dll"
} else {
    "assimp-vc143-mt.dll"
}

$srcPath = Join-Path $SolutionDir "..\submodules\assimp\bin\$Configuration\$dllName"
$dstPath = Join-Path $OutDir $dllName

# Create output directory if it doesn't exist
if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
}

# Copy the DLL
if (Test-Path $srcPath) {
    Copy-Item -Path $srcPath -Destination $dstPath -Force
    Write-Host "Copied $dllName to output directory"
} else {
    Write-Warning "DLL not found at: $srcPath"
    exit 1
}
