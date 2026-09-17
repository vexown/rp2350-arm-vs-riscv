#!/usr/bin/env python3
"""Capture one benchmark run over USB CDC serial and save it under Results/.

Usage:
    python3 capture_serial.py /dev/ttyACM0

Flash bench_suite.uf2 (BOOTSEL copy or `picotool load`), reset the board,
then run this within a few seconds - main.c waits 3s after stdio_init_all()
before printing so the host has time to open the port.

Output file is named Results/<arch>_<timestamp>.csv, where <arch> is read
from the "arch=..." line the firmware prints first.
"""
import sys
import time
from pathlib import Path

import serial

RESULTS_DIR = Path(__file__).resolve().parent.parent / "Results"


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <serial-port>", file=sys.stderr)
        return 1

    port = sys.argv[1]
    RESULTS_DIR.mkdir(exist_ok=True)

    with serial.Serial(port, baudrate=115200, timeout=10) as ser:
        arch = None
        lines = []
        while True:
            raw = ser.readline().decode("utf-8", errors="replace").strip()
            if not raw:
                print("timed out waiting for the board", file=sys.stderr)
                return 1
            print(raw)
            if raw.startswith("arch="):
                arch = raw.split("=", 1)[1]
                continue
            lines.append(raw)
            if raw == "done":
                break

    arch = arch or "unknown"
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    out_path = RESULTS_DIR / f"{arch}_{timestamp}.csv"
    out_path.write_text("\n".join(lines[:-1]) + "\n")  # drop the "done" marker
    print(f"\nWrote {out_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
