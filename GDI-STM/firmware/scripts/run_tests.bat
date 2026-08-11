@echo off
setlocal EnableExtensions

REM Static verification checks (host compiler not required).
REM For embedded runtime tests / coverage you will need hardware.

cd /d "%~dp0.."

echo Running produced PCB pinmap verification...
python scripts\\verify_pcb_vs_firmware.py
if errorlevel 1 (
    echo FAILED
    exit /b 1
)

echo OK
exit /b 0
