#!/usr/bin/env python3
"""Compare an ARM and a RISC-V benchmark CSV side by side.

    python3 Tools/compare_results.py                  # newest of each arch
    python3 Tools/compare_results.py a.csv b.csv      # specific files

Ratios and the winner come from the microsecond column, which is the fair
cross-architecture number (shared hardware timer). Cycles per iteration are
shown alongside because that's what explains a delta, but they're ISA-native
counters - see the README's measurement notes.
"""
import csv
import sys
from pathlib import Path

RESULTS_DIR = Path(__file__).resolve().parent.parent / "Results"
ARCHES = ("arm-cortex-m33", "riscv-hazard3")
TIE_THRESHOLD = 0.01  # within 1% is a tie, not a winner


def newest(arch: str) -> Path:
    # Timestamps are YYYYmmdd-HHMMSS, so lexicographic order is chronological.
    runs = sorted(p for p in RESULTS_DIR.glob(f"{arch}_*.csv")
                  if "_partial" not in p.name)
    if not runs:
        sys.exit(f"error: no complete {arch} run in {RESULTS_DIR} - "
                 f"capture one with Tools/capture_serial.py")
    return runs[-1]


def load(path: Path) -> dict[str, tuple[int, int, int]]:
    with path.open() as f:
        return {r["name"]: (int(r["iterations"]), int(r["cycles"]), int(r["us"]))
                for r in csv.DictReader(f)}


def main() -> int:
    if len(sys.argv) == 3:
        arm_path, rv_path = Path(sys.argv[1]), Path(sys.argv[2])
    elif len(sys.argv) == 1:
        arm_path, rv_path = newest(ARCHES[0]), newest(ARCHES[1])
    else:
        sys.exit(f"usage: {sys.argv[0]} [<arm.csv> <riscv.csv>]")

    arm, rv = load(arm_path), load(rv_path)
    print(f"ARM   : {arm_path.name}")
    print(f"RISC-V: {rv_path.name}\n")

    print(f"{'benchmark':<14}{'ARM us':>9}{'RV us':>9}{'ratio':>8}"
          f"{'winner':>9}   {'ARM c/it':>9}{'RV c/it':>9}{'delta':>9}")
    print("-" * 78)

    for name, (iters, arm_cyc, arm_us) in arm.items():
        if name not in rv:
            print(f"{name:<14}  (not in the RISC-V run)")
            continue
        rv_iters, rv_cyc, rv_us = rv[name]
        if rv_iters != iters:
            print(f"{name:<14}  (iteration counts differ - not comparable)")
            continue

        ratio = max(arm_us, rv_us) / min(arm_us, rv_us)
        if ratio - 1 < TIE_THRESHOLD:
            winner = "tie"
        else:
            winner = "RISC-V" if rv_us < arm_us else "ARM"
        arm_ci, rv_ci = arm_cyc / iters, rv_cyc / iters

        print(f"{name:<14}{arm_us:>9}{rv_us:>9}{ratio:>8.2f}{winner:>9}   "
              f"{arm_ci:>9.2f}{rv_ci:>9.2f}{rv_ci - arm_ci:>+9.2f}")

    missing = [n for n in rv if n not in arm]
    if missing:
        print(f"\nOnly in the RISC-V run: {', '.join(missing)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
