cmake_minimum_required(VERSION 3.16)

project(GDI_STM_Firmware LANGUAGES C CXX ASM)

option(GDI_BUILD_FIRMWARE "Build firmware (ARM)" ON)
option(GDI_BUILD_TESTS "Build host tests" OFF)

option(GDI_ENABLE_HS_VBAT "Enable HS_VBATx gate-driver HIN signals" OFF)
set(GDI_HS_VBAT_MASK "0x0" CACHE STRING "Bitmask for HS_VBAT enables (bit0=HS_VBAT1 .. bit3=HS_VBAT4)")
option(GDI_ENABLE_HS_VBOOST "Enable HS_VBOOSTx gate-driver HIN signals" OFF)
set(GDI_HS_VBOOST_MASK "0x0" CACHE STRING "Bitmask for HS_VBOOST enables (bit0=HS_VBOOST1 .. bit3=HS_VBOOST4)")
set(GDI_HS_ENABLE_DELAY_MS "500" CACHE STRING "Delay before enabling high-side outputs (ms)")

option(GDI_PROFILE_VBAT_ONLY_4CYL_HPFP_BANK4 "Profile: enable HS_VBOOST banks 1,2,4 (VBAT-only via Vdrive tie; HPFP on bank 4)" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CROSSCOMPILING)
    set(GDI_BUILD_TESTS OFF CACHE BOOL "Build host tests" FORCE)
endif()

set(PROJECT_INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32G474METX_FLASH.ld)

set(STM32CUBE_DIR ${CMAKE_SOURCE_DIR}/lib/STM32CubeG4)
set(FREERTOS_REPO_DIR ${CMAKE_SOURCE_DIR}/lib/FreeRTOS)

set(STM32_HAL_DIR ${STM32CUBE_DIR}/Drivers/STM32G4xx_HAL_Driver)
set(STM32_CMSIS_CORE_DIR ${STM32CUBE_DIR}/Drivers/CMSIS)
set(STM32_CMSIS_DEVICE_DIR ${STM32CUBE_DIR}/Drivers/CMSIS/Device/ST/STM32G4xx)

