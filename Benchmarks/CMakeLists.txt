# ****************************************************************************
# CMakeLists.txt - RP2350 ARM vs RISC-V benchmark suite
#
# Bare-metal (no RTOS, no WiFi) so that measured cycles/size/disassembly
# reflect the CPU core and compiler only, not scheduler or stack overhead.
#
# Configure separately per architecture, e.g.:
#   cmake -S . -B build-arm    -DPICO_PLATFORM=rp2350-arm-s
#   cmake -S . -B build-riscv  -DPICO_PLATFORM=rp2350-riscv \
#       -DPICO_TOOLCHAIN_PATH=$HOME/.pico-sdk/toolchain/15/bin
# or just run ./build.sh to do both.
# ****************************************************************************
cmake_minimum_required(VERSION 3.13)

# Use the pico-sdk vendored (as a submodule) under the shared top-level
# Dependencies/. The pinned revision is authoritative: cycle counts and code
# sizes are only comparable against a known SDK, so an ambient PICO_SDK_PATH
# in the environment (a machine-wide SDK checkout from some other project)
# must not silently swap it out. An explicit -DPICO_SDK_PATH= on the command
# line still wins, for deliberately testing against a different SDK.
if (NOT PICO_SDK_PATH)
    get_filename_component(VENDORED_PICO_SDK_PATH
            "${CMAKE_CURRENT_LIST_DIR}/../Dependencies/pico-sdk" ABSOLUTE)
    if (EXISTS "${VENDORED_PICO_SDK_PATH}/pico_sdk_init.cmake")
        if (DEFINED ENV{PICO_SDK_PATH})
            message(STATUS "Ignoring PICO_SDK_PATH=$ENV{PICO_SDK_PATH} from the "
                    "environment in favour of the pinned submodule "
                    "(pass -DPICO_SDK_PATH=... to override deliberately)")
        endif ()
        set(PICO_SDK_PATH "${VENDORED_PICO_SDK_PATH}")
    elseif (NOT DEFINED ENV{PICO_SDK_PATH})
        message(FATAL_ERROR
                "Dependencies/pico-sdk is not checked out. Run ./build.sh, "
                "which bootstraps it, or:\n"
                "  git submodule update --init --recursive Dependencies/pico-sdk")
    endif ()
endif ()

include(pico_sdk_import.cmake)

project(rp2350_benchmarks C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Same optimization level on both architectures is the whole point of the
# comparison - keep this the single knob that controls it.
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()

pico_sdk_init()

add_subdirectory(App)
