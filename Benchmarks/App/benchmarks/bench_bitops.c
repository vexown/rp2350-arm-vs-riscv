#include "bench_common.h"

static inline uint32_t rotl32(uint32_t x, unsigned n)
{
    return (x << n) | (x >> (32 - n));
}

/* Popcount/clz/rotate via compiler builtins. Worth inspecting the
 * disassembly of this one specifically: Hazard3 on RP2350 has the Zbb
 * bitmanip extension (native popcount/clz/rotate), while Cortex-M33 has a
 * native CLZ but no popcount instruction (GCC will call a software helper
 * for __builtin_popcount on ARM). That asymmetry is exactly the kind of
 * ISA-level difference this project exists to surface. */
void bench_bitops(uint32_t iterations)
{
    uint32_t a = bench_seed | 1u;
    uint32_t acc = 0;

    for (uint32_t i = 0; i < iterations; i++)
    {
        acc += (uint32_t)__builtin_popcount(a);
        acc += (uint32_t)__builtin_clz(a | 1u);
        a = rotl32(a, (i & 31u) + 1u) ^ (i * 2654435761u);
    }

    bench_sink = acc;
}
