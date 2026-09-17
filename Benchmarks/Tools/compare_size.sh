#!/usr/bin/env bash
# Prints a size report (text/data/bss) for both architecture builds
# side by side. Run build.sh first.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RISCV_TOOLCHAIN_BIN="${RISCV_TOOLCHAIN_BIN:-$HOME/.pico-sdk/toolchain/15/bin}"

ARM_ELF="$ROOT/build-arm/App/bench_suite.elf"
RISCV_ELF="$ROOT/build-riscv/App/bench_suite.elf"

for f in "$ARM_ELF" "$RISCV_ELF"; do
    if [ ! -f "$f" ]; then
        echo "error: missing $f -- run ./build.sh first" >&2
        exit 1
    fi
done

echo "== ARM (Cortex-M33) =="
arm-none-eabi-size "$ARM_ELF"

echo
echo "== RISC-V (Hazard3) =="
"$RISCV_TOOLCHAIN_BIN/riscv32-unknown-elf-size" "$RISCV_ELF"
