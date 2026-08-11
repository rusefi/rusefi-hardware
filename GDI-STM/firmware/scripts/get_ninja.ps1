param(
    [Parameter(Mandatory = $true)]
    [string] $Destination
)

$ErrorActionPreference = 'Stop'

New-Item -ItemType Directory -Force -Path $Destination | Out-Null

$zip = Join-Path $Destination 'ninja-win.zip'
$uri = 'https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip'

Invoke-WebRequest -Uri $uri -OutFile $zip
Expand-Archive -Path $zip -DestinationPath $Destination -Force

$ninjaExe = Join-Path $Destination 'ninja.exe'
if (-not (Test-Path $ninjaExe)) {
    throw "ninja.exe not found after extraction to '$Destination'."
}

