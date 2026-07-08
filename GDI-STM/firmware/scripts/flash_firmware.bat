@echo off
setlocal EnableExtensions
echo ========================================
echo GDI STM Firmware Flashing Script
echo ========================================

REM Resolve project root relative to this script (robust to being run from any CWD).
set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "PROJ_ROOT=%%~fI"
for %%I in ("%PROJ_ROOT%\build") do set "DEFAULT_BUILD_DIR=%%~fI"

REM Check if STM32CubeProgrammer is installed
set "PROG_CLI=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
if not exist "%PROG_CLI%" (
    set "PROG_CLI=C:\Program Files (x86)\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
)
if not exist "%PROG_CLI%" (
    where STM32_Programmer_CLI.exe >nul 2>nul
    if errorlevel 1 (
        echo ERROR: STM32CubeProgrammer CLI not found.
        echo Install STM32CubeProgrammer or add STM32_Programmer_CLI.exe to PATH.
        echo Download from: https://www.st.com/en/development-tools/stm32cubeprog.html
        pause
        exit /b 1
    )
    set "PROG_CLI=STM32_Programmer_CLI.exe"
)

echo Using programmer:
echo   "%PROG_CLI%"

REM Optional connection tuning (helps with stubborn boards / long wires):
REM   set GDI_STLINK_SN=38FF...        (optional; forces a specific ST-LINK probe)
REM   set GDI_SWD_FREQ=1000           (kHz)
REM   set GDI_SWD_MODE=UR             (UR, Normal, HotPlug)
REM   set GDI_SWD_RESET=HWrst         (SWrst, HWrst)
REM If none are set, this script will try a few known-good profiles automatically.
set "CONNECT_BASE=port=SWD"
if not "%GDI_STLINK_SN%"=="" (
    set "CONNECT_BASE=%CONNECT_BASE% sn=%GDI_STLINK_SN%"
)

REM Check if firmware file exists
if not "%GDI_FIRMWARE_FILE%"=="" (
    set "HEX_FILE=%GDI_FIRMWARE_FILE%"
) else (
    set "HEX_FILE=%DEFAULT_BUILD_DIR%\GDI_STM_Firmware.hex"
    if not exist "%HEX_FILE%" (
        REM Fallback if user runs the script from inside the build directory.
        if exist "%CD%\GDI_STM_Firmware.hex" (
            set "HEX_FILE=%CD%\GDI_STM_Firmware.hex"
        )
    )
)
if not exist "%HEX_FILE%" (
    echo ERROR: Firmware file not found.
    echo Looked for:
    echo   "%DEFAULT_BUILD_DIR%\GDI_STM_Firmware.hex"
    echo   "%CD%\GDI_STM_Firmware.hex"
    echo.
    echo Please build the firmware first using scripts\build_firmware.bat
    echo Or set GDI_FIRMWARE_FILE to the full path of the .hex
    pause
    exit /b 1
)

echo Connecting to STM32G474MET3 and flashing firmware...
echo Firmware:
echo   "%HEX_FILE%"
echo.

set "USER_TUNED=0"
if not "%GDI_SWD_FREQ%"=="" set "USER_TUNED=1"
if not "%GDI_SWD_MODE%"=="" set "USER_TUNED=1"
if not "%GDI_SWD_RESET%"=="" set "USER_TUNED=1"

if "%USER_TUNED%"=="1" (
    if "%GDI_SWD_FREQ%"=="" (set "GDI_SWD_FREQ=4000")
    if "%GDI_SWD_MODE%"=="" (set "GDI_SWD_MODE=Normal")
    if "%GDI_SWD_RESET%"=="" (set "GDI_SWD_RESET=SWrst")

    REM Normalize common values (case-insensitive)
    if /I "%GDI_SWD_MODE%"=="NORMAL" set "GDI_SWD_MODE=Normal"
    if /I "%GDI_SWD_MODE%"=="HOTPLUG" set "GDI_SWD_MODE=HotPlug"

    set "CONNECT_OPTS=%CONNECT_BASE% freq=%GDI_SWD_FREQ% mode=%GDI_SWD_MODE% reset=%GDI_SWD_RESET%"
    echo SWD options:
    echo   %CONNECT_OPTS%
    echo.

    "%PROG_CLI%" -c %CONNECT_OPTS% -d "%HEX_FILE%" -v
) else (
    echo SWD options: auto
    echo   %CONNECT_BASE%
    echo.

    set "FLASH_OK=0"
    call :TryFlash "freq=4000 mode=Normal reset=SWrst"
    if not errorlevel 1 set "FLASH_OK=1"

    if "%FLASH_OK%"=="0" (
        call :TryFlash "freq=1000 mode=Normal reset=SWrst"
        if not errorlevel 1 set "FLASH_OK=1"
    )

    if "%FLASH_OK%"=="0" (
        call :TryFlash "freq=1000 mode=UR reset=HWrst"
        if not errorlevel 1 set "FLASH_OK=1"
    )

    if "%FLASH_OK%"=="0" (
        call :TryFlash "freq=4000 mode=UR reset=SWrst"
        if not errorlevel 1 set "FLASH_OK=1"
    )
)

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: Failed to flash firmware
    echo ========================================
    echo.
    echo Please check:
    echo 1. STM32G474MET3 is connected via SWD
    echo 2. Power is supplied to the board
    echo 3. No other programs are using the debugger - close CubeProgrammer GUI
    echo 4. If needed: set GDI_SWD_MODE=UR ^& GDI_SWD_RESET=HWrst ^& GDI_SWD_FREQ=1000
    echo 5. If multiple probes: set GDI_STLINK_SN=... (see GUI log)
) else (
    echo.
    echo ========================================
    echo SUCCESS: Firmware flashed successfully!
    echo ========================================
    echo.
    echo The GDI STM firmware is now ready on your STM32G474MET3
    echo You can now test the integration with rusEFI super-uaefi
)

echo.
pause
exit /b %errorlevel%

:TryFlash
set "CONNECT_OPTS=%CONNECT_BASE% %~1"
echo Trying:
echo   %CONNECT_OPTS%
echo.
"%PROG_CLI%" -c %CONNECT_OPTS% -d "%HEX_FILE%" -v
if errorlevel 1 (
    echo.
    echo flash attempt failed
    echo.
    exit /b 1
)
exit /b 0
