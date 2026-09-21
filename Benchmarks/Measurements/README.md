# Measurements

Raw benchmark output, committed so the numbers can be read without cloning,
building and flashing. Files are exactly as `Tools/capture_serial.py` wrote
them - nothing filtered, averaged or reordered.

**There is no analysis here, deliberately.** This directory records what was
measured and under what conditions, so you can draw your own conclusions.
Where the project does interpret its own results, it does so next to the
code being explained - the comments in `App/benchmarks/*.c` and the
top-level [README](../../README.md).

To compare two runs:

```sh
python3 Tools/compare_results.py Measurements/2026-09-21/arm-cortex-m33_20260921-171240.csv \
                                 Measurements/2026-09-21/riscv-hazard3_20260921-171343.csv
```

## CSV format

`name,iterations,cycles,us` - one row per benchmark.

- `cycles` is an **ISA-native** counter: DWT->CYCCNT on Cortex-M33, the
  `mcycle` CSR on Hazard3. The two count their own core's clock, so treat
  them as comparable *within* an architecture, not across it.
- `us` comes from `time_us_64()`, RP2350's shared hardware timer peripheral,
  which both cores read identically.

Both cores ran at the same clock during these captures, so dividing `cycles`
by `us` should come out at the core clock in MHz for every row - a cheap
sanity check on any run.

## Capture sets

### 2026-09-21

Five consecutive runs per architecture, same board, same flashing session.
Each run was started by a full power cycle (USB unplugged and replugged),
so every run began from a cold boot rather than a warm reset. All ten files
come from one build of one commit.

| | |
|---|---|
| Board | Raspberry Pi Pico 2 W (RP2350A) |
| Core clock | 150 MHz (SDK default for this platform) |
| Repo commit | `7ff220f` |
| Build type | `Release` (`-O3`) |
| `PICO_BOARD` | `pico2_w` |
| pico-sdk | `bddd20f` (2.1.1) |
| ARM toolchain | `arm-none-eabi-gcc` 13.2.1 20231009 |
| RISC-V toolchain | `riscv32-unknown-elf-gcc` 15.1.0 (pico-sdk-tools v2.1.1-3) |
| CMake | 3.28.3 |
| Host | Linux x86_64 |

Run-to-run spread within an architecture is at most 0.154% on any row
(`integer_arith`; every other row is under 0.06%), so any single run is
representative of its set. All five are kept rather than a chosen one, so
the noise floor is visible rather than asserted.
