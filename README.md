# RP2350: ARM vs RISC-V

A hands-on comparison of the **dual-core Arm Cortex-M33** and **dual-core Hazard3 RISC-V** implementations in the Raspberry Pi RP2350.

The goal is simple:

> **As an embedded firmware engineer, how much does the CPU architecture actually matter when the hardware around it is the same?**

Rather than comparing the architectures purely on paper, this project runs the **same or equivalent workloads on the same RP2350**, switching between the Cortex-M33 and Hazard3 RISC-V cores.

## Why?

RISC-V gets a lot of attention for being an open ISA, supporting extensions, and offering architectural freedom.

Those are genuinely important properties — particularly for chip designers.

But what does any of that actually mean for someone writing everyday MCU firmware?

The RP2350 provides a particularly interesting way to investigate this because the same chip contains both:

* **2× Arm Cortex-M33 cores**
* **2× Hazard3 RISC-V cores**

with the same memory, peripherals, DMA, PIO, timers, GPIO, etc.

That makes it possible to change the CPU architecture while keeping much of the rest of the system constant.

## What will be compared?

The benchmark suite will evolve over time, but may include:

* Integer arithmetic
* Multiplication and division
* Bit manipulation
* Memory access
* `memcpy` / `memset`
* Function calls and calling conventions
* Interrupt latency
* Exception/trap handling
* Context switching
* Atomic operations
* DSP-style workloads
* Floating-point workloads
* Code size
* Compiler output / generated assembly
* Potentially power consumption

The intention is not to produce a single meaningless "winner" score, but to understand **where the architectures actually differ and whether those differences matter in real firmware**.

## Experimental philosophy

Whenever practical:

1. Use the same RP2350 hardware.
2. Use the same clock configuration.
3. Use the same peripherals and memory.
4. Use equivalent C/C++ source code.
5. Use comparable compiler optimization settings.
6. Measure the actual hardware rather than relying solely on theoretical instruction counts.
7. Inspect generated assembly when it helps explain the results.
8. Document architectural differences that explain significant results.

The important question is not:

> "Which ISA is better?"

but:

> **"What changes when I move real embedded firmware from Cortex-M33 to RISC-V?"**

## Example

A simple workload might be compiled for both targets:

```text
                Same RP2350
                    │
          ┌─────────┴─────────┐
          │                   │
     Cortex-M33           Hazard3 RISC-V
          │                   │
          └─────────┬─────────┘
                    │
              Same workload
                    │
             Measure results
```

## What this project is NOT

This is not intended to be:

* a general benchmark of all ARM processors vs all RISC-V processors
* a claim that one ISA is universally better
* a synthetic ISA shootout
* a comparison of different MCU vendors

The comparison is deliberately narrow:

**Cortex-M33 vs Hazard3 on the RP2350.**

## Expected outcome

Honestly, I don't know.

For a lot of ordinary MCU firmware, the expectation is that the difference will be surprisingly small because the majority of the development experience comes from the **peripherals, SDK, compiler, debugger, RTOS, and surrounding SoC architecture**, rather than the ISA itself.

This project is an attempt to find out where that assumption stops being true.

## Hardware
* Raspberry Pi Pico 2 / Pico 2 W (or another RP2350 board)

## Software

The project will use the Raspberry Pi Pico SDK and appropriate toolchains for both architectures.

## Status

🚧 **Experimental / Work in Progress**

Currently setting up the benchmark framework and establishing equivalent build configurations for the Cortex-M33 and Hazard3 targets.

## License

This project is licensed under the **GNU General Public License v3.0**.

See [LICENSE](LICENSE) for the full license text.
