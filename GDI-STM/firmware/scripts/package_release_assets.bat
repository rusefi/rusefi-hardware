@echo off
setlocal EnableExtensions

REM Usage:
REM   scripts\package_release_assets.bat v0.1.1
REM Packages build outputs into release\<tag>\ for GitLab release asset upload.

if "%~1"=="" (
    echo Usage: scripts\package_release_assets.bat ^<tag^>
    echo Example: scripts\package_release_assets.bat v0.1.1
    exit /b 1
)

REM Move to project root.
cd /d "%~dp0.."

set "TAG=%~1"
set "OUT_DIR=release\%TAG%"

if not exist "build\GDI_STM_Firmware.hex" (
    echo ERROR: Missing build\GDI_STM_Firmware.hex
    echo Build first using scripts\build_firmware.bat
    exit /b 1
)
if not exist "build\GDI_STM_Firmware.bin" (
    echo ERROR: Missing build\GDI_STM_Firmware.bin
    echo Build first using scripts\build_firmware.bat
    exit /b 1
)
if not exist "build\GDI_STM_Firmware.map" (
    echo ERROR: Missing build\GDI_STM_Firmware.map
    echo Build first using scripts\build_firmware.bat
    exit /b 1
)

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

copy /Y "build\GDI_STM_Firmware.hex" "%OUT_DIR%\GDI_STM_Firmware.hex" >nul
copy /Y "build\GDI_STM_Firmware.bin" "%OUT_DIR%\GDI_STM_Firmware.bin" >nul
copy /Y "build\GDI_STM_Firmware.map" "%OUT_DIR%\GDI_STM_Firmware.map" >nul

powershell -NoProfile -Command "$files=@('%OUT_DIR%\\GDI_STM_Firmware.hex','%OUT_DIR%\\GDI_STM_Firmware.bin','%OUT_DIR%\\GDI_STM_Firmware.map'); Get-FileHash -Algorithm SHA256 $files | ForEach-Object { '{0}  {1}' -f $_.Hash, [System.IO.Path]::GetFileName($_.Path) } | Set-Content -Encoding ascii '%OUT_DIR%\\SHA256SUMS.txt'"
if errorlevel 1 (
    echo ERROR: Failed to generate SHA256SUMS.txt
    exit /b 1
)

echo Release assets packaged in:
echo   %OUT_DIR%
echo Upload these files to the GitLab Release for tag %TAG%:
echo   GDI_STM_Firmware.hex
echo   GDI_STM_Firmware.bin
echo   GDI_STM_Firmware.map
echo   SHA256SUMS.txt

exit /b 0
