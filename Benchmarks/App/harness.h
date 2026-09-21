/*
 * Cycle-counter harness, one implementation per architecture selected at
 * compile time. Both sides expose the same three symbols:
 *
 *   BENCH_ARCH_NAME  - human-readable label for the current build
 *   bench_init()     - one-time setup (enable the counter)
 *   bench_cycles()   - free-running core-clock cycle snapshot
 *
 * bench_cycles() is ISA-native (DWT->CYCCNT on Cortex-M33, the `cycle` CSR
 * on Hazard3) so it is only meaningful as a *delta* within one build - do
 * not compare raw cycle values across architectures directly. For a fair
 * cross-architecture number use the wall-clock time_us_64() reading taken
 * alongside it in main.c: it comes from the RP2350's shared hardware timer
 * peripheral, which both cores read identically.
 */
#pragma once
#include <stdint.h>

#if defined(__arm__)

#define BENCH_ARCH_NAME "arm-cortex-m33"

/* Raw MMIO instead of pulling in the SDK's cmsis_core target: these DWT/
 * CoreDebug register addresses are fixed by the Armv8-M architecture, not
 * the SDK's CMake library graph, so this works regardless of which pico-sdk
 * libraries the target happens to link. */
#define BENCH_DWT_CTRL   (*(volatile uint32_t *)0xE0001000u)
#define BENCH_DWT_CYCCNT (*(volatile uint32_t *)0xE0001004u)
#define BENCH_DEMCR      (*(volatile uint32_t *)0xE000EDFCu)

#define BENCH_DEMCR_TRCENA       (1u << 24)
#define BENCH_DWT_CTRL_CYCCNTENA (1u << 0)

static inline void bench_init(void)
{
    BENCH_DEMCR |= BENCH_DEMCR_TRCENA;
    BENCH_DWT_CYCCNT = 0;
    BENCH_DWT_CTRL |= BENCH_DWT_CTRL_CYCCNTENA;
}

static inline uint32_t bench_cycles(void)
{
    return BENCH_DWT_CYCCNT;
}

#elif defined(__riscv)

#define BENCH_ARCH_NAME "riscv-hazard3"

static inline void bench_init(void)
{
    /* mcycle free-runs from reset on Hazard3; nothing to enable. */
}

static inline uint32_t bench_cycles(void)
{
    uint32_t cycles;
    __asm__ volatile("csrr %0, mcycle" : "=r"(cycles));
    return cycles;
}

#else
#error "harness.h: unsupported architecture (expected __arm__ or __riscv)"
#endif
