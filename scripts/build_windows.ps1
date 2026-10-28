param(
    [switch]$NoPause
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root "build"
$dist = Join-Path $root "dist"

function Invoke-Checked {
    param(
        [Parameter(Mandatory = $true)][string]$File,
        [Parameter(Mandatory = $true)][string[]]$Arguments
    )
    Write-Host ""
    Write-Host "> $File $($Arguments -join ' ')" -ForegroundColor Cyan
    & $File @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE for: $File"
    }
}

try {
    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        throw "CMake no está instalado o no está en PATH."
    }

    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
    $vswherePath = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswherePath)) {
        throw "No se encontró vswhere.exe. Instala Visual Studio con la carga de trabajo 'Desktop development with C++'."
    }

    $installationVersion = (& $vswherePath -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion | Select-Object -First 1)
    if ([string]::IsNullOrWhiteSpace($installationVersion)) {
        throw "No se encontró una instalación de Visual Studio con las herramientas C++ x64."
    }

    $majorVersion = [int](($installationVersion -split '[.]')[0])
    if ($majorVersion -ge 18) {
        $generator = "Visual Studio 18 2026"
    }
    elseif ($majorVersion -ge 17) {
        $generator = "Visual Studio 17 2022"
    }
    elseif ($majorVersion -ge 16) {
        $generator = "Visual Studio 16 2019"
    }
    else {
        throw "Versión de Visual Studio no compatible: $installationVersion"
    }

    $iscc = $null
    $command = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($command) { $iscc = $command.Source }
    if (-not $iscc) {
        $isccCandidates = @(
            (Join-Path $programFilesX86 'Inno Setup 6\ISCC.exe'),
            (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe')
        )
        $iscc = $isccCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    }
    if (-not $iscc) {
        throw "No se encontró ISCC.exe. Instala Inno Setup 6 para generar el instalador."
    }

    Write-Host "Visual Studio: $installationVersion ($generator)" -ForegroundColor Green
    New-Item -ItemType Directory -Force -Path $dist | Out-Null

    Invoke-Checked "cmake" @("-S", $root, "-B", $build, "-G", $generator, "-A", "x64")
    Invoke-Checked "cmake" @("--build", $build, "--config", "Release", "--target", "YMYVerb_VST3", "test_dsp", "--parallel", "2")
    Invoke-Checked "ctest" @("--test-dir", $build, "-C", "Release", "--output-on-failure")

    $bundleCandidates = @(
        (Join-Path $build 'YMYVerb_artefacts\Release\VST3\YMYVerb.vst3'),
        (Join-Path $build 'YMYVerb_artefacts\RelWithDebInfo\VST3\YMYVerb.vst3')
    )
    $vst3Bundle = $bundleCandidates | Where-Object { Test-Path $_ -PathType Container } | Select-Object -First 1
    if (-not $vst3Bundle) {
        $vst3Bundle = Get-ChildItem -Path $build -Recurse -Directory -Filter 'YMYVerb.vst3' -ErrorAction SilentlyContinue |
            Select-Object -ExpandProperty FullName -First 1
    }
    if (-not $vst3Bundle) {
        throw "La compilación terminó, pero no se encontró el bundle YMYVerb.vst3."
    }

    $vst3Binary = Get-ChildItem -Path $vst3Bundle -Recurse -File -Filter '*.vst3' -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if (-not $vst3Binary) {
        throw "El bundle existe, pero no contiene un binario .vst3."
    }

    $stagedBundle = Join-Path $dist 'YMYVerb.vst3'
    if (Test-Path $stagedBundle) { Remove-Item -Recurse -Force $stagedBundle }
    Copy-Item -Path $vst3Bundle -Destination $stagedBundle -Recurse -Force

    Invoke-Checked $iscc @("/DVST3Dir=$vst3Bundle", (Join-Path $root 'Installer\YMYVerb.iss'))

    $installer = Join-Path $dist 'YMYVerb_Setup.exe'
    if (-not (Test-Path $installer -PathType Leaf)) {
        throw "Inno Setup no generó el archivo esperado: $installer"
    }
    if ((Get-Item $installer).Length -lt 100000) {
        throw "El instalador es demasiado pequeño; no parece una compilación válida."
    }

    Write-Host ""
    Write-Host "COMPILACIÓN COMPLETADA" -ForegroundColor Green
    Get-Item $vst3Binary, (Join-Path $dist 'YMYVerb.vst3'), $installer |
        Select-Object FullName, Length |
        Format-Table -AutoSize
}
catch {
    Write-Error $_
    exit 1
}

if (-not $NoPause -and $Host.Name -eq 'ConsoleHost') {
    Read-Host "Presiona Enter para cerrar"
}
