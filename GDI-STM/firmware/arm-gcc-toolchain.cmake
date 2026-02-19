# ARM GCC Toolchain for STM32
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Avoid try-compile linking an executable during configure (common for bare-metal).
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Locate the cross compiler toolchain (expects it on PATH or passed via CMAKE_*_COMPILER).
find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc REQUIRED)
find_program(ARM_NONE_EABI_GXX arm-none-eabi-g++ REQUIRED)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_NONE_EABI_OBJDUMP arm-none-eabi-objdump REQUIRED)
find_program(ARM_NONE_EABI_SIZE arm-none-eabi-size REQUIRED)

set(CMAKE_C_COMPILER ${ARM_NONE_EABI_GCC})
set(CMAKE_CXX_COMPILER ${ARM_NONE_EABI_GXX})
set(CMAKE_ASM_COMPILER ${ARM_NONE_EABI_GCC})
set(CMAKE_OBJCOPY ${ARM_NONE_EABI_OBJCOPY})
set(CMAKE_OBJDUMP ${ARM_NONE_EABI_OBJDUMP})
set(CMAKE_SIZE ${ARM_NONE_EABI_SIZE})

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set the compiler flags for ARM Cortex-M4
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT "${CPU_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
set(CMAKE_CXX_FLAGS_INIT "${CPU_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS} -x assembler-with-cpp")

# Set linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--gc-sections -Wl,--print-memory-usage")
