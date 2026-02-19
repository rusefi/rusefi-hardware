#!/bin/bash

# GDI STM Firmware Build Script for Windows
# Requires ARM GCC toolchain and CMake installed

# Set paths (adjust as needed)
TOOLCHAIN_PATH="/c/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.10/bin"
CMAKE_PATH="/c/Program Files/CMake/bin"

# Add to PATH
export PATH="$TOOLCHAIN_PATH:$CMAKE_PATH:$PATH"

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure with CMake
# Optional: pass extra CMake args via env var, e.g.
#   export GDI_CMAKE_EXTRA_ARGS="-DGDI_ENABLE_HS_VBAT=ON -DGDI_HS_VBAT_MASK=0x3"
cmake .. -DCMAKE_TOOLCHAIN_FILE=../arm-gcc-toolchain.cmake -G "MinGW Makefiles" ${GDI_CMAKE_EXTRA_ARGS}

# Build
make -j$(nproc)

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Generated files:"
    ls -la *.elf *.hex *.bin
else
    echo "Build failed!"
    exit 1
fi