set(FREERTOS_KERNEL_DIR ${FREERTOS_REPO_DIR}/FreeRTOS/Source)

	if(GDI_BUILD_FIRMWARE)
	    if(GDI_PROFILE_VBAT_ONLY_4CYL_HPFP_BANK4)
	        # Conservative bring-up profile:
	        # - VBAT-only bring-up expects Vdrive tied to battery (see WIRING_GUIDE.md).
	        # - Enable HS_VBOOST selects for banks 1+2 (4 injectors) and bank 4 (HPFP). On the produced PCB,
	        #   HS_VBOOSTx controls the MOSFET that connects Vdrive to Hx (H1/H3/H5/H7).
	        # - Keep HS_VBAT selects disabled to avoid backfeeding into the VBAT diode path.
	        # - Add a longer delay before enabling outputs.
	        set(GDI_ENABLE_HS_VBAT OFF CACHE BOOL "" FORCE)
	        set(GDI_HS_VBAT_MASK "0x0" CACHE STRING "" FORCE)
	        set(GDI_ENABLE_HS_VBOOST ON CACHE BOOL "" FORCE)
	        set(GDI_HS_VBOOST_MASK "0xB" CACHE STRING "" FORCE)  # banks 1,2,4
	        set(GDI_HS_ENABLE_DELAY_MS "2000" CACHE STRING "" FORCE)
	    endif()

    foreach(path IN ITEMS
        ${STM32_HAL_DIR}/Inc
        ${STM32_HAL_DIR}/Src
        ${STM32_CMSIS_DEVICE_DIR}/Include
        ${FREERTOS_KERNEL_DIR}/include
    )
        if(NOT EXISTS ${path})
            message(FATAL_ERROR
                "Missing required dependency path: ${path}\n"
                "It looks like git submodules were not initialized.\n"
                "Run the build script in scripts/ (it will fetch submodules), or manually init submodules in lib/FreeRTOS and lib/STM32CubeG4."
            )
        endif()
    endforeach()

     set(FIRMWARE_SOURCES
         src/main.cpp
         src/board_init.cpp
         src/adc_sampler.cpp
         src/boost_control.cpp
         src/can_driver.cpp
         src/gdi_can_protocol.cpp
         src/freertos_config.c
         src/high_side_control.cpp
         src/injector_bridge.cpp
         src/injector_control.cpp
         src/lua_scripting.cpp
         src/safety_monitor.cpp
         src/spi_driver.cpp
        src/stm32g4xx_hal_msp.c
        src/startup_stm32g474xx.s
        ${STM32CUBE_DIR}/Projects/STM32G474E-EVAL1/Templates_LL/Src/system_stm32g4xx.c
    )

     set(HAL_SOURCES
         ${STM32_HAL_DIR}/Src/stm32g4xx_hal.c
         ${STM32_HAL_DIR}/Src/stm32g4xx_hal_adc.c
         ${STM32_HAL_DIR}/Src/stm32g4xx_hal_adc_ex.c
         ${STM32_HAL_DIR}/Src/stm32g4xx_hal_cortex.c
         ${STM32_HAL_DIR}/Src/stm32g4xx_hal_dma.c
         ${STM32_HAL_DIR}/Src/stm32g4xx_hal_dma_ex.c
         ${STM32_HAL_DIR}/Src/stm32g4xx_hal_flash.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_flash_ex.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_gpio.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_pwr.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_pwr_ex.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_rcc.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_rcc_ex.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_tim.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_tim_ex.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_spi.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_fdcan.c
        ${STM32_HAL_DIR}/Src/stm32g4xx_hal_iwdg.c
    )

    set(FREERTOS_SOURCES
        ${FREERTOS_KERNEL_DIR}/tasks.c
        ${FREERTOS_KERNEL_DIR}/queue.c
        ${FREERTOS_KERNEL_DIR}/list.c
        ${FREERTOS_KERNEL_DIR}/event_groups.c
        ${FREERTOS_KERNEL_DIR}/stream_buffer.c
        ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM4F/port.c
        ${FREERTOS_KERNEL_DIR}/portable/MemMang/heap_4.c
    )

    add_library(lua STATIC
        lib/lua/lapi.c
        lib/lua/lauxlib.c
        lib/lua/lbaselib.c
        lib/lua/lcode.c
        lib/lua/lctype.c
        lib/lua/ldebug.c
        lib/lua/ldo.c
        lib/lua/ldump.c
        lib/lua/lfunc.c
        lib/lua/lgc.c
        lib/lua/linit.c
        lib/lua/llex.c
        lib/lua/lmathlib.c
        lib/lua/lmem.c
        lib/lua/lobject.c
        lib/lua/lopcodes.c
        lib/lua/lparser.c
        lib/lua/lstate.c
        lib/lua/lstring.c
        lib/lua/lstrlib.c
        lib/lua/ltable.c
        lib/lua/ltablib.c
        lib/lua/ltm.c
        lib/lua/lundump.c
        lib/lua/lutf8lib.c
        lib/lua/lvm.c
        lib/lua/lzio.c
    )
    target_include_directories(lua PUBLIC lib/lua)

    add_executable(GDI_STM_Firmware
        ${FIRMWARE_SOURCES}
        ${HAL_SOURCES}
        ${FREERTOS_SOURCES}
    )

    target_include_directories(GDI_STM_Firmware PRIVATE
        ${PROJECT_INC_DIR}
        lib/lua
        ${STM32_HAL_DIR}/Inc
        ${STM32_HAL_DIR}/Inc/Legacy
        ${STM32_CMSIS_CORE_DIR}/Include
        ${STM32_CMSIS_DEVICE_DIR}/Include
        ${FREERTOS_KERNEL_DIR}/include
        ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM4F
    )

    target_compile_definitions(GDI_STM_Firmware PRIVATE
        STM32G474xx
        GDI_ENABLE_HS_VBAT=$<BOOL:${GDI_ENABLE_HS_VBAT}>
        GDI_HS_VBAT_MASK=${GDI_HS_VBAT_MASK}
        GDI_ENABLE_HS_VBOOST=$<BOOL:${GDI_ENABLE_HS_VBOOST}>
        GDI_HS_VBOOST_MASK=${GDI_HS_VBOOST_MASK}
        GDI_HS_ENABLE_DELAY_MS=${GDI_HS_ENABLE_DELAY_MS}
    )

    target_compile_options(GDI_STM_Firmware PRIVATE
        -ffunction-sections
        -fdata-sections
    )

    target_link_options(GDI_STM_Firmware PRIVATE
        -T${LINKER_SCRIPT}
        -Wl,-Map=$<TARGET_FILE_DIR:GDI_STM_Firmware>/$<TARGET_FILE_BASE_NAME:GDI_STM_Firmware>.map
        -Wl,--gc-sections
        --specs=nano.specs
        --specs=nosys.specs
    )

    target_link_libraries(GDI_STM_Firmware PRIVATE lua)

    add_custom_command(TARGET GDI_STM_Firmware POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:GDI_STM_Firmware> $<TARGET_FILE_DIR:GDI_STM_Firmware>/$<TARGET_FILE_BASE_NAME:GDI_STM_Firmware>.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:GDI_STM_Firmware> $<TARGET_FILE_DIR:GDI_STM_Firmware>/$<TARGET_FILE_BASE_NAME:GDI_STM_Firmware>.bin
        COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:GDI_STM_Firmware>
        VERBATIM
    )
endif()

if(GDI_BUILD_TESTS)
    include(CTest)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(unit_tests
        tests/test_main.cpp
        tests/test_spi_driver.cpp
        tests/test_lua_scripting.cpp
        tests/integration/test_can_integration.cpp
    )

    target_include_directories(unit_tests PRIVATE
        ${PROJECT_INC_DIR}
        lib/lua
    )

    target_link_libraries(unit_tests PRIVATE
        gtest
        gtest_main
        gmock
        gmock_main
        lua
    )

    add_test(NAME UnitTests COMMAND unit_tests)
endif()
