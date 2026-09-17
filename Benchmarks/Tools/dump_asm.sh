#!/usr/bin/env bash
# Dumps annotated (source-interleaved) disassembly for both builds into
# Results/, and a per-function size breakdown alongside it. Run build.sh
# first.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RISCV_TOOLCHAIN_BIN="${RISCV_TOOLCHAIN_BIN:-$HOME/.pico-sdk/toolchain/15/bin}"
RESULTS="$ROOT/Results"
mkdir -p "$RESULTS"

ARM_ELF="$ROOT/build-arm/App/bench_suite.elf"
RISCV_ELF="$ROOT/build-riscv/App/bench_suite.elf"

for f in "$ARM_ELF" "$RISCV_ELF"; do
    if [ ! -f "$f" ]; then
        echo "error: missing $f -- run ./build.sh first" >&2
        exit 1
    fi
done

arm-none-eabi-objdump -d -S "$ARM_ELF" > "$RESULTS/bench_suite_arm.lst"
"$RISCV_TOOLCHAIN_BIN/riscv32-unknown-elf-objdump" -d -S "$RISCV_ELF" > "$RESULTS/bench_suite_riscv.lst"

arm-none-eabi-nm --size-sort -S "$ARM_ELF" > "$RESULTS/bench_suite_arm_symbols.txt"
"$RISCV_TOOLCHAIN_BIN/riscv32-unknown-elf-nm" --size-sort -S "$RISCV_ELF" > "$RESULTS/bench_suite_riscv_symbols.txt"

echo "Wrote:"
echo "  $RESULTS/bench_suite_arm.lst"
echo "  $RESULTS/bench_suite_riscv.lst"
echo "  $RESULTS/bench_suite_arm_symbols.txt"
echo "  $RESULTS/bench_suite_riscv_symbols.txt"
