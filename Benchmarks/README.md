# Benchmarks

Bare-metal (no RTOS, no WiFi) benchmark suite that builds for both RP2350
cores from the same source: `PICO_PLATFORM=rp2350-arm-s` (Cortex-M33) and
`PICO_PLATFORM=rp2350-riscv` (Hazard3). Deliberately minimal so a measured
cycle count reflects the CPU core and the compiler, not scheduler/driver
overhead. See the top-level [README](../README.md) for the project's goals.

Uses the pico-sdk/OpenOCD/debugprobe/picotool submodules vendored in the
top-level [`Dependencies/`](../Dependencies). The pinned pico-sdk revision is
authoritative - `./build.sh` checks it out on first run and passes it to CMake
explicitly, so a `PICO_SDK_PATH` left in your environment by some other project
won't silently swap the SDK out from under a comparison. Pass
`-DPICO_SDK_PATH=...` if you actually want to build against a different one.

## Layout

```
Benchmarks/
  CMakeLists.txt         top-level build, points at the shared pico-sdk
  App/                   the bench_suite firmware
    main.c                 runs each benchmark, prints CSV over USB serial
    harness.h               per-architecture cycle counter (DWT / mcycle CSR)
    benchmarks/              one workload per file
  Tools/
    compare_size.sh        text/data/bss size, both architectures side by side
    compare_results.py     two captured runs side by side, us ratio + cyc/iter
    dump_asm.sh             annotated disassembly + symbol sizes -> Results/
    capture_serial.py       save one benchmark run's CSV output -> Results/
    install_riscv_toolchain.sh  fetches riscv32-unknown-elf-gcc (auto-run by build.sh)
  Results/                (gitignored) captured .csv/.lst output lands here
  build.sh                configures + builds both architectures
```

## One-time setup: RISC-V toolchain

`arm-none-eabi-gcc` is assumed to already be on `PATH`. RISC-V needs
Raspberry Pi's prebuilt `riscv32-unknown-elf-gcc` (matches the vendored
pico-sdk 2.1.1 exactly, including Hazard3's Zcb/Zcmp compressed-instruction
support) - `./build.sh` installs it automatically the first time (to
`~/.pico-sdk/toolchain/15`, a local non-sudo install, nothing added to git)
if it isn't already there. To fetch it up front, or on an unsupported host
(Linux/macOS x86_64/arm64 are auto-detected; see
[pico-sdk-tools releases](https://github.com/raspberrypi/pico-sdk-tools/releases/tag/v2.1.1-3)
for anything else):

```sh
Tools/install_riscv_toolchain.sh
```

## Build

```sh
./build.sh
```

Produces `build-arm/App/bench_suite.uf2` and `build-riscv/App/bench_suite.uf2`.
Both are built with `CMAKE_BUILD_TYPE=Release` by default (`BUILD_TYPE=Debug
./build.sh` to override) - keep this the same across both when comparing, since
it's the single biggest lever on the numbers.

The board is pinned to `PICO_BOARD=pico2_w` rather than left to the SDK's
default, so it's a recorded build input alongside the SDK revision
(`PICO_BOARD=pico2 ./build.sh` to override). The choice doesn't affect the
measurements - a `pico2` build differs by exactly two bytes, the board-name
string in the binary-info block, since the boards differ only in LED and
CYW43 wiring and this suite touches neither. The wireless chip is never
powered up: nothing here links `pico_cyw43_arch`.

Worth knowing on a Pico 2 W, though, if you extend the suite: **GPIO
23/24/25/29 belong to the CYW43 chip** and the onboard LED sits behind it. A
scope trigger or progress indicator wants a spare GPIO - driving the LED
would mean linking the wireless stack into a deliberately bare-metal suite.

A fresh clone needs nothing else: `build.sh` checks out the vendored pico-sdk
(recursively - the USB-CDC output needs its nested tinyusb) and installs the
RISC-V toolchain, if either is missing.

## Flash and capture results

1. Hold BOOTSEL, plug in the Pico 2 W (or `picotool reboot -u -f` if it's
   already running a UF2), then copy the `.uf2` onto the mass-storage drive
   that appears - or `picotool load build-arm/App/bench_suite.uf2` /
   OpenOCD with a debug probe if you have one wired up.
2. The board re-enumerates as a USB CDC serial port
   (`/dev/ttyACM0` on Linux, typically).
3. Capture one run:
   ```sh
   python3 Tools/capture_serial.py /dev/ttyACM0
   ```
   Start it whenever - it waits for the port to appear rather than failing if
   the board isn't plugged in yet, and it reconnects when the board
   re-enumerates, so resetting to trigger a run is the expected workflow
   (`main.c` waits 3s after boot before printing). It exits after one
   complete run, writing `Results/<arch>_<timestamp>.csv` with rows of
   `name,iterations,cycles,us`. A run cut short by a disconnect is saved as
   `..._partial.csv` rather than discarded.
4. Repeat for the other architecture's `.uf2`, then compare the two runs:
   ```sh
   python3 Tools/compare_results.py          # newest run of each architecture
   ```
   Ratios and the winner come from the microsecond column (the fair
   cross-architecture number); cycles per iteration are printed alongside
   because that's what explains a delta. Pass two paths explicitly to compare
   specific runs.

## Compare code size and disassembly

```sh
Tools/compare_size.sh   # text/data/bss for both ELFs
Tools/dump_asm.sh       # -> Results/bench_suite_{arm,riscv}.lst + symbol sizes
```

Use the disassembly to *explain* a cycle or size delta you already measured,
not as a standalone "which asm looks nicer" comparison - see the top-level
README for why. Three places worth looking first, each testing an isolated
ISA asymmetry in a different direction - and each checked against the real
disassembly rather than left as a guess:

- `bench_bitops.c` (RISC-V favoring): Hazard3's Zbb extension gives native
  popcount/clz/rotate; Cortex-M33 has a native clz but calls a software
  helper (`__popcountsi2`) for popcount.
- `bench_bitfield.c` (ARM favoring): Cortex-M33's UBFX/BFI fold a bit-field
  extract/insert into one instruction each; RISC-V needs srli+andi /
  slli+and+and+or. 8 instructions/iteration on ARM vs 11 on RISC-V.
- `bench_shift_fused.c` (inconclusive - and that's the interesting part):
  written to test ARM's barrel-shifter operand fusion, it turned out a
  near-exact tie once GCC's induction-variable strength reduction and
  RISC-V's fused compare-and-branch were accounted for. See the comment in
  that file for the full breakdown - it's a good example of why a single
  synthetic snippet is a hypothesis to verify against disassembly, not a
  conclusion on its own.

## Measurement notes

- **Cycles** (`bench_cycles()` in `harness.h`) are ISA-native (DWT->CYCCNT vs
  the `mcycle` CSR) and only comparable *within* one architecture's build -
  don't diff raw cycle counts across architectures.
- **Microseconds** (`time_us_64()`, printed alongside cycles) come from
  RP2350's shared hardware timer peripheral, which both cores read
  identically - this is the fair cross-architecture number.
- Every benchmark result is funneled through a `volatile` sink
  (`bench_common.c`) and derives its input data from a `volatile` seed, so
  the compiler can't prove the loop is dead or constant-fold it away. If you
  add a benchmark, keep doing this or `-O3` may just delete your workload.
- Only core 0 runs anything right now; core 1 sits idle.

## Adding a benchmark

Add `benchmarks/bench_foo.c` with a `void bench_foo(uint32_t iterations)`
that ends by writing to `bench_sink`, declare it and add it to the
`benchmarks[]` table in `main.c`, and list the new file in
`App/CMakeLists.txt`.
