#!/usr/bin/env bash
# Configures and builds the benchmark suite for both RP2350 cores.
# ARM uses the system arm-none-eabi-gcc; RISC-V uses the toolchain installed
# at RISCV_TOOLCHAIN_BIN below (see Benchmarks/README.md for how to get it).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$ROOT/.." && pwd)"
PICO_SDK_DIR="$REPO_ROOT/Dependencies/pico-sdk"
RISCV_TOOLCHAIN_BIN="${RISCV_TOOLCHAIN_BIN:-$HOME/.pico-sdk/toolchain/15/bin}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
# Pinned rather than left to the SDK's default so the board is a recorded
# build input like the SDK revision, not a guess. This is the hardware the
# numbers are measured on; pico2 builds identically (the boards differ only in
# LED and CYW43 wiring, neither of which this suite touches - the compiled
# output differs by two bytes, the board name in the binary-info block). The one
# thing to remember on pico2_w: GPIO 23/24/25/29 belong to the CYW43 chip and
# the onboard LED is behind it, so a future scope trigger or progress
# indicator wants a spare GPIO, not the LED - driving the LED would mean
# linking the wireless stack into a deliberately bare-metal suite.
PICO_BOARD="${PICO_BOARD:-pico2_w}"

# A plain `git clone` leaves the submodules empty, and the suite is pinned to
# this exact SDK revision - so bootstrap it here rather than leaving it as a
# manual step a fresh clone has to know about. tusb.h is the check rather than
# pico_sdk_init.cmake because the USB-CDC output needs pico-sdk's own nested
# tinyusb submodule too, which a non-recursive init leaves behind.
if [ ! -f "$PICO_SDK_DIR/pico_sdk_init.cmake" ] || \
   [ ! -f "$PICO_SDK_DIR/lib/tinyusb/src/tusb.h" ]; then
    echo "Vendored pico-sdk incomplete - initialising submodule (one-off, ~1GB)."
    git -C "$REPO_ROOT" submodule update --init --recursive Dependencies/pico-sdk
fi

if [ ! -x "$RISCV_TOOLCHAIN_BIN/riscv32-unknown-elf-gcc" ]; then
    echo "RISC-V toolchain not found at $RISCV_TOOLCHAIN_BIN - installing it now."
    RISCV_TOOLCHAIN_BIN="$RISCV_TOOLCHAIN_BIN" "$ROOT/Tools/install_riscv_toolchain.sh"
fi

echo "== ARM (Cortex-M33), board: $PICO_BOARD, build type: $BUILD_TYPE =="
cmake -S "$ROOT" -B "$ROOT/build-arm" \
    -DPICO_SDK_PATH="$PICO_SDK_DIR" \
    -DPICO_BOARD="$PICO_BOARD" \
    -DPICO_PLATFORM=rp2350-arm-s \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$ROOT/build-arm" -j"$(nproc)"

echo
echo "== RISC-V (Hazard3), board: $PICO_BOARD, build type: $BUILD_TYPE =="
cmake -S "$ROOT" -B "$ROOT/build-riscv" \
    -DPICO_SDK_PATH="$PICO_SDK_DIR" \
    -DPICO_BOARD="$PICO_BOARD" \
    -DPICO_PLATFORM=rp2350-riscv \
    -DPICO_TOOLCHAIN_PATH="$RISCV_TOOLCHAIN_BIN" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$ROOT/build-riscv" -j"$(nproc)"

echo
echo "Done:"
echo "  $ROOT/build-arm/App/bench_suite.uf2"
echo "  $ROOT/build-riscv/App/bench_suite.uf2"
