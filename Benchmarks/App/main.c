/*
 * RP2350 ARM vs RISC-V benchmark runner.
 *
 * Bare-metal: no RTOS, no WiFi, single core (core 0), nothing but the SDK's
 * stdio/timer init and the benchmark loop below - so a cycle count here
 * reflects the CPU core and the compiler's code generation, not scheduling
 * or driver overhead.
 *
 * Each benchmark is timed two ways:
 *   - bench_cycles(): ISA-native cycle counter (DWT->CYCCNT / `mcycle` CSR).
 *     Only comparable *within* one architecture's build.
 *   - time_us_64(): RP2350's shared hardware timer peripheral. Both cores
 *     read the same physical counter, so this is the fair cross-architecture
 *     number.
 *
 * Output is CSV over the USB CDC serial port, one line per benchmark:
 *   name,iterations,cycles,us
 */

#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "harness.h"
#include "bench_common.h"

typedef void (*bench_fn_t)(uint32_t iterations);

typedef struct {
    const char *name;
    bench_fn_t fn;
    uint32_t iterations;
} bench_entry_t;

void bench_integer_arith(uint32_t iterations);
void bench_muldiv(uint32_t iterations);
void bench_memcpy_workload(uint32_t iterations);
void bench_bitops(uint32_t iterations);
void bench_shift_fused(uint32_t iterations);
void bench_bitfield(uint32_t iterations);

static const bench_entry_t benchmarks[] = {
    {"integer_arith", bench_integer_arith,   200000},
    {"muldiv",        bench_muldiv,          200000},
    {"memcpy",        bench_memcpy_workload,  50000},
    {"bitops",        bench_bitops,          200000},
    {"shift_fused",   bench_shift_fused,     200000},
    {"bitfield",      bench_bitfield,        200000},
};

int main(void) {
    stdio_init_all();
    bench_init();

    /* Give the host time to enumerate/open the USB CDC port before the
     * results scroll past. */
    sleep_ms(3000);

    printf("arch=%s\n", BENCH_ARCH_NAME);
    printf("name,iterations,cycles,us\n");

    for (unsigned i = 0; i < sizeof(benchmarks) / sizeof(benchmarks[0]); i++) {
        const bench_entry_t *b = &benchmarks[i];

        uint32_t c0 = bench_cycles();
        uint64_t t0 = time_us_64();
        b->fn(b->iterations);
        uint64_t t1 = time_us_64();
        uint32_t c1 = bench_cycles();

        printf("%s,%lu,%lu,%llu\n",
               b->name,
               (unsigned long)b->iterations,
               (unsigned long)(c1 - c0),
               (unsigned long long)(t1 - t0));
    }

    printf("done\n");

    while (true) {
        tight_loop_contents();
    }
}
