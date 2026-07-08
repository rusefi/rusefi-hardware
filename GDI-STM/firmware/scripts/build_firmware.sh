#!/usr/bin/env bash
# Build GDI-STM firmware (Linux/CI).
# Usage: scripts/build_firmware.sh [extra cmake args...]

set -e

cd "$(dirname "$0")/.."

echo "Building GDI STM Firmware..."

# Initialize submodules if needed
if [[ -d lib/STM32CubeG4/.git ]] && [[ ! -d lib/STM32CubeG4/Drivers/STM32G4xx_HAL_Driver/Inc ]]; then
    echo "Initializing STM32CubeG4 submodules..."
    git -C lib/STM32CubeG4 submodule update --init --recursive
fi

if [[ -d lib/FreeRTOS/.git ]] && [[ ! -f lib/FreeRTOS/FreeRTOS/Source/include/FreeRTOS.h ]]; then
    echo "Initializing FreeRTOS kernel submodule..."
    git -C lib/FreeRTOS submodule update --init --recursive
fi

mkdir -p build
cd build

cmake .. \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DGDI_BUILD_TESTS=OFF \
    "$@"

cmake --build .

echo "Build completed successfully!"
echo "Verifying produced PCB pinmap against firmware definitions..."
python3 ../scripts/verify_pcb_vs_firmware.py --pcb ../GDI-STM.kicad_pcb --board-pins ../include/board_pins.hpp

echo "Firmware files created in build/ directory"
