@echo off
setlocal EnableExtensions
echo Building GDI STM Firmware...

REM Change to project root directory
cd /d "%~dp0.."

REM Resolve CMake path (prefer PATH, fallback to default install location)
set "CMAKE_EXE=cmake"
where %CMAKE_EXE% >nul 2>&1
if errorlevel 1 (
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
    ) else (
        echo ERROR: CMake not found. Install CMake and/or add it to PATH.
        exit /b 1
    )
)

REM Ensure ARM GCC toolchain is on PATH if installed in the common location
if exist "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin\arm-none-eabi-gcc.exe" set "PATH=C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin;%PATH%"

REM Initialize dependency submodules (STM32CubeG4 + FreeRTOS) if needed
if exist "lib\STM32CubeG4\.git" (
    if not exist "lib\STM32CubeG4\Drivers\STM32G4xx_HAL_Driver\Inc" (
        echo Initializing STM32CubeG4 submodules...
        git -C lib\STM32CubeG4 submodule update --init --recursive
        if errorlevel 1 (
            echo ERROR: Failed to init STM32CubeG4 submodules.
            exit /b 1
        )
    )
)

if exist "lib\FreeRTOS\.git" (
    if not exist "lib\FreeRTOS\FreeRTOS\Source\include\FreeRTOS.h" (
        echo Initializing FreeRTOS kernel submodule...
        git -C lib\FreeRTOS submodule update --init --recursive
        if errorlevel 1 (
            echo ERROR: Failed to init FreeRTOS submodules.
            exit /b 1
        )
    )
)

REM Ensure Ninja is available (download locally if missing)
set "TOOLS_DIR=%CD%\scripts\tools"
set "NINJA_EXE=%TOOLS_DIR%\ninja\ninja.exe"
if not exist "%NINJA_EXE%" (
    echo Downloading Ninja build tool...
    if not exist "%TOOLS_DIR%\ninja" mkdir "%TOOLS_DIR%\ninja"
    powershell -NoProfile -ExecutionPolicy Bypass -File "%CD%\scripts\get_ninja.ps1" -Destination "%TOOLS_DIR%\ninja"
    if errorlevel 1 (
        echo ERROR: Failed to download Ninja. Check your network or download it manually.
        exit /b 1
    )
)

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Change to build directory
cd build

REM If the build directory was generated with a different generator, clean it.
if exist CMakeCache.txt (
    findstr /c:"CMAKE_GENERATOR:INTERNAL=Ninja" CMakeCache.txt >nul 2>&1
    if errorlevel 1 (
        echo Cleaning build directory - generator changed...
        del /f /q CMakeCache.txt >nul 2>&1
        if exist CMakeFiles rmdir /s /q CMakeFiles
        if exist Makefile del /f /q Makefile >nul 2>&1
    )
)

REM Configure with CMake
REM Optional: pass extra CMake args via env var, e.g.
REM   set GDI_CMAKE_EXTRA_ARGS=-DGDI_ENABLE_HS_VBAT=ON -DGDI_HS_VBAT_MASK=0x3
REM   scripts\build_firmware.bat
"%CMAKE_EXE%" .. -G Ninja -DCMAKE_MAKE_PROGRAM="%NINJA_EXE%" -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DGDI_BUILD_TESTS=OFF %GDI_CMAKE_EXTRA_ARGS%

REM Build the project
"%CMAKE_EXE%" --build .

REM Check if build was successful
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo Firmware files created in build/ directory
echo.
echo Verifying produced PCB pinmap against firmware definitions...
python ..\\scripts\\verify_pcb_vs_firmware.py --pcb ..\\GDI-STM.kicad_pcb --board-pins ..\\include\\board_pins.hpp
if errorlevel 1 (
    echo ERROR: Pinmap verification failed. Check PCB vs firmware pin definitions.
    exit /b 1
)
