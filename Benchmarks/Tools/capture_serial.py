#!/usr/bin/env python3
"""Capture one benchmark run over USB CDC serial and save it under Results/.

Usage:
    python3 capture_serial.py /dev/ttyACM0

Flash bench_suite.uf2 (BOOTSEL copy or `picotool load`), then run this and
reset the board. The port doesn't have to exist yet - like tio, this waits
for the device to appear and reopens it on every reset, so you can leave it
running while you swap UF2s between the ARM and RISC-V builds.

Output file is named Results/<arch>_<timestamp>.csv, where <arch> is read
from the "arch=..." line the firmware prints first.
"""
import errno
import sys
import time
from pathlib import Path

import serial

RESULTS_DIR = Path(__file__).resolve().parent.parent / "Results"
BAUD = 115200
# main.c waits 3s after boot before printing; 10s leaves room for a reset
# that happens slightly after we open the port.
READ_TIMEOUT_S = 10
POLL_INTERVAL_S = 0.25


def open_port(port: str) -> serial.Serial:
    """Open the port, waiting for it to show up rather than dying if it
    doesn't exist yet.

    The board re-enumerates on every reset and vanishes while it's in BOOTSEL,
    so "not there yet" is the normal case in this workflow, not an error. Only
    the reason for waiting is reported, and only when it changes, so replugging
    doesn't spam the terminal.
    """
    last_reason = None
    while True:
        try:
            return serial.Serial(port, baudrate=BAUD, timeout=READ_TIMEOUT_S)
        except (serial.SerialException, OSError) as exc:
            # A permission error won't clear by itself the way a missing device
            # will, so it's worth naming the usual cause - but keep waiting
            # regardless, since udev rules can land a moment after plug-in.
            if getattr(exc, "errno", None) == errno.EACCES:
                reason = (f"No permission to open {port} - you may need to be "
                          f"in the 'dialout' group (log out and back in after "
                          f"adding yourself). Still waiting")
            else:
                reason = f"Waiting for {port}"
            if reason != last_reason:
                print(f"{reason}... (Ctrl-C to give up)", file=sys.stderr)
                last_reason = reason
            time.sleep(POLL_INTERVAL_S)


def save(lines: list[str], arch: str | None, partial: bool) -> None:
    if not lines:
        print("Nothing captured - not writing a file.", file=sys.stderr)
        return
    RESULTS_DIR.mkdir(exist_ok=True)
    timestamp = time.strftime("%Y%m%d-%H%M%S")
    suffix = "_partial" if partial else ""
    out_path = RESULTS_DIR / f"{arch or 'unknown'}_{timestamp}{suffix}.csv"
    out_path.write_text("\n".join(lines) + "\n")
    print(f"\nWrote {out_path}" + (" (incomplete run)" if partial else ""))


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <serial-port>", file=sys.stderr)
        return 1

    port = sys.argv[1]

    # One pass per connection. Resetting the board is how you start a run, and
    # a reset drops the USB CDC port - so a disconnect sends us back to
    # waiting rather than exiting, and the script only returns once it has
    # seen a complete run (or you Ctrl-C).
    while True:
        ser = open_port(port)
        print(f"Connected to {port} - reset the board to start a run.",
              file=sys.stderr)

        arch: str | None = None
        lines: list[str] = []
        nudged = False
        try:
            while True:
                try:
                    raw = ser.readline().decode("utf-8", errors="replace").strip()
                except (serial.SerialException, OSError):
                    print(f"\n{port} disconnected mid-capture.", file=sys.stderr)
                    # Don't silently bin a run that was nearly complete.
                    if lines:
                        save(lines, arch, partial=True)
                    break

                if not raw:
                    if not nudged:
                        print(f"No output for {READ_TIMEOUT_S}s - reset the "
                              f"board (or check it isn't in BOOTSEL).",
                              file=sys.stderr)
                        nudged = True
                    continue

                print(raw)
                if raw.startswith("arch="):
                    arch = raw.split("=", 1)[1]
                    continue
                if raw == "done":
                    save(lines, arch, partial=False)
                    return 0
                lines.append(raw)
        finally:
            ser.close()


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        print("\nInterrupted.", file=sys.stderr)
        raise SystemExit(130)
