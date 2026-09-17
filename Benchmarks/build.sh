#!/usr/bin/env bash
# Configures and builds the benchmark suite for both RP2350 cores.
# ARM uses the system arm-none-eabi-gcc; RISC-V uses the toolchain installed
# at RISCV_TOOLCHAIN_BIN below (see Benchmarks/README.md for how to get it).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RISCV_TOOLCHAIN_BIN="${RISCV_TOOLCHAIN_BIN:-$HOME/.pico-sdk/toolchain/15/bin}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

if [ ! -x "$RISCV_TOOLCHAIN_BIN/riscv32-unknown-elf-gcc" ]; then
    echo "RISC-V toolchain not found at $RISCV_TOOLCHAIN_BIN - installing it now."
    RISCV_TOOLCHAIN_BIN="$RISCV_TOOLCHAIN_BIN" "$ROOT/Tools/install_riscv_toolchain.sh"
fi

echo "== ARM (Cortex-M33), build type: $BUILD_TYPE =="
cmake -S "$ROOT" -B "$ROOT/build-arm" \
    -DPICO_PLATFORM=rp2350-arm-s \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$ROOT/build-arm" -j"$(nproc)"

echo
echo "== RISC-V (Hazard3), build type: $BUILD_TYPE =="
cmake -S "$ROOT" -B "$ROOT/build-riscv" \
    -DPICO_PLATFORM=rp2350-riscv \
    -DPICO_TOOLCHAIN_PATH="$RISCV_TOOLCHAIN_BIN" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$ROOT/build-riscv" -j"$(nproc)"

echo
echo "Done:"
echo "  $ROOT/build-arm/App/bench_suite.uf2"
echo "  $ROOT/build-riscv/App/bench_suite.uf2"
