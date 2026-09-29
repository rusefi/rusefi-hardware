@echo off
setlocal

where pixi >nul 2>nul
if not errorlevel 1 goto build

rem Also find an existing installation before the terminal's PATH is refreshed.
if not defined PIXI_HOME set "PIXI_HOME=%USERPROFILE%\.pixi"
set "PATH=%PIXI_HOME%\bin;%PATH%"
where pixi >nul 2>nul
if not errorlevel 1 goto build

echo Pixi not found. Installing Pixi...
powershell -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference = 'Stop'; Invoke-RestMethod https://pixi.sh/install.ps1 | Invoke-Expression"
if errorlevel 1 (
    echo Failed to install Pixi.
    exit /b 1
)

where pixi >nul 2>nul
if errorlevel 1 (
    echo Pixi is still unavailable after installation.
    exit /b 1
)

:build
pixi run make -C . -j12
if errorlevel 1 exit /b %errorlevel%
call flash_cube.bat
